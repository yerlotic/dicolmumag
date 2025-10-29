<div align="center">
<img src="resources/banner.png" width=60% height=60% alt="Banner">
<br>
<h1>Dicolmumag</h1>
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

App for creating collages with [ImageMagick](https://imagemagick.org)
</div>

<details><summary>

## Gallery
</summary>
<br>
<div align="center">
<img src="resources/welcome.png" width=80% height=80% alt="Welcome screenshot">
<img src="resources/screenshot.png" width=80% height=80% alt="Screenshot">
<img src="resources/collage1.avif" width=80% height=80% alt="Collage 1">
</div>
</details>

## Features

- [x] Windows support
- [x] Multiple languages
- [x] Multiple color schemes
- [x] Indication of success or failure
- [x] Specify gravity
- [x] Best fit
- [x] Resize input images
- [x] Change location for temporary files
- [x] Specify output resolution and margins
- [x] Tell the user that output resolution is disabled or not
- [ ] Specify maximum images per output
- [ ] Add labels to images
- [ ] Toggle sections in settings directly
- [x] Tips

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

    | Name           | Description                                    | Notes |
    | -------------- | ---------------------------------------------- | ----- |
    | `APPIMAGE`     | Use magick binary from `$APPDIR`               | POSIX |
    | `DEBUG`        | Enable some `printf`s                          |       |
    | `INSTALLED`    | Use `/usr/...` paths                           | POSIX |
    | `LAZY_RENDER`  | Why render when u can do nothing?              |       |
    | `NO_SCALING`   | Disable scaling with <kbd>+</kbd>/<kbd>-</kbd> |       |
    | `NO_THREADING` | Magick will be launched in the main thread     |       |
    | `UI_TESTING`   | Enable FPS counter and Clay debug mode         |       |
    </details>

    ```shell
    mkdir -p build
    (cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DLAZY_RENDER=ON .. && make -j$(nproc))
    ```

3. Run:

    ```shell
    ./build/dicolmumag
    ```

## Troubleshooting

<details><summary>

### Windows
</summary>

1. Type <kbd>Win+R</kbd>, type `cmd`, hit <kbd>Enter</kbd>
2. Enter a disk letter with `dicolmumag_win` folder extracted. For example:

  ```powershell
  D:
  ```
3. Type `cd <path>`, where `<path>` is a path to folder with `dicolmumag.exe`

  ```powershell
  cd path\to\dicolmumag_win
  ```
4. Run the program with logs visible:

  ```powershell
  .\dicolmumag.exe 2>&1 | echo
  ```
</details>

<br>
<div align="center">Made on 🌍 with ❤️</div>
