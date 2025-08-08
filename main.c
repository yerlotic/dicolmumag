#include "nob.h"
#include "thirdparty/cthreads.h"
#include <stdio.h>
#include <unistd.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "raylib/raylib.h"
#include "raylib/clay_renderer_raylib.c"
#include "layout.c"
#include <linux/limits.h>
#include <assert.h>
#include <fontconfig/fontconfig.h>
#define SUPPORT_URL "https://youtu.be/dQw4w9WgXcQ"
#include "thirdparty/cthreads.c"
#ifdef _WIN32
#include <processthreadsapi.h>
#endif // _WIN32

#define TESTING

// ascii file separator
// thx ascii!
// #define MULTI_SEPARATOR '\x1c'
#define MULTI_SEPARATOR '|'
#include "tinyfiledialogs.c"

#define MODIFIERS 3 // only 3 modifiers at the same time: !, </> and ^
#define ASHLAR_PREFIX "ashlar:"
#define SLEEP_THRESHOLD 5 //seconds of activity before not rendering that many frames

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} str_arr_t;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} str_t;

typedef struct AppState {
    Vector2 mousePosition;
    Vector2 scrollDelta;
    int renderWidth;
    int renderHeight;
} AppState;

#define pressed(id) IsMouseButtonPressed(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id)))
#define released(id) IsMouseButtonReleased(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id)))
#define CLAY_DIMENSIONS (Clay_Dimensions) { .width = GetScreenWidth(), \
                                            .height = GetScreenHeight() }
#define RunMagickThreaded() \
        cthreads_thread_ensure_cancelled(data->magickThread, &data->threadRunning); \
        cthreads_thread_create(&data->magickThread, NULL, RunMagickThread, (void *)data, NULL);

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("[CLAY] [ERROR] %s\n", errorData.errorText.chars);
}

// Don't forget to free the thing itself if u need to
#define nob_free_all(to_free) do {                              \
    for (size_t i = 0; i < to_free.count; i++) {                \
        fprintf(stderr, "Freeing: \"%s\"\n", to_free.items[i]); \
        free((void *) to_free.items[i]);                        \
    }                                                           \
} while(0)

void nob_kill(Nob_Proc *proc) {
    if (*proc == NOB_INVALID_PROC) return;
#ifdef _WIN32
    TerminateProcess(proc, 69);
#else
    kill(*proc, SIGKILL);
#endif // _WIN32
    *proc = NOB_INVALID_PROC;
}

bool nob_proc_running(Nob_Proc proc) {
    if (proc == NOB_INVALID_PROC) return false;
#ifdef _WIN32
    return GetExitCodeProcess(proc, STILL_ACTIVE);
#else
    int status;
    pid_t return_pid = waitpid(proc, &status, WNOHANG);
    if (return_pid == -1) {
        // error
        return false;
    } else if (return_pid == 0) {
        /* child is still running */
        return true;
    } else if (return_pid == proc) {
        return false;
    }
#endif // _WIN32
    return false;
}

void cthreads_thread_ensure_cancelled(struct cthreads_thread thread, bool *running) {
    if (*running) {
        cthreads_thread_cancel(thread);
        *running = false;
    }
}

#ifdef _WIN32
#define BLACKHOLE "nul"
#else
#define BLACKHOLE "/dev/null"
#endif // _WIN32
#define nob_cmd_run_async_silent(cmd) do { \
    Nob_Fd blackhole = nob_fd_open_for_write(BLACKHOLE); \
    nob_cmd_run_async_redirect(cmd, (Nob_Cmd_Redirect) {.fdout = &blackhole, .fderr = &blackhole}); \
} while(0)

int GetInputFiles(ClayVideoDemo_Data *data) {
    int allow_multiple_selects = true;
    char const *filter_params[] = {"*.png", "*.svg", "*.jpg", "*.jpeg"};
    char *input_path = tinyfd_openFileDialog("Path to images", "./", 4, filter_params, "Image file", allow_multiple_selects);

    if (!input_path) {
        fprintf(stderr, "Too bad no files\n");
        return false;
    }
    char c;
    Nob_Cmd to_free = {0};
    Nob_String_Builder buf = {0};
    nob_free_all(data->inputFiles);
    data->inputFiles.count = 0;

    // parse files
    for (int i = 0; ; i++) {
        c = input_path[i];
        if (c == MULTI_SEPARATOR || c == '\0') {
            nob_sb_append_null(&buf);
            // moving buffer to another place on the heap
            char *pointer = strdup(buf.items);
            nob_cmd_append(&data->inputFiles, pointer);
            nob_cmd_append(&to_free, pointer);
            buf.count = 0;
        } else {
            nob_sb_append_buf(&buf, &c, 1);
        }
        if (c == '\0') break;
    }
    nob_sb_free(buf);
    return true;
}

