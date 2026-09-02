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
#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

int main(int argc, char **argv)
{
    // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
    // runs it again.
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
    // command line that you want to execute.
    Nob_Cmd cmd = {0};

    // nob.h ships with a bunch of nob_cc_* macros that try abstract away the specific compiler.
    // They are verify basic and not particularly flexible, but you can redefine them if you need to
    // or not use them at all and create your own abstraction on top of Nob_Cmd.
    nob_cc(&cmd);
    nob_cc_flags(&cmd);

    // don't care to do two-step compilation
    for (int i = 1; i < argc; i++) {
        nob_cc_define(&cmd, argv[i]);
    }

    nob_cc_inputs(&cmd, SRC_FOLDER "main.c");
#ifdef _WIN32

    nob_cmd_append(&cmd, "-Ofast");
    nob_cmd_append(&cmd, "# -DNO_THREADING");
    nob_cmd_append(&cmd, "-DLAZY_RENDER");
    nob_cmd_append(&cmd, "# -Wformat");
    nob_cmd_append(&cmd, "# -ggdb");
    nob_cmd_append(&cmd, "../src/main.c");
    nob_cmd_append(&cmd, "my.res");
    nob_cmd_append(&cmd, "-o ./"$NAME.exe"");
    nob_cmd_append(&cmd, "-mwindows # probably important");
    nob_cmd_append(&cmd, "-I../src/thirdparty/");
    // nob_cmd_append(&cmd, "-lm");
    nob_cmd_append(&cmd, "-lole32"); // coinitialize
    nob_cmd_append(&cmd, "-lraylib");
    nob_cmd_append(&cmd, "-lwinmm");
    // nob_cmd_append(&cmd, "-lvcomp140");
    // nob_cmd_append(&cmd, "-lntdll");
    // nob_cmd_append(&cmd, "-lpthread");
    nob_cmd_append(&cmd, "-L./"$raylib_dir"/lib/");
#else // POSIX

#endif // _WIN32
    nob_cmd_append(&cmd, "-lm");
    nob_cmd_append(&cmd, "-lfontconfig");
    nob_cmd_append(&cmd, "-lm");
    nob_cmd_append(&cmd, "-lSDL3");
    nob_cc_output(&cmd, BUILD_FOLDER "dicolmumag");
    if (!nob_cmd_run(&cmd)) return 1;


    return 0;
}
