#!/usr/bin/bash

set -euo pipefail      # safe mode
export LC_ALL=C.UTF-8  # speed

source "$(dirname -- "${BASH_SOURCE[0]}")/resources/functions.sh"

build=build
magick_ver=7.1.2-5
raylib_ver=5.5
raylib_platform="win64_mingw-w64"
raylib_dir=raylib-"$raylib_ver"_"$raylib_platform"


archive="$raylib_dir.zip"

# in case you run in not from build
if [ "$(basename "$PWD")" != "$(basename "$build")" ]; then
    mkdir -p "$build"
    cd "$build"
fi

if ! [ -d "$raylib_dir" ]; then
    wget "https://github.com/raysan5/raylib/releases/download/$raylib_ver/$archive"
    unzip "$archive"
fi

if ! [ -f "magick/magick.exe" ]; then
    mkdir -p magick
    m_archive="ImageMagick-$magick_ver-portable-Q16-HDRI-x64.7z"
    wget "https://github.com/ImageMagick/ImageMagick/releases/download/$magick_ver/$m_archive"
    7z x -y "$m_archive" -o./magick
fi

if ! [ -f icon.ico ]; then
    round-corners "$ICON_RADIUS" "$ICON" icon.ico
fi

! [ -f my.rc ] && echo "id ICON icon.ico" > my.rc

x86_64-w64-mingw32-windres my.rc -O coff -o my.res

args=(
    -O3
    -Wall -Wextra -fwrapv -Wno-missing-braces
    # -DNO_THREADING
    -DLAZY_RENDER
    # -Wformat
    # -ggdb
    ../src/main.c
    my.res
    -o ./"$NAME.exe"
    -mwindows # probably important
    -I../src/thirdparty/
    # -lm
    -lole32  # coinitialize
    -lraylib
    -lwinmm
    # -lvcomp140
    # -lntdll
    # -lpthread
    -L./"$raylib_dir"/lib/
)

x86_64-w64-mingw32-gcc "${args[@]}"

out="${NAME}_win"
if [ "${1:-}" == pack ]; then
    rm -rf "$out"{,.zip}
    mkdir "$out"
    cp "/usr/share/fonts/noto/NotoSans-Regular.ttf" font.ttf
    mv "$NAME.exe" font.ttf "$out"/
    cp ../resources/{banner,icon}.png magick/magick.exe "$out"/

    # zip -o "$out".zip -r "$out"/
    zip -ro "$out"{.zip,} &
    shift
fi

if [ "${1:-}" == run ]; then
    cd "$out"
    [ -n "${WAYLAND_DISPLAY:-}" ] && unset DISPLAY
    # WINEDEBUG=-all wine "$NAME.exe" || winedbg --gdb "$NAME.exe"
    wine "$NAME.exe" || winedbg --gdb "$NAME.exe" &
    # WINEDEBUG=+all
    # winedbg --gdb "$NAME.exe"
fi
wait