int RunMagick(ClayVideoDemo_Data *data) {
    if (data->inputFiles.count == 0) {
        fprintf(stderr, "No files to process\n");
        return false;
    }

    Nob_Cmd cmd = {0};
    Nob_Cmd to_free = {0};
    Nob_String_Builder buf = {0};
    bool free_buf = false;
    char *ashlar_path = malloc(strlen(ASHLAR_PREFIX) + data->outputFile.count);
    *ashlar_path = '\0';
    strcat(ashlar_path, ASHLAR_PREFIX);
    strcat(ashlar_path, data->outputFile.items);

    nob_cmd_append(&cmd, "magick");
    nob_cmd_append(&cmd, "-verbose");

    nob_cmd_extend(&cmd, &data->inputFiles);
    // resize images
    // NOTE: should be after all images
    char *resize_str = NULL;
    if (data->state & MAGICK_RESIZE) {
        // https://usage.imagemagick.org/resize
        resize_str = malloc(strlen(data->resize_str.h) + strlen(data->resize_str.w) + 2 + MODIFIERS); // 'x', '\0' and modifiers
        *resize_str = '\0';
        strcat(resize_str, data->resize_str.w);
        strcat(resize_str, "x");
        strcat(resize_str, data->resize_str.h);

        if (data->state & MAGICK_IGNORE_RATIO) // 1
            strcat(resize_str, MAGICK_RESIZE_IGNORE_RATIO);
        if (data->state & MAGICK_SHRINK_LARGER) // 2
            strcat(resize_str, MAGICK_RESIZE_SHRINK_LARGER);
        else if (data->state & MAGICK_ENLARGE_SMALLER)
            strcat(resize_str, MAGICK_RESIZE_ENLARGE_SMALLER);
        if (data->state & MAGICK_FILL_AREA) // 3
            strcat(resize_str, MAGICK_RESIZE_FILL_AREA);
        // if (data->state & MAGICK_PERCENTAGE)
        //     strcat(resize_str, MAGICK_RESIZE_PERCENTAGE);
        // if (data->state & MAGICK_PIXEL_LIMIT)
        //     strcat(resize_str, MAGICK_RESIZE_PIXEL_LIMIT);
        nob_cmd_append(&cmd, "-resize", resize_str);
    }
    if (data->state & MAGICK_TRANSPARENT_BG) {
        nob_cmd_append(&cmd, "-background", "transparent");
    } else {
        buf.count = 0;
        nob_sb_append_cstr(&buf, "rgba(");
        nob_sb_append_cstr(&buf, data->color_str.r);
        nob_sb_append_cstr(&buf, ",");
        nob_sb_append_cstr(&buf, data->color_str.g);
        nob_sb_append_cstr(&buf, ",");
        nob_sb_append_cstr(&buf, data->color_str.b);
        nob_sb_append_cstr(&buf, ",");
        // 1.00 - 5 chars
        char alpha[5] = {0};
        sprintf(alpha, "%0.2f", (float)data->color.a/100);
        nob_sb_append_cstr(&buf, alpha);
        nob_sb_append_cstr(&buf, ")");
        nob_sb_append_null(&buf);

        char* pointer = strdup(buf.items);
        nob_cmd_append(&to_free, pointer);
        nob_cmd_append(&cmd, "-background", pointer);
        free_buf = true;
    }
    if (data->state & MAGICK_BEST_FIT)
        nob_cmd_append(&cmd, "-define", "ashlar:best-fit=true");
    if (data->tempDir.count > 0) {
        nob_cmd_append(&cmd,"-define");
        buf.count = 0;
        nob_sb_append_cstr(&buf, "registry:temporary-path=");
        nob_sb_append_buf(&buf, data->tempDir.items, data->tempDir.count);
        nob_sb_append_null(&buf);
        char* temp_path = strdup(buf.items);
        nob_cmd_append(&to_free, temp_path);
        nob_cmd_append(&cmd, temp_path);
        free_buf = true;
    }
    nob_cmd_append(&cmd, "-gravity", "center");
    nob_cmd_append(&cmd, ashlar_path);

    bool ret = true;
    data->magickProc = nob_cmd_run_async(cmd);

    // Freeing memory
    free(ashlar_path);
    if (data->state & MAGICK_RESIZE)
        free(resize_str);
    if (free_buf)
        nob_sb_free(buf);
    nob_free_all(to_free);
    nob_cmd_free(to_free);
    nob_cmd_free(cmd);
    return ret;
}

