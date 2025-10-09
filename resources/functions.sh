#!/usr/bin/bash

export ICON_RES=256x256
export ICON_RADIUS=50
export NAME=dicolmumag
export EXE_NAME=dicolmumag
export ICON_BASE="$NAME.png"
# relative to build directory
export ICON="../resources/$ICON_BASE"
# round corners in place
# Usage round-corners radius input.png output.jpg
round-corners() {
    args=(
        "$2" -resize "$ICON_RES"
        \(
            +clone -alpha extract
            -draw "fill black polygon 0,0 0,$1 $1,0 fill white circle $1,$1 $1,0"
            \( +clone -flip \)
            -compose Multiply
            -composite \( +clone -flop \)
            -compose Multiply -composite
        \)
        -alpha off
        -compose CopyOpacity
        -composite "$3"
        )
    magick "${args[@]}"
}

