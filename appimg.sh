#!/usr/bin/env bash

set -euo pipefail  # safe mode
export LC_ALL=C    # speed

APP_ROOT=AppDir.AppDir
NAME=dicolmumag
EXE_NAME=app
# relative to build directory
ICON="../resources/$NAME.png"
DESKTOP="../resources/$NAME.desktop"

if ! command -v make-portable 2>/dev/null; then
    echo -e "\033[1;31mInstall make-portable somewhere where this script can find it\033[0m"
    echo "Link:"
    echo "https://github.com/natanael-b/make-portable"
    exit 1
fi

if command -v appimagetool 2>/dev/null; then
    tool_available=true
else
    echo -e "\033[33mNo icons stripping\033[0m"
    echo "install appimagetool maybe?"
    echo "https://github.com/AppImage/appimagetool/releases/tag/continuous"
    tool_available=false
fi


[ "$(basename "$PWD")" != build ] && cd build

rm -rf "$APP_ROOT"
cmake_flags=(
    -DUSE_EXTERNAL_GLFW=ON
    -DBUILD_EXAMPLES=OFF
    -DCUSTOMIZE_BUILD=ON
    -DBUILD_SHARED_LIBS=ON
    -DLAZY_RENDER=ON
    -DCMAKE_BUILD_TYPE=Release
)
cmake .. "${cmake_flags[@]}" -DAPPIMAGE=OFF && make "-j$(nproc)"

sudo rm /bin/"$EXE_NAME"
sudo cp "$EXE_NAME" /bin/

echo "$PWD"
mkdir -vp "$APP_ROOT"/usr/{lib,bin}
(
    cd "$APP_ROOT"
    ln -fs usr/lib lib
    ln -fs lib usr/lib
    cd usr
    ln -fs lib lib64
)
cp "$EXE_NAME" "$APP_ROOT"/usr/bin/

# ldd "$EXE_NAME" | grep '=>' | sed 's_.* => /\(.*\) (.*)_cp "/\1" "AppDir/.AppDir/\1"; patchelf --replace-needed "/\1" "\1" "$EXE_NAME"\n_g' | tee /dev/stderr | bash

mkdir -p "$APP_ROOT"/usr/share/{icons,applications}

# cp "$ICON" "$APP_ROOT"/usr/share/icons/
# cp "$DESKTOP" "$APP_ROOT"/usr/share/applications/

args=()
[ "$tool_available" == false ] && args+=(--generate-appimage)
args+=(
    --appdir=AppDir
    --desktop="$DESKTOP"
    --icon="$ICON"
    /usr/bin/"$EXE_NAME"
)

# https://github.com/natanael-b/make-portable
make-portable "${args[@]}" || true

# cleanup and fixes for libs
while read -r file; do
    mv -v "$file" "${file/.wrapped/}"
done < <(find "$APP_ROOT"/usr/lib -type f -iname '*.wrapped')
cp -rv /etc/ImageMagick-?/ "$APP_ROOT"/etc/
cp -rv /usr/lib/ImageMagick-*.*.*/ "$APP_ROOT"/usr/lib/


# rebuild to use APPIMAGE
cmake .. "${cmake_flags[@]}" -DAPPIMAGE=ON && make "-j$(nproc)"

# replace the executable
mv ./"$EXE_NAME" "$APP_ROOT"/usr/bin/"$EXE_NAME".wrapped

if [ "$tool_available" == true ]; then
    rm -rf "$APP_ROOT"/usr/share/icons/* "${APP_ROOT:?}"/home
    cp "$ICON" "$APP_ROOT"/usr/share/icons/
    appimagetool -n "$APP_ROOT"
fi