void* RunMagickThread(void *arg) {
    ClayVideoDemo_Data *data = (ClayVideoDemo_Data *) arg;
    data->threadRunning = true;
    RunMagick(data);
    data->threadRunning = false;
    return NULL;
}

void ChangeOutputPath(ClayVideoDemo_Data *data) {
    const char *filter_params[] = {"*.png", "*.avif", "*.jpg", "*"};
    char *output_path = tinyfd_saveFileDialog("Path to output image", NULL, sizeof(filter_params) / sizeof(filter_params[0]), filter_params, "Image files");
    if (!output_path) {
        fprintf(stderr, "Too bad, no output files\n");
        return;
    }
    if (output_path[0] != '\0') {
        // Yeah, this buffer contains nothing, u can use it!
        data->outputFile.count = 0;
        nob_sb_append_cstr(&data->outputFile, output_path);
        nob_sb_append_null(&data->outputFile);
    }
}

void SetTempDir(Nob_String_Builder *sb) {
    char *output_path = tinyfd_selectFolderDialog("Path to a location for temporary files", "./");
    if (!output_path) {
        fprintf(stderr, "Too bad, no output tempdir\n");
        return;
    }
    if (output_path[0] != '\0') {
        // Yeah, this buffer contains nothing, u can use it!
        sb->count = 0;
        nob_sb_append_cstr(sb, output_path);
        nob_sb_append_null(sb);
        fprintf(stderr, "Set tmp dir: %s\n", sb->items);
    }
}

// https://gitlab.com/camconn/fontconfig-example
int defaultFont(char** res) {
    FcConfig*       conf;
    FcFontSet*      fs;
    FcObjectSet*    os = 0;
    FcPattern*      pat;
    FcResult        result;

    conf = FcInitLoadConfigAndFonts();
    pat = FcNameParse((FcChar8*) "");

    if (!pat) {
        fprintf(stderr, "Pattern is empty\n");

        return -1;
    }

    FcConfigSubstitute(conf, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    fs = FcFontSetCreate();
    os = FcObjectSetBuild(FC_FILE, (char*)0);

    FcFontSet *font_patterns;
    font_patterns = FcFontSort(conf, pat, FcTrue, 0, &result);

    if (!font_patterns || font_patterns->nfont == 0) {
        fprintf(stderr, "Fontconfig could not find ANY fonts on the system?\n");
        return -1;
    }

    FcPattern *font_pattern;
    font_pattern = FcFontRenderPrepare(conf, pat, font_patterns->fonts[0]);
    if (font_pattern){
        FcFontSetAdd(fs, font_pattern);
    } else {
        fprintf(stderr, "Could not prepare matched font for loading.\n");
        return -1;
    }

    FcFontSetSortDestroy(font_patterns);
    FcPatternDestroy(pat);

    if (fs) {
        if (fs->nfont > 0) {
            FcValue v;
            FcPattern *font;

            font = FcPatternFilter(fs->fonts[0], os);

            FcPatternGet(font, FC_FILE, 0, &v);
            char* filepath = (char*)v.u.f;
            *res = strdup(filepath);
            FcPatternDestroy(font);
        }
        FcFontSetDestroy(fs);
    } else {
        fprintf(stderr, "could not obtain fs\n");
    }

    if (os)
        FcObjectSetDestroy(os);

    return 0;
}

int8_t scrollDirection(Vector2 scrollDelta) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || scrollDelta.y > 0 || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP))
        return 1;
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || scrollDelta.y < 0 || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN))
        return -1;
    return 0;
}

AppState GetAppState() {
    AppState state = {
        .mousePosition = GetMousePosition(),
        .scrollDelta = GetMouseWheelMoveV(),
        .renderHeight = GetRenderHeight(),
        .renderWidth = GetRenderWidth(),
        // .renderHeight = GetScreenHeight(),
        // .renderWidth = GetScreenWidth(),
    };
    return state;
}

