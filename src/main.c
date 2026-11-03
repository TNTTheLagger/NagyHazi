// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

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

#include <pspctrl.h>
#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>

// PSP_MODULE_INFO is required
PSP_MODULE_INFO("3DConsole", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}

bool running = false;

int main(void)
{
    setup_callbacks();
    pspDebugScreenInit();

    // --- PSP controller init ---
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    get_screen_size();
    if (output_screen.display == NULL)
        return 1;

    setup_player_global();
    setup_main_menu();

    menu_active = true;
    active_menu = &main_menu;

    if (output_screen.display)
    {
        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);
        menu_render(active_menu);
    }

    long int elapsed_time = 0;
    uint64_t tp2 = current_timestamp_ms();
    uint64_t tp1;

    running = true;

    while (running)
    {
        // ---- MENU MODE ----
        if (menu_active && active_menu)
        {
            menu_update_input();
            sleep_ms(20);
            sceDisplayWaitVblankStart();
            continue;
        }

        // ---- GAME MODE ----
        update_player_movement((double)elapsed_time);

        render_frame();
        render_screen();

        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);

        // ---- START button = ESC / open menu ----
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if ((pad.Buttons & PSP_CTRL_START) && !active_menu)
        {
            menu_active = true;
            active_menu = &main_menu;
            menu_render(active_menu);
            sleep_ms(200); // debounce
            continue;
        }

        // ---- TIMING ----
        tp1 = current_timestamp_ms();
        elapsed_time = tp1 - tp2;
        tp2 = tp1;

        sceDisplayWaitVblankStart();
    }

    // ---- CLEANUP ----
    if (game_map.m)
        free(game_map.m);

    free_menus();
    free_screen();

    return 0;
}
