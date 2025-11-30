#include "map.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

map game_map = {0};

map load_map(char file_path[]) {
    map m = {0};
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        perror("Fájl megnyitása sikertelen");
        return m;
    }

    char buff[100000];

    // determine width from first line
    if (fgets(buff, sizeof(buff), fp)) {
        for (int i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == ',') {
                m.width++;
            }
        }
        m.width++;
    }

    // determine height
    rewind(fp);
    while (fgets(buff, sizeof(buff), fp)) {
        m.height++;
    }

    m.m = malloc(m.width * m.height * sizeof(char));
    if (m.m == NULL) {
        perror("Memóriafoglalás sikertelen");
        fclose(fp);
        m.width = m.height = 0;
        return m;
    }

    rewind(fp);
    int y = 0;
    while (fgets(buff, sizeof(buff), fp) && y < m.height) {
        int x = 0;
        for (int i = 0; buff[i] != '\0' && x < m.width; i++) {
            if (buff[i] == '#' || buff[i] == '.' || buff[i] == 'X') {
                if (buff[i] == 'X') {
                    m.player_start_x = x;
                    m.player_start_y = y;
                    m.m[y * m.width + x] = '.';
                } else {
                    m.m[y * m.width + x] = buff[i];
                }
                x++;
            }
        }
        y++;
    }

    fclose(fp);
    return m;
}

void save_map_to_file(const char *fname) {
    if (game_map.m == NULL || game_map.width <= 0 || game_map.height <= 0) {
        return;
    }

    const char *outname = fname ? fname : "map_saved.csv";
    FILE *fp = fopen(outname, "w");
    if (!fp) {
        perror("Failed to open map file for saving");
        return;
    }

    int px = (int)player.x;
    int py = (int)player.y;

    for (int y = 0; y < game_map.height; ++y) {
        for (int x = 0; x < game_map.width; ++x) {
            char ch = game_map.m[y * game_map.width + x];
            if (x == px && y == py) ch = 'X';
            fputc(ch, fp);
            if (x + 1 < game_map.width) fputc(',', fp);
        }
        fputc('\n', fp);
    }

    fclose(fp);
}