bool VectorsEqual(Vector2 *this, Vector2 *that) {
    if (this->x != that->x) return false;
    if (this->y != that->y) return false;
    return true;
}

bool StatesEqual(AppState *this, AppState *that) {
    if (!VectorsEqual(&this->mousePosition, &that->mousePosition)) return false;
    if (!VectorsEqual(&this->scrollDelta, &that->scrollDelta)) return false;
    if (this->renderWidth != that->renderWidth) return false;
    if (this->renderHeight != that->renderHeight) return false;
    return true;
}

Clay_RenderCommandArray CreateLayout(Clay_Context* context, ClayVideoDemo_Data *data, AppState appstate) {
    Clay_SetCurrentContext(context);
    // Run once per frame
    Clay_SetLayoutDimensions(CLAY_DIMENSIONS);
    Vector2 mousePosition = appstate.mousePosition;
    Vector2 scrollDelta = appstate.scrollDelta;
    Clay_SetPointerState(
            (Clay_Vector2) { mousePosition.x, mousePosition.y },
            IsMouseButtonDown(0)
    );
    Clay_UpdateScrollContainers(
            true,
            (Clay_Vector2) { scrollDelta.x, scrollDelta.y },
            GetFrameTime()
    );

    Nob_Cmd cmd = {0};

    if (data->state & MAGICK_OPEN_ON_DONE && data->magickProc != NOB_INVALID_PROC && !nob_proc_running(data->magickProc)) {
        nob_cmd_append(&cmd, "xdg-open", data->outputFile.items);
        nob_cmd_run_async_silent(cmd);
        data->magickProc = NOB_INVALID_PROC;
    }
    if (released("Quit")) {
        data->shouldClose = true;
        printf("close\n");
    } else if (released("Support")) {
        nob_cmd_append(&cmd, "xdg-open", SUPPORT_URL);
        nob_cmd_run_async_silent(cmd);
    } else if (released("Select Images")) {
        GetInputFiles(data);
        nob_kill(&data->magickProc);
        RunMagickThreaded();
    } else if (released(SELECT_TEMP)) {
        SetTempDir(&data->tempDir);
    } else if (released("Run")) {
        RunMagickThreaded();
    } else if (released("Stop")) {
        nob_kill(&data->magickProc);
        cthreads_thread_ensure_cancelled(data->magickThread, &data->threadRunning);
    } else if (released("file")) {
        printf("befor: %s\n", data->outputFile.items);
        ChangeOutputPath(data);
        printf("aftir: %s\n", data->outputFile.items);
    } else if (released("Open result")) {
        if (data->outputFile.items[0] != '\0') {
            nob_cmd_append(&cmd, "xdg-open", data->outputFile.items);
            nob_cmd_run_async_silent(cmd);
            nob_cmd_free(cmd);
        } else {
            fprintf(stderr, "No file was made\n");
        }
    // Number pickers handling
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ResizeAll")))) {
        int8_t scroll = scrollDirection(scrollDelta);
        // Don't update in no interaction happened
        if (scroll != 0) {
            data->resize.h = data->resize.w = data->resize.w + 50 * scroll;
            sprintf(data->resize_str.w, "%d", data->resize.w);
            sprintf(data->resize_str.h, "%d", data->resize.h);
        }
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ResizeW")))) {
        data->resize.w += 50 * scrollDirection(scrollDelta);
        sprintf(data->resize_str.w, "%d", data->resize.w);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ResizeH")))) {
        data->resize.h += 50 * scrollDirection(scrollDelta);
        sprintf(data->resize_str.h, "%d", data->resize.h);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ResizeEach")))) {
        data->resize.w += 50 * scrollDirection(scrollDelta);
        data->resize.h += 50 * scrollDirection(scrollDelta);
        sprintf(data->resize_str.w, "%d", data->resize.w);
        sprintf(data->resize_str.h, "%d", data->resize.h);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("r")))) {
        data->color.r += scrollDirection(scrollDelta);
        sprintf(data->color_str.r, "%d", data->color.r);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("g")))) {
        data->color.g += scrollDirection(scrollDelta);
        sprintf(data->color_str.g, "%d", data->color.g);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("b")))) {
        data->color.b += scrollDirection(scrollDelta);
        sprintf(data->color_str.b, "%d", data->color.b);
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("a")))) {
        data->color.a += scrollDirection(scrollDelta);
        // guess what
        // we dont have a tool for that called %
        if (data->color.a == 255) data->color.a = 100;
        if (data->color.a > 100) data->color.a = 0;  // wrap
        sprintf(data->color_str.a, "%d", data->color.a);
    }

    return ClayVideoDemo_CreateLayout(data);
}

