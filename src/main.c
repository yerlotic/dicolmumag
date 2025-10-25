#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#define SUPPORT_URL "https://youtu.be/dQw4w9WgXcQ"
#ifdef _WIN32
#include "thirdparty/raylib/fix_win32_compatibility.h"
#include <processthreadsapi.h>
#else
#include <fontconfig/fontconfig.h>
#include <linux/limits.h>
#endif // _WIN32

#define CLAY_IMPLEMENTATION
#include "thirdparty/clay.h"
#include "thirdparty/raylib/raylib.h"
#include "thirdparty/raylib/clay_renderer_raylib.c"

#include "thirdparty/nob.h"
#define CTHREADS_DEBUG
#include "thirdparty/cthreads.h"
#include "thirdparty/cthreads.c"
#include "layout.c"
// ascii file separator
// thx ascii!
// #define MULTI_SEPARATOR '\x1c'
#define MULTI_SEPARATOR '|'
#include "thirdparty/tinyfiledialogs.c"

#ifndef FALLBACK_FPS
#define FALLBACK_FPS 60
#endif // FALLBACK_FPS

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
#ifndef NO_SCALING
    float scale;
#endif // NO_SCALING
    uint8_t language;
    uint16_t tabWidth;
} AppState;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define pressed(id)  (IsMouseButtonPressed(0)  && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id))))
#define released(id) (IsMouseButtonReleased(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING(id))))
#define pressed_s(id)  (IsMouseButtonPressed(0)  && Clay_PointerOver(Clay_GetElementId((id))))
#define released_s(id) (IsMouseButtonReleased(0) && Clay_PointerOver(Clay_GetElementId((id))))
#define CLAY_DIMENSIONS (Clay_Dimensions) { .width = GetScreenWidth(), \
                                            .height = GetScreenHeight() }
// #define CLAY_DIMENSIONS (Clay_Dimensions) { .width = GetRenderWidth(), .height = GetRenderHeight() }
#ifdef NO_THREADING
  #define RunMagickThreaded() \
    data->selectedDocumentIndex = MAGICK_ADVANCED_SETTINGS; \
    RunMagick(&data->params)
#else
  #define RunMagickThreaded() \
    data->selectedDocumentIndex = MAGICK_ADVANCED_SETTINGS; \
    fprintf(stderr, "Cancelling thread\n"); \
    nob_kill(&data->params.magickProc); \
    cthreads_thread_ensure_cancelled(data->magickThread, &data->params.threadRunning); \
    fprintf(stderr, "Starting thread\n"); \
    cthreads_thread_create(&data->magickThread, NULL, RunMagickThread, (void *)data, &data->threadArgs); \
    fprintf(stderr, "Thread STARTED\n")
#endif // NO_THREADING


void HandleClayErrors(Clay_ErrorData errorData) {
    printf("[CLAY] [ERROR] %s\n", errorData.errorText.chars);
}

// Don't forget to free the thing itself if u need to
#define nob_free_all(to_free) do {                                  \
    for (size_t i = 0; i < (to_free)->count; i++) {                \
        fprintf(stderr, "Freeing: \"%s\"\n", (to_free)->items[i]); \
        free((void *) (to_free)->items[i]);                        \
    }                                                            \
} while (0)

// Declarations
bool testMagick(char* magickBin);

void nob_kill(Nob_Proc *proc) {
    fprintf(stderr, "nob_kill\n");
    if (*proc == NOB_INVALID_PROC) return;
    fprintf(stderr, "\033[31mkilling\033[0m\n");
#ifdef _WIN32
    TerminateProcess(*proc, 69);
    WaitForSingleObject(*proc, INFINITE);
#else
    kill(*proc, SIGKILL);
#endif // _WIN32
    // *proc = NOB_INVALID_PROC;
    // Dont do that ^ cuz status should be collected
    // so processes don't become zombies
}

