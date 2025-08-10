<div align="center">
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

![Screenshot](resources/screenshot.png)
![Collage 1](resources/collage1.avif)
</details>

## Features

- [ ] Specify output resolutions and margins
- [ ] Indication of success or failure
- [ ] Change color scheme
- [x] Specify gravity
- [x] Best fit
- [x] Transparent background
- [x] Resize input images
- [x] Change location for temporary files

## Requirements

 - C Compiler
 - Git
 - A GPU
 - CMake

## Installation

1. Clone the repo
2. Compile
    ```shell
    mkdir -p build
    (cd build && cmake .. && make -j$(nproc))
    ```

3. Run:

    ```shell
    ./build/app
    ```

