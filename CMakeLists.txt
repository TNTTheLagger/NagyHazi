cmake_minimum_required(VERSION 3.5)
project(3dGame C)

set(CMAKE_C_STANDARD 23)

add_executable(3dGame
        src/main.c
        src/platform.c
        src/screen.c
        src/map.c
        src/player.c
        src/menu.c
        src/render.c

)

if (UNIX)
    target_compile_definitions(3dGame PRIVATE "_POSIX")
endif()