ProcStatus nob_proc_running(Nob_Proc proc) {
    if (proc == NOB_INVALID_PROC) {
        fprintf(stderr, "Got invalid proc\n");
        return PROCESS_EXITED_OK;
    }
#ifdef _WIN32
    // LPDWORD exit_code = 0;
    // BOOL success = GetExitCodeProcess(proc, exit_code);
    // if (!success) {
    //     fprintf(stderr, "Could not get exit code of a process: %s\n", nob_win32_error_message(GetLastError()));
    //     CloseHandle(proc);
    //     return PROCESS_OS_BULLSHIT;
    // }
    // fprintf(stderr, "Code: %p\n", exit_code);
    // if (exit_code == NULL) {
    //     return PROCESS_RUNNING;
    // }
    //
    // fprintf(stderr, "Returning STILL_ACTIVE\n");
    // if (*exit_code == STILL_ACTIVE)
    //     return PROCESS_RUNNING;
    // fprintf(stderr, "Returning PROCESS_CRASHED\n");
    // CloseHandle(proc);
    // return PROCESS_CRASHED;

    if (WaitForSingleObject(proc, 0) == WAIT_TIMEOUT)
        return PROCESS_RUNNING;

    DWORD exit_status;
    if (GetExitCodeProcess(proc, &exit_status) == 0) {
        fprintf(stderr, "Could not get process exit code. System Error Code: %s\n", nob_win32_error_message(GetLastError()));
        CloseHandle(proc);
        return PROCESS_OS_BULLSHIT;
    }

    if (exit_status != 0) {
        fprintf(stderr, "Command exited with exit code %lu\n", exit_status);
        CloseHandle(proc);
        if (exit_status == 69)
            return PROCESS_WAS_TERMINATED;
        return PROCESS_CRASHED;
    }

    CloseHandle(proc);
    return PROCESS_EXITED_OK;

#else

    int status;
    pid_t return_pid = waitpid(proc, &status, WNOHANG);
    if (return_pid == -1) {
        fprintf(stderr, "whoa! a crash in waitpid or sth\n");
        return PROCESS_CRASHED;
    } else if (return_pid == 0) {
        /* child is still running */
        return PROCESS_RUNNING;
    } else if (return_pid == proc) {
        // child stopped running
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 0) {
                return PROCESS_EXITED_OK;
            } else {
                fprintf(stderr, "command exited with exit code %d:", exit_status);
                return PROCESS_CRASHED;
            }
        }

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "command process was terminated by signal %d", WTERMSIG(status));
            return PROCESS_WAS_TERMINATED;
        }
    }
#endif // _WIN32
    fprintf(stderr, "Reached the end of nob_proc_running\n");
    return PROCESS_CRASHED;
}

void cthreads_thread_ensure_cancelled(struct cthreads_thread thread, ProcStatus *running) {
    if (running == PROCESS_RUNNING) {
        cthreads_thread_cancel(thread);
        *running = PROCESS_WAS_TERMINATED;
    }
}

#ifdef _WIN32
#define BLACKHOLE "nul"
#define LAUNCHER "explorer.exe"
#else
#define BLACKHOLE "/dev/null"
#define LAUNCHER  "xdg-open"
#endif // _WIN32
#define nob_cmd_run_async_silent(cmd) do { \
    Nob_Fd blackhole = nob_fd_open_for_write(BLACKHOLE); \
    nob_cmd_run_async_redirect(cmd, (Nob_Cmd_Redirect) {.fdout = &blackhole, .fderr = &blackhole}); \
} while(0)

MagickStatus GetInputFiles(Nob_Cmd *inputFiles) {
    int allow_multiple_selects = true;
    // int allow_multiple_selects = false;

    // TODO: wrap this into a macro
#ifdef _WIN32
    wchar_t const *filter_params[] = {L"*.png", L"*.svg", L"*.jpg", L"*.jpeg"};
    wchar_t *input_path = tinyfd_openFileDialogW(
        L"Path to images",
        L"./",
        sizeof(filter_params) / sizeof(filter_params[0]),
        filter_params,
        L"Image file",
        allow_multiple_selects);
#else
    char const *filter_params[] = {"*.png", "*.svg", "*.jpg", "*.jpeg"};
    char *input_path = tinyfd_openFileDialog(
        "Path to images",
        "./",
        sizeof(filter_params) / sizeof(filter_params[0]),
        filter_params,
        "Image file",
        allow_multiple_selects);
#endif // _WIN32

    if (!input_path) {
        fprintf(stderr, "Too bad no input files\n");
        return MAGICK_ERROR_CANCELLED;
    }
#ifdef _WIN32
    char *utf8_text;
    wchar_t c;
#define ZERO L'\0'
#else
    char c;
#define ZERO '\0'
#endif // _WIN32
    Nob_Cmd to_free = {0};
    Nob_String_Builder buf = {0};
    nob_free_all(inputFiles);
    inputFiles->count = 0;

    // parse files
    for (int i = 0; ; i++) {
        c = input_path[i];
        if (c == MULTI_SEPARATOR || c == ZERO) {
            nob_sb_append_null(&buf);
            // moving buffer to another place on the heap
            char *pointer = strdup(buf.items);
            fprintf(stderr, "File: %s\n", pointer);
            nob_cmd_append(inputFiles, pointer);
            nob_cmd_append(&to_free, pointer);
            buf.count = 0;
        } else {
            #ifdef _WIN32
            int utf8Size = 0;
            utf8_text = CodepointToUTF8(c, &utf8Size);
            nob_sb_append_buf(&buf, utf8_text, utf8Size);
            #else // POSIX
            nob_sb_append_buf(&buf, &c, 1);
            #endif // _WIN32
        }
        if (c == ZERO) break;
    }
    nob_sb_free(buf);
    return MAGICK_ERROR_OK;
#undef ZERO
}

