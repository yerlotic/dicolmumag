#include <stdio.h>
#include <unistd.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "raylib/raylib.h"
#include "raylib/clay_renderer_raylib.c"
#include "layout.c"
#include <linux/limits.h>
#include <assert.h>
#include <fontconfig/fontconfig.h>
#define SUPPORT_URL "https://youtu.be/dQw4w9WgXcQ"

#define TESTING

// ascii file separator
// thx ascii!
// #define MULTI_SEPARATOR '\x1c'
#define MULTI_SEPARATOR '|'
#include "tinyfiledialogs.c"

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

#define pressed(id) IsMouseButtonPressed(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id)))
#define released(id) IsMouseButtonReleased(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id)))

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("[CLAY] [ERROR] %s\n", errorData.errorText.chars);
}

int runMagick(ClayVideoDemo_Data *data) {
    int allow_multiple_selects = true;
    char const *filter_params[] = {"*.png", "*.svg", "*.jpg", "*.jpeg"};
    char *input_path = tinyfd_openFileDialog("Path to image", "./", 4, filter_params, "Image file", allow_multiple_selects);
    char *output_path = "res.png";
    char *ashlar_path = malloc(strlen(output_path) + strlen("ashlar:") + 1);
    *ashlar_path = '\0';
    strcat(ashlar_path, "ashlar:");
    strcat(ashlar_path, output_path);

    if (!input_path) {
        fprintf(stderr, "Too bad no files\n");
        free(ashlar_path);
        return false;
    }
    char c;
    Nob_Cmd cmd = {0};
    Nob_Cmd to_free = {0};
    Nob_String_Builder buf = {0};
    nob_cmd_append(&cmd, "magick");
    nob_cmd_append(&cmd, "-verbose");

    for (int i = 0; ; i++) {
        c = input_path[i];
        if (c == MULTI_SEPARATOR || c == '\0') {
            nob_sb_append_null(&buf);
            // moving buffer to another place on the heap
            char *pointer = strcpy(malloc(sizeof(buf.items) * buf.count), buf.items);
            nob_cmd_append(&cmd, pointer);
            nob_cmd_append(&to_free, pointer);
            buf.count = 0;
        } else {
            nob_sb_append_buf(&buf, &c, 1);
        }
        if (c == '\0') break;
    }

    char *resize_str = malloc(2 * strlen(data->resize_str) + 3); // x, \0 and modifier maybe
    *resize_str = '\0';
    strcat(resize_str, data->resize_str);
    strcat(resize_str, "x");
    strcat(resize_str, data->resize_str);
    // strcat(resize_str, "x"); // modifier
    nob_cmd_append(&cmd, "-resize", resize_str);
    if (data->state & MAGICK_TRANSPARENT_BG)
        nob_cmd_append(&cmd, "-background", "transparent");
    if (data->state & MAGICK_BEST_FIT)
        nob_cmd_append(&cmd, "-define", "ashlar:best-fit=true");
    nob_cmd_append(&cmd, "-gravity", "center");
    nob_cmd_append(&cmd, ashlar_path);
    fprintf(stderr, "First : %s\n", cmd.items[0]);
    fprintf(stderr, "Second: %s\n", cmd.items[1]);

    bool ret = true;
    if (nob_cmd_run_sync_and_reset(&cmd)) {
        data->resultFile = output_path;
        if (data->state & MAGICK_OPEN_ON_DONE) {
            nob_cmd_append(&cmd, "xdg-open", output_path);
            nob_cmd_run_async(cmd);
        }
    } else {
        fprintf(stderr, "Too bad, no result\n");
        ret = false;
    }
    // Freeing memory
    free(ashlar_path);
    free(resize_str);
    for (size_t i = 0; i < to_free.count; i++) {
        // free(&to_free.items[i]);
        printf("char: %c, pointer: %p\n", *to_free.items[i], to_free.items[i]);
        printf("string: %s\n", to_free.items[i]);
        free((void *) to_free.items[i]);
    }
    nob_cmd_free(cmd);
    nob_cmd_free(to_free);
    return ret;
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

Clay_RenderCommandArray CreateLayout(Clay_Context* context, ClayVideoDemo_Data *data) {
    Clay_SetCurrentContext(context);
#ifdef TESTING
    // Clay_SetDebugModeEnabled(true);
#endif
    // Run once per frame
    Clay_SetLayoutDimensions((Clay_Dimensions) {
            .width = GetScreenWidth(),
            .height = GetScreenHeight()
    });
    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
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
    sprintf(data->resize_str, "%d", data->resize);
    if (released("Quit")) {
        data->shouldClose = true;
        printf("close\n");
    } else if (released("Support")) {
        nob_cmd_append(&cmd, "xdg-open", SUPPORT_URL);
        nob_cmd_run_async(cmd);
    } else if (released("Select_Images")) {
        runMagick(data);
    } else if (released("Open result")) {
        if (data->resultFile[0] != '\0') {
            nob_cmd_append(&cmd, "xdg-open", data->resultFile);
            nob_cmd_run_async(cmd);
            nob_cmd_free(cmd);
        } else {
            fprintf(stderr, "No file was made\n");
        }
    } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("Resize")))) {
        if (IsMouseButtonPressed(0) || scrollDelta.y > 0 || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP))
            data->resize += 50;
        else if (IsMouseButtonPressed(1) || scrollDelta.y < 0 || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN))
            data->resize -= 50;
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
    return true;
}

int main(void) {
    if (!testMagick()) {
        fprintf(stderr, "Sorry, no magick — no worky\n");
        return -1;
    }
    char* title = "Magick deez nuts";
    // vsync makes resizes slower, we don't want this
    // but antialiasing is nice
    Clay_Raylib_Initialize(1024, 768, title, FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

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
    Clay_Context *clayContext = Clay_Initialize(clayMemory, (Clay_Dimensions) {
       .width = GetScreenWidth(),
       .height = GetScreenHeight()
    }, (Clay_ErrorHandler) { HandleClayErrors }); // This final argument is new since the video was published
    ClayVideoDemo_Data data = ClayVideoDemo_Initialize();
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    while (!WindowShouldClose()) {
        Clay_RenderCommandArray renderCommandsTop = CreateLayout(clayContext, &data);
        if (data.shouldClose) break;

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(renderCommandsTop, fonts);
#ifdef TESTING
        DrawFPS(0,0);
#endif // TESTING
        EndDrawing();
    }

    Clay_Raylib_Close();
}
