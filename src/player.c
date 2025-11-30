#include "player.h"
#include "globals.h"
#include "platform.h"
#include <math.h>

player_model player;

void setup_player_global() {
    player.x = game_map.player_start_x + 0.5;
    player.y = game_map.player_start_y + 0.5;
    player.fov = 3.14159 / 3.0;
    player.a = 0;
    player.speed = 0.005;
    player.turn_speed = 0.001;
    player.render_distance = 10;
}

void update_player_movement(double elapsed_ms) {
    // elapsed_ms is milliseconds
    if (platform_get_key_state('W') & 0x8000) {
        player.x += sin(player.a) * player.speed * elapsed_ms;
        player.y += cos(player.a) * player.speed * elapsed_ms;
        if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
            player.x -= sin(player.a) * player.speed * elapsed_ms;
            player.y -= cos(player.a) * player.speed * elapsed_ms;
        }
    }

    if (platform_get_key_state('S') & 0x8000) {
        player.x -= sin(player.a) * player.speed * elapsed_ms;
        player.y -= cos(player.a) * player.speed * elapsed_ms;
        if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
            player.x += sin(player.a) * player.speed * elapsed_ms;
            player.y += cos(player.a) * player.speed * elapsed_ms;
        }
    }

    if (platform_get_key_state('A') & 0x8000) {
        player.a -= player.turn_speed * elapsed_ms;
    }
    if (platform_get_key_state('D') & 0x8000) {
        player.a += player.turn_speed * elapsed_ms;
    }
}