MagickStatus RunMagick(magick_params_t *params) {
    if (params->inputFiles.count == 0) {
        fprintf(stderr, "No files to process\n");
        return MAGICK_ERROR_NO_FILES;
    }

    // outputFile, inputFiles, magickBinary, state, resize_str, color_str, color, tempDir, gravity, magickProc
    Nob_Cmd cmd = {0};
    Nob_Cmd to_free = {0};
    Nob_String_Builder buf = {0};
    bool free_buf = false;
    Nob_String_Builder ashlar_path = {0};
    nob_sb_append_cstr(&ashlar_path, ASHLAR_PREFIX);
    // There should always me '\0' at the end of outputFile
    nob_sb_append_cstr(&ashlar_path, params->outputFile.items);
    if (params->state & MAGICK_SET_RESOLUTION) {
        nob_sb_append_buf(&ashlar_path, "[", 1);
        nob_sb_append_cstr(&ashlar_path, params->resizes[RESIZES_OUTPUT_RES].str.w);
        nob_sb_append_buf(&ashlar_path, "x", 1);
        nob_sb_append_cstr(&ashlar_path, params->resizes[RESIZES_OUTPUT_RES].str.h);

        // Casting cuz it can be negative
        if ((int8_t) params->resizes[RESIZES_OUTPUT_MARGIN].values.w >= 0)
            nob_sb_append_buf(&ashlar_path, "+", 1);
        nob_sb_append_cstr(&ashlar_path, params->resizes[RESIZES_OUTPUT_MARGIN].str.w);

        if ((int8_t) params->resizes[RESIZES_OUTPUT_MARGIN].values.h >= 0)
            nob_sb_append_buf(&ashlar_path, "+", 1);
        nob_sb_append_cstr(&ashlar_path, params->resizes[RESIZES_OUTPUT_MARGIN].str.h);

        nob_sb_append_buf(&ashlar_path, "]", 1);
    }
    nob_sb_append_null(&ashlar_path);
    fprintf(stderr, "Ashlar: %s\n", ashlar_path.items);

    nob_cmd_append(&cmd, params->magickBinary.items);
    nob_cmd_append(&cmd, "-verbose");

    nob_cmd_extend(&cmd, &params->inputFiles);
    // resize images
    // NOTE: should be after all images
    char *resize_str = NULL;
    if (params->state & MAGICK_RESIZE) {
        // https://usage.imagemagick.org/resize
        resize_str = malloc(strlen(params->resizes[RESIZES_INPUT].str.h) + strlen(params->resizes[RESIZES_INPUT].str.w) + 2 + MODIFIERS); // 'x', '\0' and modifiers
        *resize_str = '\0';
        strcat(resize_str, params->resizes[RESIZES_INPUT].str.w);
        strcat(resize_str, "x");
        strcat(resize_str, params->resizes[RESIZES_INPUT].str.h);

        if (params->state & MAGICK_IGNORE_RATIO) // 1
            strcat(resize_str, MAGICK_RESIZE_IGNORE_RATIO);
        if (params->state & MAGICK_SHRINK_LARGER) // 2
            strcat(resize_str, MAGICK_RESIZE_SHRINK_LARGER);
        else if (params->state & MAGICK_ENLARGE_SMALLER)
            strcat(resize_str, MAGICK_RESIZE_ENLARGE_SMALLER);
        if (params->state & MAGICK_FILL_AREA) // 3
            strcat(resize_str, MAGICK_RESIZE_FILL_AREA);
        // if (params->state & MAGICK_PERCENTAGE)
        //     strcat(resize_str, MAGICK_RESIZE_PERCENTAGE);
        // if (params->state & MAGICK_PIXEL_LIMIT)
        //     strcat(resize_str, MAGICK_RESIZE_PIXEL_LIMIT);
        nob_cmd_append(&cmd, "-resize", resize_str);
    }
    if (params->state & MAGICK_TRANSPARENT_BG) {
        nob_cmd_append(&cmd, "-background", "transparent");
    } else {
        buf.count = 0;
        nob_sb_append_cstr(&buf, "rgba(");
        nob_sb_append_cstr(&buf, params->color_str.r);
        nob_sb_append_cstr(&buf, ",");
        nob_sb_append_cstr(&buf, params->color_str.g);
        nob_sb_append_cstr(&buf, ",");
        nob_sb_append_cstr(&buf, params->color_str.b);
        nob_sb_append_cstr(&buf, ",");
        // 1.00 - 5 chars
        char alpha[5] = {0};
        sprintf(alpha, "%0.2f", (float)params->color.a/100);
        nob_sb_append_cstr(&buf, alpha);
        nob_sb_append_cstr(&buf, ")");
        nob_sb_append_null(&buf);

        char* pointer = strdup(buf.items);
        nob_cmd_append(&to_free, pointer);
        nob_cmd_append(&cmd, "-background", pointer);
        free_buf = true;
    }
    if (params->state & MAGICK_BEST_FIT)
        nob_cmd_append(&cmd, "-define", "ashlar:best-fit=true");
    if (params->tempDir.count > 0) {
        nob_cmd_append(&cmd,"-define");
        buf.count = 0;
        nob_sb_append_cstr(&buf, "registry:temporary-path=");
        nob_sb_append_buf(&buf, params->tempDir.items, params->tempDir.count);
        nob_sb_append_null(&buf);
        char* temp_path = strdup(buf.items);
        nob_cmd_append(&to_free, temp_path);
        nob_cmd_append(&cmd, temp_path);
        free_buf = true;
    }
    nob_cmd_append(&cmd, "-gravity", params->gravity.values[params->gravity.selected]);
    nob_cmd_append(&cmd, ashlar_path.items);

    MagickStatus res = MAGICK_ERROR_OK;
    fprintf(stderr, "Staring magick\n");
#ifdef NO_THREADING
    if (!nob_cmd_run_sync(cmd)) {
        res = MAGICK_ERROR_PROCESS_CRASHED;
        fprintf(stderr, "Running magick failed\n");
    } else {
        fprintf(stderr, "Running magick ok\n");
    }
#else
    if (params->magickProc == NOB_INVALID_PROC) {
        params->magickProc = nob_cmd_run_async(cmd);
        if (params->magickProc == NOB_INVALID_PROC)
            fprintf(stderr, "run_async returned invalid proc!!!!\n");
        fprintf(stderr, "Started magick\n");
    } else {
        fprintf(stderr, "BRO WTF how is here a race condition\n");
    }
#endif // NO_THREADING

    // Freeing memory
    nob_sb_free(ashlar_path);
    if (params->state & MAGICK_RESIZE)
        free(resize_str);
    if (free_buf)
        nob_sb_free(buf);
    nob_free_all(&to_free);
    nob_cmd_free(to_free);
    nob_cmd_free(cmd);
    fprintf(stderr, "RunMagick exited\n");
    return res;
}

