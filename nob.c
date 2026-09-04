// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

// nob.h is an stb-style library https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
// What that means is that it's a single file that acts both like .c and .h files, but by default
// when you include it, it acts only as .h. To make it include implementations of the functions
// you must define NOB_IMPLEMENTATION macro. This is done to give you full control over where
// the implementations go.
#define NOB_IMPLEMENTATION
#include "src/thirdparty/nob.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "./build/"
#define SRC_FOLDER   "../src/"

#define MAGICK_VER "7.1.2-5"
#define MAGICK_DIR "magick_win"
#define RAYLIB_VER "5.5"
#define RAYLIB_WIN "win64_mingw-w64"
#define RAYLIB_LIN "linux_amd64"
#define RAYLIB_LIN_DIR "raylib-"RAYLIB_VER"_"RAYLIB_LIN
#define RAYLIB_WIN_DIR "raylib-"RAYLIB_VER"_"RAYLIB_WIN

#define NAME "dicolmumag"
#define ICON_RC "icon.rc"
#define ICON_RES "icon.res"
#define ICON_RESOLUTION "256x256"
#define ICON_RADIUS "50"
#define ICON_BASE "icon.png"
#define ICON "../resources/"ICON_BASE

#define unwrap(fn) if (!fn) return false;
#define write_literal(fd, literal) write((fd), ("" literal ""), sizeof("" literal "")-1)
// nob run reset
#define nob_rr nob_cmd_run_sync_and_reset

bool round_corners(const char* input, const char* output) {
    Nob_Cmd cmd = {0};
    Nob_String_Builder sb = {0};

    nob_cmd_append(&cmd,
        "magick", input, "-resize", ICON_RESOLUTION,
        "(",
            "+clone", "-alpha", "extract",
            "-draw",
           "fill black polygon 0,0 0,"ICON_RADIUS" "ICON_RADIUS",0 fill white circle "ICON_RADIUS","ICON_RADIUS" "ICON_RADIUS",0",
           // nob_temp_sprintf("fill black polygon 0,0 0,%d %d,0 fill white circle %d,%d %d,0", radius,radius,radius,radius,radius),
            "(", "+clone", "-flip", ")",
            "-compose", "Multiply",
            "-composite", "(", "+clone", "-flop", ")",
            "-compose", "Multiply", "-composite",
        ")",
        "-alpha", "off",
        "-compose", "CopyOpacity",
        "-composite", output
    );
    unwrap(nob_rr(&cmd))

    nob_cmd_free(cmd);
    return true;
}

bool ensure_downloaded(bool for_windows, Nob_String_Builder raylib_include_dir) {
    nob_log(INFO, "Making sure the dependencies are ready");
    Nob_Cmd cmd = {0};
    if (!nob_file_exists(raylib_include_dir.items)) {
        cmd_append(&cmd, "wget");
        if (for_windows)
            cmd_append(&cmd, "https://github.com/raysan5/raylib/releases/download/"RAYLIB_VER"/"RAYLIB_WIN_DIR".zip");
        else
            cmd_append(&cmd, "https://github.com/raysan5/raylib/releases/download/"RAYLIB_VER"/"RAYLIB_LIN_DIR".tar.gz");
        nob_log(INFO, "Downloading raylib");
        unwrap(nob_rr(&cmd));
        if (for_windows) {
            cmd_append(&cmd, "unzip", RAYLIB_WIN_DIR".zip");
        } else {
            cmd_append(&cmd, "tar", "xzf", RAYLIB_LIN_DIR".tar.gz");
        }
        nob_log(INFO, "Extracting raylib");
        unwrap(nob_rr(&cmd));
    }

    if (!file_exists("icon.ico")) {
        nob_log(INFO, "Rounding the corners");
        unwrap(round_corners(ICON, "icon.ico"));
    }

    if (!file_exists(ICON_RC)) {
        nob_log(INFO, "Creating the "ICON_RC" file");
        Nob_Fd file = nob_fd_open_for_write(ICON_RC);
        write_literal(file, "id ICON icon.ico\n");
        close(file);

    }

    if (!file_exists(ICON_RES)) {
        nob_log(INFO, "Creating the "ICON_RES" file");
        cmd_append(&cmd, "x86_64-w64-mingw32-windres", ICON_RC, "-O", "coff", "-o", ICON_RES);
        unwrap(nob_rr(&cmd));
    }

    nob_cmd_free(cmd);
    return true;
}

bool go_build(bool for_windows) {
    unwrap(mkdir_if_not_exists(BUILD_FOLDER));
    set_current_dir(BUILD_FOLDER);

    Nob_Cmd cmd = {0};

    Nob_String_Builder raylib_include_dir = {0};
    // nob_sb_append_cstr(&raylib_include_dir, BUILD_FOLDER);


    if (for_windows) {
        nob_log(INFO, "Compiling for windows");
        cmd_append(&cmd, "x86_64-w64-mingw32-gcc");
        sb_append_cstr(&raylib_include_dir, RAYLIB_WIN_DIR);
    } else {
        nob_log(INFO, "Compiling for linux");
        cmd_append(&cmd, "gcc");
        sb_append_cstr(&raylib_include_dir, RAYLIB_LIN_DIR);
    }
    sb_append_cstr(&raylib_include_dir, "/lib");
    sb_append_null(&raylib_include_dir);

    nob_log(INFO, "Include dir: %s", raylib_include_dir.items);
    unwrap(ensure_downloaded(for_windows, raylib_include_dir));

    // nob_cc_flags(&cmd);

    // if (argc > 1) {
    //     if (strcmp(argv[1], "win") == 0) {
    //         for_windows = true;
    //     }
    // }
    // // don't care to do two-step compilation
    // for (int i = 1; i < argc; i++) {
    //     cc_define(&cmd, argv[i]);
    // }

    cmd_append(&cmd, "-Ofast", "-Wall", "-Wextra", "-fwrapv", "-Wno-missing-braces", "-Wpadded");
    cmd_append(&cmd, "-DLAZY_RENDER");
    nob_cc_inputs(&cmd, SRC_FOLDER "main.c");
    cmd_append(&cmd, "-l:libraylib.a"); // raylib should be linked with before winmm
    cmd_append(&cmd, "-L");
    cmd_append(&cmd, raylib_include_dir.items);

    if (for_windows) {
      // cmd_append(&cmd, "-DNO_THREADING");
      // cmd_append(&cmd, "-Wformat");
      // cmd_append(&cmd, "-ggdb");
      cmd_append(&cmd, ICON_RES);
      cmd_append(&cmd, "-mwindows"); // probably important
      cmd_append(&cmd, "-I./src/thirdparty/");
      // cmd_append(&cmd, "-lm");
      cmd_append(&cmd, "-lole32"); // coinitialize

      cmd_append(&cmd, "-lwinmm");
      // cmd_append(&cmd, "-lvcomp140");
      // cmd_append(&cmd, "-lntdll");
      // cmd_append(&cmd, "-lpthread");
    } else {
        cmd_append(&cmd, "-lm");
        cmd_append(&cmd, "-lfontconfig");
        cmd_append(&cmd, "-lSDL3");
    }

    nob_cc_output(&cmd, NAME);
    unwrap(cmd_run(&cmd));
    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool for_windows = 1;
    if (!go_build(for_windows)) return 1;

    return 0;
}