bool testMagick() {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "magick", "-version");

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        fprintf(stderr, "Bro, where is yr magick??\n");
        nob_cmd_free(cmd);
        return false;
    }
    nob_cmd_free(cmd);
    return true;
}

int main(void) {
    if (!testMagick()) {
        // yes, this is an mdash
        fprintf(stderr, "Sorry, no magick — no worky\n");
        return -1;
    }
    char* title = "Magick deez nuts";
    // vsync makes resizes slower, we don't want this
    // but antialiasing is nice
    //
    // HIGHDPI with raylib and SDL backend crops text weirdly
    // without HIGHDPI GetScreen... and GetRender... are identical
    // raylib with GLFW: bad crop every time
    // HIGHDPI scales everything nicely
    //
    Clay_Raylib_Initialize(1024, 768, title, FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    // Clay_Raylib_Initialize(1024, 768, title, FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);

    Font fonts[1];
    char *fontpath = {0};
    if (defaultFont(&fontpath) == -1) {
        return -1;
    }
    // https://stackoverflow.com/questions/73248125/raylibs-drawtextex-doesnt-display-unicode-characters
    int codepoints[512] = {0};
    for (int i = 0; i < 95; i++) codepoints[i] = 32 + i;
    for (int i = 0; i < 255; i++) codepoints[96 + i] = 0x400 + i;
    fonts[FONT_ID_BODY_16] = LoadFontEx(fontpath, FONT_LOAD_SIZE, codepoints, 512);
    free(fontpath);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);

    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));
    Clay_Context *clayContext = Clay_Initialize(clayMemory, CLAY_DIMENSIONS, (Clay_ErrorHandler) { HandleClayErrors, NULL }); // This final argument is new since the video was published
    ClayVideoDemo_Data data = ClayVideoDemo_Initialize();
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    // Disable ESC to exit
    SetExitKey(KEY_NULL);
#ifdef TESTING
    // Clay_SetDebugModeEnabled(true);
#endif
#ifdef LAZY_RENDER
    AppState old_state = {0};
    int prevtime = SLEEP_THRESHOLD;
    bool are_equal = false;
#endif // LAZY_RENDER
    AppState state = {0};
    while (!WindowShouldClose() &&
           !data.shouldClose &&
           !((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_Q))
          ) {
        // Update the app's state
#ifdef LAZY_RENDER
        old_state = state;
#endif // LAZY_RENDER
        state = GetAppState();
        Clay_RenderCommandArray renderCommands = CreateLayout(clayContext, &data, state);
        BeginDrawing();
        // ClearBackground(BLACK);
#ifdef LAZY_RENDER
        int time = (int)GetTime();

        are_equal = StatesEqual(&state, &old_state);
        if (!are_equal) {
            prevtime = time + SLEEP_THRESHOLD;
        }
        // if we might not care to render anything anymore
        if (time > prevtime) {
#ifdef DEBUG
            fprintf(stderr, "The time has come\n");
#endif // DEBUG
            do {
                // Refresh raylib state (usually happends on EndDrawing())
                PollInputEvents();

                // Don't sleep, just exit
                if ((data.shouldClose = WindowShouldClose())) break;
                state = GetAppState();
                if (!(are_equal = StatesEqual(&state, &old_state))) break;
                old_state = state;
#ifdef DEBUG
                fprintf(stderr, "Not rendering: %d\n", time);
#endif // DEBUG
                WaitTime(1);
            } while (true);
            // Look! it moved!
            prevtime = time + SLEEP_THRESHOLD;
        }
#endif // LAZY_RENDER

        Clay_Raylib_Render(renderCommands, fonts);
#ifdef TESTING
        // DrawFPS(0,0);
#endif // TESTING
        EndDrawing();
    }
    fprintf(stderr, "Cancelling threads\n");

    Clay_Raylib_Close();
}