void *RunMagickThread(void *arg) {
    AppData *data = (AppData *) arg;
    if (data->params.threadRunning == true)
        return NULL;
    // idk how to actually escape race conditions
    data->params.threadRunning = true;

    if (!testMagick(data->params.magickBinary.items)) {
        data->errorIndex = MAGICK_ERROR_NOT_WORK;
        return NULL;
    }
    fprintf(stderr, "errorIndex befor: %d\n", data->errorIndex);
    data->errorIndex = RunMagick(&data->params);
    fprintf(stderr, "errorIndex after: %d\n", data->errorIndex);
    data->params.threadRunning = false;
    return NULL;
}

MagickStatus ChangeMagickBinary(Nob_String_Builder *magickBin) {
#ifdef _WIN32
    const char *filter_params[] = {"*.exe", "*.msi", "*"};
#else
    const char *filter_params[] = {"*"};
#endif // _WIN32

    fprintf(stderr, "runnin\n");
    char *new_magick = tinyfd_openFileDialog("Path to magick executable", NULL, sizeof(filter_params) / sizeof(filter_params[0]), filter_params, "Executable files", false);

    if (!new_magick) {
        fprintf(stderr, "Too bad, no another magick binary\n");
        return MAGICK_ERROR_CANCELLED;
    }

    if (!testMagick(new_magick)) {
        fprintf(stderr, "Too bad, no another magick binary (\"%s\") cuz testing was too bad\n", new_magick);
        return MAGICK_ERROR_INVALID_BINARY_SELECTED;
    }

    // Yeah, this buffer contains nothing, u can use it!
    magickBin->count = 0;
    nob_sb_append_cstr(magickBin, new_magick);
    nob_sb_append_null(magickBin);

    return MAGICK_ERROR_OK;
}

