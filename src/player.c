// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include <pspctrl.h>
#include "player.h"
#include "globals.h"
#include "platform.h"
#include <math.h>

player_model player;

void setup_player_global()
{
    player.x = game_map.player_start_x + 0.5;
    player.y = game_map.player_start_y + 0.5;
    player.fov = 3.14159 / 3.0;
    player.a = 0;
    player.speed = 0.005;
    player.turn_speed = 0.001;
    player.render_distance = 10;
}

void update_player_movement(double elapsed_ms)
{
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);

    double move_step = player.speed * elapsed_ms;
    double turn_step = player.turn_speed * elapsed_ms;

    // Forward
    if (pad.Buttons & PSP_CTRL_UP)
    {
        double nx = player.x + sin(player.a) * move_step;
        double ny = player.y + cos(player.a) * move_step;

        if (game_map.m[(int)ny * game_map.width + (int)nx] != '#')
        {
            player.x = nx;
            player.y = ny;
        }
    }

    // Backward
    if (pad.Buttons & PSP_CTRL_DOWN)
    {
        double nx = player.x - sin(player.a) * move_step;
        double ny = player.y - cos(player.a) * move_step;

        if (game_map.m[(int)ny * game_map.width + (int)nx] != '#')
        {
            player.x = nx;
            player.y = ny;
        }
    }

    // Turn left
    if (pad.Buttons & PSP_CTRL_LEFT)
    {
        player.a -= turn_step;
    }

    // Turn right
    if (pad.Buttons & PSP_CTRL_RIGHT)
    {
        player.a += turn_step;
    }
}