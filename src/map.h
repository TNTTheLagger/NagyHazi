// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifndef INC_3DGAME_MAP_H
#define INC_3DGAME_MAP_H

typedef struct map
{
    int width;
    int height;
    char *m;
    int player_start_x;
    int player_start_y;
} map;

map load_map(char file_path[]);
void save_map_to_file(const char *fname);

#endif // INC_3DGAME_MAP_H