void ChangeOutputPath(Nob_String_Builder *outputFile) {
    #ifdef _WIN32
    const wchar_t *filter_params[] = {L"*.png", L"*.avif", L"*.jpg", L"*"};
    // TODO: internationalization
    wchar_t *path = tinyfd_saveFileDialogW(L"Path to output image", NULL, sizeof(filter_params) / sizeof(filter_params[0]), filter_params, L"Image files");
    char *output_path = toUTF8(path, 0, NULL);
    #else // POSIX
    const char *filter_params[] = {"*.png", "*.avif", "*.jpg", "*"};
    char *output_path = tinyfd_saveFileDialog("Path to output image", NULL, sizeof(filter_params) / sizeof(filter_params[0]), filter_params, "Image files");
    #endif // _WIN32
    if (!output_path) {
        fprintf(stderr, "Too bad, no output files\n");
        return;
    }
    if (output_path[0] != '\0') {
        // Yeah, this buffer contains nothing, u can use it!
        outputFile->count = 0;
        nob_sb_append_cstr(outputFile, output_path);
        nob_sb_append_null(outputFile);
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
#ifdef _WIN32
    *res = ".\\font.ttf";
#else
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
#endif // _WIN32

    return 0;
}

int8_t scrollDirection(Vector2 scrollDelta) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || scrollDelta.y > 0 || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP))
        return 1;
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || scrollDelta.y < 0 || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN))
        return -1;
    return 0;
}

AppState GetAppState(AppData *data) {
    AppState state = {
        .mousePosition = GetMousePosition(),
        .scrollDelta = GetMouseWheelMoveV(),
        .renderHeight = GetRenderHeight(),
        .renderWidth = GetRenderWidth(),
#ifndef NO_SCALING
        .scale = scale,
#endif // NO_SCALING
        .language = language,
        .tabWidth = data->tabWidth,
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
#ifndef NO_SCALING
    if (this->scale != that->scale) return false;
#endif // NO_SCALING
    if (this->language != that->language) return false;
    if (this->tabWidth != that->tabWidth) return false;
    return true;
}

bool UpdateResizes(resize_element_t *resizes, int8_t scroll) {
    bool scrolled = false;

    // Handle one in header bar
    if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ResizeAll")))) {
        resizes[RESIZES_INPUT].values.h = resizes[RESIZES_INPUT].values.w = resizes[RESIZES_INPUT].values.w + 50 * scroll;
        sprintf(resizes[RESIZES_INPUT].str.w, "%d", resizes[RESIZES_INPUT].values.w);
        sprintf(resizes[RESIZES_INPUT].str.h, "%d", resizes[RESIZES_INPUT].values.h);
        return true;
    }

    resize_element_t *resize;
    uint8_t step;
    Nob_String_Builder sb = {0};
    // Number pickers handling (questionable)
    for (size_t i = 0; i < resizes_len; i++) {
        resize = &resizes[i];
        step = resize->step;

        if (!Clay_PointerOver(Clay_GetElementId(resize->id))) continue;
        scrolled = true;

        // All this could have been done with a macro
        // probably
        sb.count = 0;
        nob_sb_append_buf(&sb, resize->id.chars, resize->id.length);

        nob_sb_append_buf(&sb, "W", 1);
        if (Clay_PointerOver(Clay_GetElementId(CLAY_SB_STRING(sb)))) {
            resize->values.w += step * scroll;
            if (strncmp(resize->id.chars, ID_RESIZE_OUTPUT_MARGIN, resize->id.length) == 0)
                sprintf(resize->str.w, "%d", (int8_t) resize->values.w);
            else
                sprintf(resize->str.w, "%d", resize->values.w);
            break;
        }

        sb.count -= 1;
        nob_sb_append_buf(&sb, "H", 1);
        if (Clay_PointerOver(Clay_GetElementId(CLAY_SB_STRING(sb)))) {
            resize->values.h += step * scroll;
            if (strncmp(resize->id.chars, ID_RESIZE_OUTPUT_MARGIN, resize->id.length) == 0)
                sprintf(resize->str.h, "%d", (int8_t) resize->values.h);
            else
                sprintf(resize->str.h, "%d", resize->values.h);
            break;
        }

        sb.count -= 1;
        nob_sb_append_buf(&sb, "Each", 4);
        if (Clay_PointerOver(Clay_GetElementId(CLAY_SB_STRING(sb)))) {
            resize->values.w += step * scroll;
            resize->values.h += step * scroll;
            if (strncmp(resize->id.chars, ID_RESIZE_OUTPUT_MARGIN, resize->id.length) == 0) {
                sprintf(resize->str.w, "%d", (int8_t) resize->values.w);
                sprintf(resize->str.h, "%d", (int8_t) resize->values.h);
            } else {
                sprintf(resize->str.w, "%d", resize->values.w);
                sprintf(resize->str.h, "%d", resize->values.h);
            }
            break;
        }
    }
    nob_sb_free(sb);
    return scrolled;
}

