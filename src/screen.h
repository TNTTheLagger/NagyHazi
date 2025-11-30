//
// Created by TNT on 11/30/2025.
//

#ifndef INC_3DGAME_SCREEN_H
#define INC_3DGAME_SCREEN_H

typedef struct screen {
    int width;
    int height;
    char *display;
} screen;

void get_screen_size();
void update_screen_size();
void render_screen();

#endif //INC_3DGAME_SCREEN_H