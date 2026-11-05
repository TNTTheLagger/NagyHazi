cmake_minimum_required(VERSION 3.5)
project(3dGame C)

set(CMAKE_C_STANDARD 23)

# ------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------
# Build the PSP port instead of the desktop build. Requires the PSPSDK
# toolchain (psp-gcc) and an active PSP toolchain file.
option(BUILD_PSP "Build the PSP port (requires the PSPSDK toolchain)" OFF)

# Enable debugmalloc memory debugging (intercepts malloc/free in all TUs).
option(ENABLE_DEBUGMALLOC "Enable debugmalloc memory debugging" OFF)

set(SOURCES
        src/main.c
        src/platform.c
        src/screen.c
        src/map.c
        src/player.c
        src/menu.c
        src/render.c
)

# ------------------------------------------------------------------
# PSP target (MIPS, built with the PSPSDK / psp-gcc toolchain)
# ------------------------------------------------------------------
if(BUILD_PSP)
    add_executable(3dGame ${SOURCES})

    target_link_libraries(3dGame PRIVATE
            pspdebug
            pspdisplay
            pspge
            pspctrl
    )

    create_pbp_file(
            TARGET 3dGame
            ICON_PATH NULL
            BACKGROUND_PATH NULL
            PREVIEW_PATH NULL
            TITLE 3dGame
            VERSION 01.00
    )
    return()
endif()

# ------------------------------------------------------------------
# Desktop target (Windows / Linux)
# ------------------------------------------------------------------
add_executable(3dGame ${SOURCES})

if(UNIX AND NOT APPLE)
    target_compile_definitions(3dGame PRIVATE "_POSIX")
    target_link_libraries(3dGame PRIVATE m)
endif()

if(ENABLE_DEBUGMALLOC)
    target_compile_definitions(3dGame PRIVATE ENABLE_DEBUGMALLOC=1)
endif()
