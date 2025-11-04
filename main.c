#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <windows.h>
#include <stdbool.h>
#include <sys/time.h>
#include <stdint.h>
#include <conio.h>
#include <windows.h>

//#define _USE_MATH_DEFINES
#define SHADING_COUNT (sizeof(SHADING)/sizeof(SHADING[0]))
#define FLOOR_SHADING_COUNT (sizeof(FLOOR_SHADING)/sizeof(FLOOR_SHADING[0]))

typedef struct map {
    int width;
    int height;
    char *m;
    int player_start_x;
    int player_start_y;
}map;

typedef struct screen {
    int width;
    int height;
    char *display;
}screen;

typedef struct player_model {
    double x;
    double y;
    double a;
    double fov;
    double speed;
    double turn_speed;
    int render_distance;
}player_model;

//GLOBALS
const char FLOOR_SHADING[] = { ' ', '-', '.', 'x', '#' };
const char SHADING[] = {' ','.','`','^','"',' ',',',':',';','I','l','!','i','>','<','~','+','_','-','?','[',']','{','}','1',')','(','|','\\','/','t','f','j','r','x','n','u','v','c','z','X','Y','U','J','C','L','Q','0','O','Z','m','w','q','p','d','b','k','h','a','o','*','#','M','W','&','8','%','B','@','$'};
map game_map;
player_model player;
screen output_screen;


void getScreenSize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    output_screen.width = columns;
    output_screen.height = rows;
    output_screen.display = malloc(columns * rows * sizeof(char));
}

map load_map(char file_path[]) {
    map m = {0};
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        perror("Fájl megnyitása sikertelen");
        return m;
    }

    char buff[100000];

    // --- Count columns from first line ---
    if (fgets(buff, sizeof(buff), fp)) {
        for (int i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == ',') {
                m.width++;
            }
        }
        m.width++;
    }

    // --- Count rows ---
    rewind(fp);
    while (fgets(buff, sizeof(buff), fp)) {
        m.height++;
    }

    // --- Allocate memory for map on the heap ---
    m.m = malloc(m.width * m.height * sizeof(char));
    if (m.m == NULL) {
        perror("Memóriafoglalás sikertelen");
        fclose(fp);
        m.width = m.height = 0;
        return m;
    }

    // --- Load map data ---
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

void print_map(const map *m) {
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            if (x == player.x && y == player.y)
                putchar('X');
            else
                putchar(m->m[y * m->width + x]);
        }
        putchar('\n');
    }
}

void setup_player_global() {
    player.x = game_map.player_start_x;
    player.y = game_map.player_start_y;
    player.fov = 3.14159 / 4.0;
    player.a = 0;
    player.speed = 0.01;
    player.turn_speed = 0.005;
    player.render_distance = 10;
}

uint64_t current_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

char get_shade(double distance_to_wall) {
    if (distance_to_wall >= player.render_distance)
        return SHADING[0];

    double fraction = distance_to_wall / player.render_distance;
    int index = (int)(fraction * (SHADING_COUNT - 1));

    index = SHADING_COUNT - 1 - index;

    return SHADING[index];
}

void calc_column(int x) {
    double ray_angle = (player.a - player.fov / 2.0) + (x / output_screen.width) * player.fov;
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
        }else if (game_map.m[test_y * game_map.width + test_x] == '#') {
            hit_wall = true;
        }
    }
    double celing =  (output_screen.height / 2.0) - output_screen.height / distance_to_wall;
    double floor = output_screen.height - celing;

    char shade  = get_shade(distance_to_wall);
    for (int y = 0;y < output_screen.height;y++) {
        if (y < celing) {
            output_screen.display[y * output_screen.width + x] = ' ';
        }else if (y > celing && y <= floor) {
            output_screen.display[y * output_screen.width + x] = shade;
        }else {
            double b = 1.0 - (y - output_screen.height / 2.0) / (output_screen.height / 2.0);
            int index = (int)(b * (FLOOR_SHADING_COUNT - 1));
            if (index < 0) index = 0;
            if (index >= FLOOR_SHADING_COUNT) index = FLOOR_SHADING_COUNT - 1;

            // Optional: invert if you want darker at bottom
            index = FLOOR_SHADING_COUNT - 1 - index;

            output_screen.display[y * output_screen.width + x] = FLOOR_SHADING[index];
        }
    }
}

void render_screen() {
    //printf("\033[H"); // move cursor to top-left
    fwrite(output_screen.display, sizeof(char), output_screen.width * output_screen.height, stdout);
    fflush(stdout);
}


void game_loop() {
    long int elapsed_time = 0;
    bool running = true;
    uint64_t tp2 = current_timestamp_ms();
    uint64_t tp1;
    char key = 0;
    while (running) {
        tp1 = current_timestamp_ms();
        elapsed_time = tp1-tp2;
        tp2 = tp1;

        if (_kbhit()) {
            key = _getch();
            switch (key) {
                case 'w':
                case 'W':
                    player.x += sin(player.a) * player.speed * elapsed_time;
                    player.y += cos(player.a) * player.speed * elapsed_time;
                    if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                        player.x -= sin(player.a) * player.speed * elapsed_time;
                        player.y -= cos(player.a) * player.speed * elapsed_time;
                    }
                    break;
                case 's':
                case 'S':
                    player.x -= sin(player.a) * player.speed * elapsed_time;
                    player.y -= cos(player.a) * player.speed * elapsed_time;
                    if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                        player.x += sin(player.a) * player.speed * elapsed_time;
                        player.y += cos(player.a) * player.speed * elapsed_time;
                    }
                    break;
                case 'a':
                case 'A':
                    player.a -= player.turn_speed * elapsed_time;
                    break;
                case 'd':
                case 'D':
                    player.a += player.turn_speed * elapsed_time;
                    break;
                case 'q':
                case 'Q':
                    running = false;
                default:
                    break;
            }
        }
        for (int x = 0;x < output_screen.width;x++) {
            calc_column(x);
        }
        render_screen();
        //printf("%s",output_screen.display);
        //for (int i = 0; i < output_screen.width*output_screen.height; i++) {
        //    putchar(output_screen.display[i]);
        //}
        //printf("\033[H");
        //printf("Player a: %ld",player.a);
    }
    return;
}

int main(void) {
    getScreenSize();
    if (output_screen.display == NULL) {
        return 1;
    }
    printf("columns: %d\trows: %d\n", output_screen.width,output_screen.height);
    game_map = load_map("map.csv");
    printf("Map size: %dx%d\n",game_map.width,game_map.height);
    printf("Player start pos: %dx%d\n",game_map.player_start_x,game_map.player_start_y);
    setup_player_global();
    print_map(&game_map);
    game_loop();
    free(output_screen.display);
    free(game_map.m);
    system("pause");
    return 0;
}