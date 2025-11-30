//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifndef INC_3DGAME_SCREEN_H
#define INC_3DGAME_SCREEN_H

typedef struct screen {
    int width;
    int height;
    char **display;  // 2D array of characters
} screen;

void get_screen_size();
void update_screen_size();
void render_screen();
void free_screen();

#endif //INC_3DGAME_SCREEN_H