Clay_RenderCommandArray CreateLayout(Clay_Context* context, AppData *data, AppState appstate) {
    Clay_SetCurrentContext(context);
    // Run once per frame
    Clay_SetLayoutDimensions((Clay_Dimensions){appstate.renderWidth, appstate.renderHeight});
    Vector2 mousePosition = appstate.mousePosition;
    Vector2 scrollDelta = appstate.scrollDelta;
    Clay_SetPointerState(
            (Clay_Vector2) { mousePosition.x, mousePosition.y },
            IsMouseButtonDown(0)
    );

    Nob_Cmd cmd = {0};

    if (data->params.state & MAGICK_OPEN_ON_DONE && data->params.magickProc != NOB_INVALID_PROC) {
        ProcStatus status = nob_proc_running(data->params.magickProc);
        if (status == PROCESS_EXITED_OK) {
            data->errorIndex = MAGICK_ERROR_OK;
            nob_cmd_append(&cmd, LAUNCHER, data->params.outputFile.items);
            nob_cmd_run_async_silent(cmd);
            cmd.count = 0;
        } else if (status == PROCESS_CRASHED) {
            data->errorIndex = MAGICK_ERROR_PROCESS_CRASHED;
        } else if (status == PROCESS_WAS_TERMINATED) {
            data->errorIndex = MAGICK_ERROR_PROCESS_TERMINATED;
        } else if (status == PROCESS_OS_BULLSHIT) {
            data->errorIndex = MAGICK_ERROR_OS_BULLSHIT;
        }

        if (status != PROCESS_RUNNING)
            data->params.magickProc = NOB_INVALID_PROC;
    }

    // TODO: remove i18n from this handler
    int8_t scroll = scrollDirection(scrollDelta);
    if (released(ID_QUIT)) {
        data->shouldClose = true;
        printf("close\n");
    } else if (released_s(i18n(AS_BUTTON_SUPPORT))) {
        nob_cmd_append(&cmd, LAUNCHER, SUPPORT_URL);
        nob_cmd_run_async_silent(cmd);
    } else if (released_s(i18n(AS_BUTTON_SELECT_IMAGES))) {
        GetInputFiles(&data->params.inputFiles);
        nob_kill(&data->params.magickProc);
        RunMagickThreaded();
    } else if (released_s(i18n(AS_START_USING))) {
        data->selectedDocumentIndex = MAGICK_ADVANCED_SETTINGS;
        GetInputFiles(&data->params.inputFiles);
        nob_kill(&data->params.magickProc);
        RunMagickThreaded();
    } else if (released_s(i18n(AS_SELECT_TEMP))) {
        SetTempDir(&data->params.tempDir);
    } else if (released_s(i18n(AS_BUTTON_RUN)) || ((IsKeyPressed(KEY_R) && data->params.magickProc == NOB_INVALID_PROC))) {
        nob_kill(&data->params.magickProc);
        RunMagickThreaded();
    } else if (released_s(i18n(AS_BUTTON_STOP)) || IsKeyPressed(KEY_R)) {
        nob_kill(&data->params.magickProc);
        cthreads_thread_ensure_cancelled(data->magickThread, &data->params.threadRunning);
    } else if (released(ID_BUTTON_SELECT_MAGICK)) {
        printf("bin befor: %s\n", data->params.magickBinary.items);
        data->errorIndex = ChangeMagickBinary(&data->params.magickBinary);
        printf("bin aftir: %d\n", data->errorIndex);
    } else if (released(ID_ERROR)) {
        data->errorIndex = MAGICK_ERROR_OK;
    } else if (released(ID_INPUT_FILE)) {
        printf("befor: %s\n", data->params.outputFile.items);
        ChangeOutputPath(&data->params.outputFile);
        printf("aftir: %s\n", data->params.outputFile.items);
    } else if (released_s(i18n(AS_BUTTON_CHANGE_UI_COLOR))) {
        colorscheme = (colorscheme + 1) % APP_COLORSCHEMES;
    } else if (released_s(i18n(AS_BUTTON_CHANGE_LANGUAGE))) {
        language = (language + 1) % APP_LANGUAGES;
        DocumentsUpdate();
    } else if (released_s(i18n(AS_BUTTON_OPEN_RESULT))) {
        if (data->params.outputFile.items[0] != '\0') {
            nob_cmd_append(&cmd, LAUNCHER, data->params.outputFile.items);
            nob_cmd_run_async_silent(cmd);
            nob_cmd_free(cmd);
        } else {
            fprintf(stderr, "No file was made\n");
        }
    } else if (IsKeyDown(KEY_L)) {
        data->tabWidth += 1;
        fprintf(stderr, "MORE: %d\n", data->tabWidth);
    } else if (IsKeyDown(KEY_H)) {
        fprintf(stderr, "LESS: %d\n", data->tabWidth);
        data->tabWidth -= 1;
    }

    // Update only on interaction
    bool scrolled = false;
    if (scroll) {
        scrolled = UpdateResizes((resize_element_t *) &data->params.resizes, scroll);

        // colors
        if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("r")))) {
            data->params.color.r += scroll;
            sprintf(data->params.color_str.r, "%d", data->params.color.r);
        } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("g")))) {
            data->params.color.g += scroll;
            sprintf(data->params.color_str.g, "%d", data->params.color.g);
            scrolled = true;
        } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("b")))) {
            data->params.color.b += scroll;
            sprintf(data->params.color_str.b, "%d", data->params.color.b);
            scrolled = true;
        } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("a")))) {
            data->params.color.a += scroll;
            // guess what
            // we dont have a tool for that called %
            if (data->params.color.a == 255) data->params.color.a = 100;
            if (data->params.color.a > 100) data->params.color.a = 0;  // wrap
            sprintf(data->params.color_str.a, "%d", data->params.color.a);
            scrolled = true;
        } else if (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("GravitySelection")))) {
            data->params.gravity.selected -= scroll;
            // guess what
            // we dont have a tool for that called %
            if (data->params.gravity.selected == 255) data->params.gravity.selected = gravity_len - 1;
            if (data->params.gravity.selected > gravity_len - 1) data->params.gravity.selected = 0;  // wrap
            scrolled = true;
        }

    }
    if (!scrolled) {
        // this should only be updated if no scroll was triggered
        Clay_UpdateScrollContainers(
            true, // drag scrolling
            (Clay_Vector2) { scale * 2.0 * scrollDelta.x, 2.0 * scale * scrollDelta.y },
            GetFrameTime()
        );
    }

    return AppCreateLayout(data);
}

