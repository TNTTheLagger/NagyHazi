//
// Created by TNT on 11/30/2025.
//

#ifndef INC_3DGAME_GLOBALS_H
#define INC_3DGAME_GLOBALS_H

#define VK_ESCAPE 0x1B
#define VK_UP     0x26
#define VK_DOWN   0x28
#define VK_RETURN 0x0D

#include "map.h"
#include "player.h"
#include "screen.h"

extern map game_map;
extern player_model player;
extern screen output_screen;

#endif //INC_3DGAME_GLOBALS_H
