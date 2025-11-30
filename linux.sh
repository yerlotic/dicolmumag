#!/usr/bin/bash

set -euo pipefail  # safe mode
export LC_ALL=C    # speed

source "$(dirname -- "${BASH_SOURCE[0]}")/resources/functions.sh"

dir="$NAME"_lin

go-to-build

rm -rf "$dir"
mkdir "$dir"
cd "$dir"
cp /bin/magick .
cp ../dicolmumag ../../resources/{icon,banner}.png .
magick morgify ./icon.png -resize 256x256

set +e

# shellcheck disable=2016
ldd dicolmumag | grep '=>' | sed 's|.* => /\(.*\) (.*)|cp "/\1" "$(basename "\1")"; patchelf --replace-needed "/\1" "$(basename "\1")" dicolmumag\n|g' | tee /dev/stderr | bash

(rm "$dir.zip"; cd .. && zip -ro "$dir.zip" "$dir")

# shellcheck disable=2016
ldd magick | grep '=>' | sed 's|.* => /\(.*\) (.*)|cp "/\1" "$(basename "\1")"; patchelf --replace-needed "/\1" "$(basename "\1")" magick\n|g' | tee /dev/stderr | bash

(rm "${dir}_more.zip"; cd .. && zip -ro "${dir}_more.zip" "$dir")

