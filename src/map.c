// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include "map.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspiofilemgr.h>

#define PSP_SAVE_DIR "ms0:/PSP/GAME/3dGame/maps/"
#define PSP_SAVE_MAP "ms0:/PSP/GAME/3dGame/maps/map_saved.csv"
#define DEBUG_MAP_LOAD 0

map game_map = {0};

map load_map(char file_name[])
{
    pspDebugScreenSetXY(0, 0);
    map m = {0};
    SceCtrlData pad; // declare once here

    char path[256];

    if (file_name && file_name[0])
    {
        snprintf(path, sizeof(path), "%s%s", PSP_SAVE_DIR, file_name);
        path[sizeof(path) - 1] = '\0'; // safety
    }
    else
    {
        snprintf(path, sizeof(path), "%s", PSP_SAVE_MAP);
    }

    // Check if file exists and not empty
    FILE *fp_check = fopen(path, "r");
    if (!fp_check)
    {
        pspDebugScreenPrintf("Error: Map file not found: %s\n", path);
        fflush(stdout);
        sceKernelDelayThread(1000000);
        sceKernelExitGame();
    }

    fseek(fp_check, 0, SEEK_END);
    long filesize = ftell(fp_check);
    fclose(fp_check);

    if (filesize <= 0)
    {
        pspDebugScreenPrintf("Error: Map file is empty: %s\n", path);
        fflush(stdout);
        sceKernelDelayThread(1000000);
        sceKernelExitGame();
    }

    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        pspDebugScreenPrintf("Failed to open map file: %s\n", path);
        fflush(stdout);
        sceKernelDelayThread(1000000);
        sceKernelExitGame();
    }

    char buff[100000];

#if DEBUG_MAP_LOAD
    // === First pass: debug raw file content ===
    pspDebugScreenPrintf("=== Raw map file content ===\n\n");
    rewind(fp);
    while (fgets(buff, sizeof(buff), fp))
        pspDebugScreenPrintf("%s", buff);
    pspDebugScreenPrintf("\nPress X to continue...\n");
    fflush(stdout);

    do
    {
        sceCtrlReadBufferPositive(&pad, 1);
    } while (!(pad.Buttons & PSP_CTRL_CROSS));
    sceKernelDelayThread(100000);
#endif

    rewind(fp);

    // === Compute width ===
    if (fgets(buff, sizeof(buff), fp))
    {
        m.width = 0;
        for (int i = 0; buff[i]; i++)
            if (buff[i] == ',')
                m.width++;
        m.width++;
    }

    // === Compute height ===
    rewind(fp);
    m.height = 0;
    while (fgets(buff, sizeof(buff), fp))
        m.height++;
    m.height--; // last line = signature

    // === Allocate memory ===
    m.m = malloc(m.width * m.height);
    if (!m.m)
    {
        fclose(fp);
        pspDebugScreenPrintf("Error: Could not allocate memory for map\n");
        fflush(stdout);
        sceKernelDelayThread(1000000);
        sceKernelExitGame();
    }

    // === Load map into memory ===
    rewind(fp);
    int y = 0;
    while (fgets(buff, sizeof(buff), fp) && y < m.height)
    {
        int x = 0;
        for (int i = 0; buff[i] && x < m.width; i++)
        {
            if (buff[i] == '#' || buff[i] == '.' || buff[i] == 'X')
            {
                if (buff[i] == 'X')
                {
                    m.player_start_x = x;
                    m.player_start_y = y;
                    m.m[y * m.width + x] = '.';
                }
                else
                    m.m[y * m.width + x] = buff[i];
                x++;
            }
        }
        y++;
    }

    fclose(fp);

#if DEBUG_MAP_LOAD
    // === Debug print the loaded map ===
    pspDebugScreenPrintf("\n=== Map loaded into memory ===\n\n");
    for (int y = 0; y < m.height; y++)
    {
        for (int x = 0; x < m.width; x++)
            pspDebugScreenPrintf("%c", m.m[y * m.width + x]);
        pspDebugScreenPrintf("\n");
    }
    pspDebugScreenPrintf("\nPress X to continue...\n");
    fflush(stdout);

    do
    {
        sceCtrlReadBufferPositive(&pad, 1);
    } while (!(pad.Buttons & PSP_CTRL_CROSS));
    sceKernelDelayThread(100000);
#endif

    return m;
}

void save_map_to_file(const char *fname)
{
    if (!game_map.m || game_map.width <= 0 || game_map.height <= 0)
        return;

    // Ensure save directory exists (safe to call repeatedly)
    sceIoMkdir(PSP_SAVE_DIR, 0777);

    const char *outname = fname && fname[0]
                              ? fname
                              : PSP_SAVE_MAP;

    FILE *fp = fopen(outname, "w");
    if (!fp)
    {
        perror("Failed to open map file for saving");
        return;
    }

    int px = (int)player.x;
    int py = (int)player.y;

    for (int y = 0; y < game_map.height; y++)
    {
        for (int x = 0; x < game_map.width; x++)
        {
            char ch = game_map.m[y * game_map.width + x];
            if (x == px && y == py)
                ch = 'X';

            fputc(ch, fp);
            if (x + 1 < game_map.width)
                fputc(',', fp);
        }
        fputc('\n', fp);
    }

    fputs("NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt\n", fp);
    fclose(fp);
}
