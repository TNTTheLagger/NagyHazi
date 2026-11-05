// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifdef ENABLE_DEBUGMALLOC
#include "debugmalloc.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "globals.h"
#include "platform.h"
#include "screen.h"
#include "menu.h"
#include "player.h"
#include "render.h"

#ifdef __PSP__
#include <pspctrl.h>
#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspkernel.h>

/* PSP_MODULE_INFO is required */
PSP_MODULE_INFO("3DConsole", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

static int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

static int callback_thread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

static int setup_callbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}
#endif

bool running = false;

int main(void)
{
#ifdef __PSP__
    setup_callbacks();
    pspDebugScreenInit();

    /* --- PSP controller init --- */
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
#endif

    get_screen_size();
    if (output_screen.display == NULL)
        return 1;

    setup_player_global();
    setup_main_menu();

    menu_active = true;
    active_menu = &main_menu;

    if (output_screen.display) {
        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);
        menu_render(active_menu);
    }

    long int elapsed_time = 0;
    uint64_t tp2 = current_timestamp_ms();
    uint64_t tp1;
    int f_counter = 0;

    running = true;

    while (running) {
        if (menu_active && active_menu) {
            menu_update_input();
            sleep_ms(20);
#ifdef __PSP__
            sceDisplayWaitVblankStart();
#endif
            continue;
        }

        f_counter++;
        if (f_counter > 60) {
#ifndef __PSP__
            update_screen_size();
#endif
            f_counter = 0;
        }

        update_player_movement((double)elapsed_time);

        render_frame();
        render_screen();

        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);

#ifdef __PSP__
        /* --- START button = ESC / open menu --- */
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if ((pad.Buttons & PSP_CTRL_START) && !active_menu) {
            menu_active = true;
            active_menu = &main_menu;
            menu_render(active_menu);
            sleep_ms(200); /* debounce */
            continue;
        }
#else
        if (platform_get_keys() & KEY_ESCAPE) {
            if (!active_menu) {
                menu_active = true;
                active_menu = &main_menu;
                menu_render(active_menu);
                sleep_ms(200);
                continue;
            }
        }
#endif

        /* --- TIMING --- */
        tp1 = current_timestamp_ms();
        elapsed_time = tp1 - tp2;
        tp2 = tp1;

#ifndef __PSP__
        const int FRAME_TIME_MS = 1000 / 60;
        if (elapsed_time < FRAME_TIME_MS) {
            if (FRAME_TIME_MS - elapsed_time < 5)
                sleep_ms(5);
            else
                sleep_ms(FRAME_TIME_MS - elapsed_time);
        }
#else
        sceDisplayWaitVblankStart();
#endif
    }

    /* --- CLEANUP --- */
    if (game_map.m)
        free(game_map.m);

    free_menus();
    free_screen();

    return 0;
}