bool testMagick(char* magickBin) {
    Nob_Cmd cmd = {0};
    bool res = true;

    nob_cmd_append(&cmd, magickBin, "-version");

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        fprintf(stderr, "Bro, where is yr magick??\n");
        res = false;
    }
    nob_cmd_free(cmd);
    return res;
}

static inline void reloadFonts(Font *fonts, char *fontpath, int codepoints[512]) {
    static bool ever_loaded = false;
    if (ever_loaded) {
        UnloadFont(fonts[FONT_ID_BODY_16]);
        UnloadFont(fonts[FONT_ID_SIDEBAR]);
        UnloadFont(fonts[FONT_ID_BUTTONS]);
        UnloadFont(fonts[FONT_ID_DOCUMNT]);
        UnloadFont(fonts[FONT_ID_WELCOME]);
    }
    fonts[FONT_ID_BODY_16] = LoadFontEx(fontpath, FONT_LOAD_SIZE * scale,    codepoints, 512);
    fonts[FONT_ID_SIDEBAR] = LoadFontEx(fontpath, sidebar_font_size * scale, codepoints, 512);
    fonts[FONT_ID_BUTTONS] = LoadFontEx(fontpath, button_font_size * scale,  codepoints, 512);
    fonts[FONT_ID_DOCUMNT] = LoadFontEx(fontpath, document_font_size * scale,codepoints, 512);
    fonts[FONT_ID_WELCOME] = LoadFontEx(fontpath, welcome_font_size * scale, codepoints, 512);
    ever_loaded = true;
}

