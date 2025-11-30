//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifndef INC_3DGAME_PLAYER_H
#define INC_3DGAME_PLAYER_H

typedef struct player_model {
    double x;
    double y;
    double a;
    double fov;
    double speed;
    double turn_speed;
    int render_distance;
} player_model;

extern player_model player;

void setup_player_global();
void update_player_movement(double elapsed_ms);

#endif //INC_3DGAME_PLAYER_H