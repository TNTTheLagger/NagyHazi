cmake_minimum_required(VERSION 3.5)
project(3dGame C)

set(CMAKE_C_STANDARD 23)

add_executable(${PROJECT_NAME}
        src/main.c
        src/platform.c
        src/screen.c
        src/map.c
        src/player.c
        src/menu.c
        src/render.c

)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspdebug
    pspdisplay
    pspge
    pspctrl
)

create_pbp_file(
    TARGET ${PROJECT_NAME}
    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL
    TITLE ${PROJECT_NAME}
    VERSION 01.00
)