static inline Image LoadAppIcon() {
#ifdef _WIN32
    return LoadImage("icon.png");
#else
  #ifdef INSTALLED
    return LoadImage("/usr/share/dicolmumag/icon.png");
  #else
    return LoadImage("../resources/icon.png");
  #endif // RELEASE
#endif // _WIN32
}

int SetAppFPS() {
    int target_fps = GetMonitorRefreshRate(GetCurrentMonitor());
    // Something is wrong
    if (target_fps <= 0) {
#ifdef DEBUG
        fprintf(stderr, "Falling back to %d fps\n", FALLBACK_FPS);
#endif // DEBUG
        target_fps = FALLBACK_FPS;
    }
    SetTargetFPS(target_fps);
    return target_fps;
}

static inline void SetAppIcon() {
    Image icon = LoadAppIcon();
#ifdef _WIN32
    ImageFormat(&icon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
#endif // _WIN32
    SetWindowIcon(icon);
    UnloadImage(icon);
}

int main() {
    char* title = "Dicolmumag — create collages with ease of creation, proceed to easily collide with creativity and proceedings";
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

    Font fonts[FONTS_IDS];
    char *fontpath = {0};
    if (defaultFont(&fontpath) == -1) {
        return -1;
    }
    // https://stackoverflow.com/questions/73248125/raylibs-drawtextex-doesnt-display-unicode-characters
    int codepoints[512] = {0};
    for (int i = 0; i < 95; i++) codepoints[i] = 32 + i;
    for (int i = 0; i < 255; i++) codepoints[96 + i] = 0x400 + i;
#ifndef NO_SCALING
    scale = GetWindowScaleDPI().x;
    fprintf(stderr, "new scale: %f\n", scale);
#endif // NO_SCALING
    reloadFonts(fonts, fontpath, codepoints);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fonts[FONT_ID_SIDEBAR].texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fonts[FONT_ID_BUTTONS].texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fonts[FONT_ID_DOCUMNT].texture, TEXTURE_FILTER_BILINEAR);

    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));
    Clay_Context *clayContext = Clay_Initialize(clayMemory, CLAY_DIMENSIONS, (Clay_ErrorHandler) { HandleClayErrors, NULL });
    AppData data = AppDataInit();
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    if (!testMagick(data.params.magickBinary.items)) {
        // yes, this is an mdash
        fprintf(stderr, "Sorry, no magick — no worky\n");
        data.errorIndex = MAGICK_ERROR_NOT_WORK;
    }

    SetAppIcon();
    int fps = SetAppFPS();

    // Disable ESC to exit
    SetExitKey(KEY_NULL);
#ifdef UI_TESTING
    Clay_SetDebugModeEnabled(true);
#endif // UI_TESTING
#ifdef LAZY_RENDER
    AppState old_state = {0};
    int prevtime = SLEEP_THRESHOLD;
    double sleep_fps = 1.f/fps;
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
        Clay_RenderCommandArray renderCommands = CreateLayout(clayContext, &data, state);
        state = GetAppState(&data);
        BeginDrawing();
#ifndef NO_SCALING
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_EQUAL)) {
            scale += 0.1;
            reloadFonts(fonts, fontpath, codepoints);
            fprintf(stderr, "%f\n", scale);
        } else if (IsKeyPressed(KEY_MINUS)) {
            scale -= 0.1;
            reloadFonts(fonts, fontpath, codepoints);
            fprintf(stderr, "%f\n", scale);
        } else if (IsKeyPressed(KEY_EQUAL)) {
            scale = 1;
            reloadFonts(fonts, fontpath, codepoints);
            fprintf(stderr, "%f\n", scale);
        }
        if (scale < 0.5) {
            scale = 0.5;
        } else if (scale > 10) {
            scale = 10;
        }
#endif // NO_SCALING
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
                state = GetAppState(&data);
                if (!(are_equal = StatesEqual(&state, &old_state))) break;
                old_state = state;
#ifdef DEBUG
                fprintf(stderr, "Not rendering: %d\n", time);
#endif // DEBUG
                WaitTime(5*sleep_fps);
            } while (true);
            // Look! it moved!
            prevtime = time + SLEEP_THRESHOLD;
        }
#endif // LAZY_RENDER

        Clay_Raylib_Render(renderCommands, fonts);
#ifdef UI_TESTING
        DrawFPS(0,0);
#endif // UI_TESTING
        EndDrawing();
    }

    Clay_Raylib_Close();
}
