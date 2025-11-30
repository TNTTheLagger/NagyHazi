#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include "globals.h"
#include "platform.h"
#include "screen.h"
#include "menu.h"
#include "player.h"
#include "render.h"


int main(void) {
    get_screen_size();
    if (output_screen.display == NULL) return 1;

    setup_player_global();
    setup_main_menu();

    menu_active = true;
    active_menu = &main_menu;
    if (output_screen.display) {
        memset(output_screen.display, ' ', output_screen.width * output_screen.height);
        menu_render(active_menu);
    }

    long int elapsed_time = 0;
    uint64_t tp2 = current_timestamp_ms();
    uint64_t tp1;
    int f_counter = 0;

    while (1) {
        if (menu_active && active_menu) {
            menu_update_input();
            sleep_ms(20);
            continue;
        }

        f_counter++;
        if (f_counter > 60) {
            update_screen_size();
            f_counter = 0;
        }

        update_player_movement((double)elapsed_time);

        render_frame();
        render_screen();
        memset(output_screen.display, ' ', output_screen.width * output_screen.height);
        if (platform_get_key_state(VK_ESCAPE)&&!active_menu) {
            menu_active = true;
            active_menu = &main_menu;
            menu_render(active_menu);
            sleep_ms(200);
        }
        tp1 = current_timestamp_ms();
        elapsed_time = tp1 - tp2;
        tp2 = tp1;

        // cap frame time roughly to 60fps
        const int FRAME_TIME_MS = 1000 / 60;
        if (elapsed_time < FRAME_TIME_MS) {
            if (FRAME_TIME_MS - elapsed_time < 5) sleep_ms(5);
            else sleep_ms(FRAME_TIME_MS - elapsed_time);
        }
    }

    return 0;
}