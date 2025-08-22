<div align="center">
<img src="resources/banner.png" width=60% height=60% alt="Banner">
<br>
<h1>Dicolmumag</div>
<div align="center">
<br>
<a href="#features">Features</a>
·
<a href="#requirements">Requirements</a>
·
<a href="#installation">Installation</a>
</div>
<br>

<div align="center">
<sup>

App for creating collages with [imagemagick](https://imagemagick.org)
</div>

<details><summary>

## Gallery
</summary>
<br>
<div align="center">
<img src="resources/screenshot.png" width=80% height=80% alt="Screenshot">
<img src="resources/collage1.avif" width=80% height=80% alt="Collage 1">
</div>
</details>

## Features

- [x] Indication of success or failure (e. g. when magick is not available)
- [x] Change color scheme
- [x] Specify gravity
- [x] Best fit
- [x] Transparent background
- [x] Resize input images
- [x] Change location for temporary files
- [x] Specify output resolution and margins
- [ ] Specify maximum images per output

## Requirements

 - C Compiler
 - Git
 - A GPU
 - CMake

## Installation

1. Clone the repo
2. Compile
    <details><summary>

    Compile definitions
    </summary>

    - `LAZY_RENDER` — Why render when u can do nothing?
    - `UI_TESTING` — Enable FPS counter and Clay debug mode
    - `DEBUG` — Enable `printf`s
    - `APPIMAGE` — Use magick from `$APPDIR`
    </details>

    ```shell
    mkdir -p build
    (cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DLAZY_RENDER=ON .. && make -j$(nproc))
    ```

3. Run:

    ```shell
    ./build/app
    ```

