cmake_minimum_required(VERSION 3.27)
project(app C)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
# set(PLATFORM       SDL)

set(CUSTOMIZE_BUILD ON)
set(USE_EXTERNAL_GLFW ON)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(app main.c)

target_compile_options(app PUBLIC)
target_include_directories(app PUBLIC .)

target_link_libraries(app PUBLIC raylib fontconfig m)

# set(CMAKE_C_FLAGS "-Werror -Wall -Wextra -Wpedantic -fwrapv -fanalyzer")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -fwrapv -Wno-missing-braces")
# set(USE_EXTERNAL_GLFW ON)

# set(PLATFORM PLATFORM_DESKTOP)
# set(GRAPHICS GRAPHICS_API_SDL)
# set(PLATFORM PLATFORM_DESKTOP_SDL3)

set(BUILD_SHARED_LIBS ON)
# set(CMAKE_BUILD_TYPE DEBUG)

option(LAZY_RENDER "Why render when u can do nothing?" ON)
if(LAZY_RENDER)
    message(STATUS "LAZY_RENDER")
    add_definitions(-DLAZY_RENDER=1)
endif()

option(UI_TESTING "Enable FPS counter and Clay debug mode" OFF)
if(UI_TESTING)
    message(STATUS "UI_TESTING")
    add_definitions(-DUI_TESTING=1)
endif()

option(DEBUG "Enable printfs" OFF)
if(DEBUG)
    message(STATUS "DEBUG")
    add_definitions(-DDEBUG=1)
endif()

option(LATTE "Use Catppuccin Latte color scheme" OFF)
if(LATTE)
    message(STATUS "LATTE")
    add_definitions(-DLATTE=1)
endif()

# set(CMAKE_C_FLAGS_DEBUG "-Werror -Wall -Wextra -Wpedantic -fwrapv -fanalyzer -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-security -Wnull-dereference -Wstack-protector -Wtrampolines -Walloca -Wvla -Warray-bounds=2 -Wimplicit-fallthrough=3 -Wtraditional-conversion -Wshift-overflow=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion -Warith-conversion -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wformat-signedness -Wshadow -Wstrict-overflow=4 -Wundef -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wstack-usage=1000000 -Wcast-align=strict -fstack-protector-strong -fstack-clash-protection -fPIE -fsanitize=bounds -fsanitize-undefined-trap-on-error -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

