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

- [ ] Specify gravity
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

First, compile this:

```shell
git clone https://gitea.vsopu.net/me/dicolmumag && cd dicolmumag
mkdir -p build
(cd build && cmake .. && make -j$(nproc))
```

Then run:

```shell
./app
```

