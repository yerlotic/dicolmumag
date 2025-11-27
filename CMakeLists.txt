cmake_minimum_required(VERSION 3.20)
project(dicolmumag C)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
# set(PLATFORM       Desktop)
set(PLATFORM       SDL)

set(CUSTOMIZE_BUILD ON)
set(USE_EXTERNAL_GLFW OFF)
set(GLFW_BUILD_WAYLAND ON)

# add_definitions(-DPLATFORM_DESKTOP_SDL3=1)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(dicolmumag src/main.c)

target_compile_options(dicolmumag PUBLIC)
target_include_directories(dicolmumag PUBLIC .)

target_link_libraries(dicolmumag PUBLIC raylib fontconfig m SDL3)

# set(CMAKE_C_FLAGS "-Werror -Wall -Wextra -Wpedantic -fwrapv -fanalyzer")
set(CMAKE_C_FLAGS "-Wall -Wextra -fwrapv -Wno-missing-braces")
set(USE_EXTERNAL_GLFW ON)

# set(PLATFORM PLATFORM_DESKTOP)
# set(GRAPHICS GRAPHICS_API_SDL)
# set(PLATFORM PLATFORM_DESKTOP_SDL3)

set(BUILD_SHARED_LIBS ON)
# set(CMAKE_BUILD_TYPE DEBUG)

option(APPIMAGE "Compile for App Image" OFF)
if(APPIMAGE)
    message(STATUS "APPIMAGE")
    add_definitions(-DAPPIMAGE=1)
endif()

option(DEBUG "Enable printfs" OFF)
if(DEBUG)
    message(STATUS "DEBUG")
    add_definitions(-DDEBUG=1)
endif()

option(INSTALLED "Use system resources (/usr/share/dicolmumag)" OFF)
if(INSTALLED)
    message(STATUS "INSTALLED")
    add_definitions(-DINSTALLED=1)
endif()

option(LAZY_RENDER "Why render when u can do nothing?" ON)
if(LAZY_RENDER)
    message(STATUS "LAZY_RENDER")
    add_definitions(-DLAZY_RENDER=1)
endif()

option(NO_SCALING "Disable scaling" OFF)
if(NO_SCALING)
    message(STATUS "NO_SCALING")
    add_definitions(-DNO_SCALING=1)
endif()

option(NO_THREADING "Disable threads, run magick in the main thread" OFF)
if(NO_THREADING)
    message(STATUS "NO_THREADING")
    add_definitions(-DNO_THREADING=1)
endif()

option(UI_TESTING "Enable FPS counter and Clay debug mode" OFF)
if(UI_TESTING)
    message(STATUS "UI_TESTING")
    add_definitions(-DUI_TESTING=1)
endif()

# set(CMAKE_C_FLAGS_DEBUG "-Werror -Wall -Wextra -Wpedantic -fwrapv -fanalyzer
# -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-security
# -Wnull-dereference -Wstack-protector -Wtrampolines -Walloca -Wvla
# -Warray-bounds=2 -Wimplicit-fallthrough=3 -Wtraditional-conversion
# -Wshift-overflow=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion
# -Warith-conversion -Wlogical-op -Wduplicated-cond -Wduplicated-branches
# -Wformat-signedness -Wshadow -Wstrict-overflow=4 -Wundef -Wstrict-prototypes
# -Wswitch-default -Wswitch-enum -Wstack-usage=1000000 -Wcast-align=strict
# -fstack-protector-strong -fstack-clash-protection -fPIE -fsanitize=bounds
# -fsanitize-undefined-trap-on-error -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
# -Wl,-z,separate-code")
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3 -fsanitize=address,undefined,bounds")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

message(CMAKE_C_FLAGS_RELEASE="${CMAKE_C_FLAGS_RELEASE}")
message(CMAKE_C_FLAGS_DEBUG="${CMAKE_C_FLAGS_DEBUG} -ggdb")


# execute_process("magick" "../resources/icon.png" "-resize" "128x128" "./icon.png")
