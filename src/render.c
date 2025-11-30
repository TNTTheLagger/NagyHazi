#include "debugmalloc.h"

#include "render.h"
#include "globals.h"
#include <math.h>
#include <string.h>

const char FLOOR_SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};
const char SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};

#define SHADING_COUNT (sizeof(SHADING)/sizeof(SHADING[0]))
#define FLOOR_SHADING_COUNT (sizeof(FLOOR_SHADING)/sizeof(FLOOR_SHADING[0]))

char get_shade(double distance_to_wall) {
    if (distance_to_wall >= player.render_distance)
        return SHADING[0];

    double fraction = distance_to_wall / player.render_distance;
    int index = (int)(fraction * (SHADING_COUNT - 1));
    index = SHADING_COUNT - 1 - index;
    return SHADING[index];
}

void calc_column(int x) {
    double ray_angle = (player.a - player.fov / 2.0) + ((double)x / (double)output_screen.width) * player.fov;
    double distance_to_wall = 0;
    double eye_x = sin(ray_angle);
    double eye_y = cos(ray_angle);
    bool hit_wall = false;
    while (!hit_wall && distance_to_wall < player.render_distance) {
        distance_to_wall += 0.1;
        int test_x = (int)floor(player.x + eye_x * distance_to_wall);
        int test_y = (int)floor(player.y + eye_y * distance_to_wall);
        if (test_x < 0 || test_x >= game_map.width || test_y < 0 || test_y >= game_map.height) {
            hit_wall = true;
            distance_to_wall = player.render_distance;
        } else if (game_map.m[test_y * game_map.width + test_x] == '#') {
            hit_wall = true;
        }
    }
    int ceiling = (double)(output_screen.height / 2.0) - output_screen.height / distance_to_wall;
    int floor = output_screen.height - ceiling;

    char shade  = get_shade(distance_to_wall);
    for (int y = 0;y < output_screen.height;y++) {
        if (y < ceiling) {
            output_screen.display[y * output_screen.width + x] = ' ';
        } else if (y > ceiling && y <= floor) {
            output_screen.display[y * output_screen.width + x] = shade;
        } else {
            double b = 1.0 - (y - output_screen.height / 2.0) / (output_screen.height / 2.0);
            int index = (int)(b * (FLOOR_SHADING_COUNT - 1));
            if (index < 0) index = 0;
            if (index >= FLOOR_SHADING_COUNT) index = FLOOR_SHADING_COUNT - 1;
            index = FLOOR_SHADING_COUNT - 1 - index;
            output_screen.display[y * output_screen.width + x] = FLOOR_SHADING[index];
        }
    }
}

void render_frame() {
    for (int x = 0; x < output_screen.width; x++) {
        calc_column(x);
    }
}