#include <stdio.h>
#include <stdint.h>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#include <stdint.h>  // for uint64_t

struct timeval {
    long tv_sec;   /* seconds */
    long tv_usec;  /* microseconds */
};

int gettimeofday(struct timeval *tp, void *tzp) {
    FILETIME ft;
    uint64_t tmpres = 0;
    const uint64_t EPOCH_DIFF = 11644473600000000ULL; // difference between Jan 1, 1601 and Jan 1, 1970 in 100-ns units

    if (tp) {
        GetSystemTimeAsFileTime(&ft);
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Convert into microseconds
        tmpres /= 10;
        // Convert from Windows epoch (1601) to Unix epoch (1970)
        tmpres -= EPOCH_DIFF;

        tp->tv_sec  = (long)(tmpres / 1000000ULL);
        tp->tv_usec = (long)(tmpres % 1000000ULL);
    }
    // tzp is ignored (timezone info deprecated)
    return 0;
}
#else
#include <sys/time.h>
#endif

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
//const char FLOOR_SHADING[] = { ' ', '-', '.', 'x', '#' };
const char FLOOR_SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};
const char SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};
const int FRAME_TIME_MS = 1000 / 60;
map game_map;
player_model player;
screen output_screen;


void get_screen_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    output_screen.width = columns-1;
    output_screen.height = rows;
    output_screen.display = malloc(columns * rows * sizeof(char));
}

void update_screen_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    output_screen.width = columns-1;
    output_screen.height = rows;
    free(output_screen.display);
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
    player.x = game_map.player_start_x + 0.5;
    player.y = game_map.player_start_y + 0.5;
    player.fov = 3.14159 / 3.0;
    player.a = 0;
    player.speed = 0.005;
    player.turn_speed = 0.001;
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
        }else if (game_map.m[test_y * game_map.width + test_x] == '#') {
            hit_wall = true;
        }
    }
    int ceiling = (double)(output_screen.height / 2.0) - output_screen.height / distance_to_wall;
    int floor = output_screen.height - ceiling;

    char shade  = get_shade(distance_to_wall);
    for (int y = 0;y < output_screen.height;y++) {
        if (y < ceiling) {
            output_screen.display[y * output_screen.width + x] = ' ';
        }else if (y > ceiling && y <= floor) {
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
    // Move cursor to top-left and clear screen content below
    fwrite("\x1b[H\x1b[J", 1, 6, stdout);

    int total_size = output_screen.width * output_screen.height;
    static char *buf = NULL;
    static int buf_size = 0;

    int needed = total_size + output_screen.height; // add '\n' per line
    if (buf_size < needed) {
        buf = realloc(buf, needed);
        buf_size = needed;
    }

    // Fill buffer with the screen content + newlines
    char *dst = buf;
    for (int y = 0; y < output_screen.height; y++) {
        memcpy(dst, &output_screen.display[y * output_screen.width],
               output_screen.width);
        dst += output_screen.width;
        *dst++ = '\n';
    }

    // Write everything in one syscall
    fwrite(buf, 1, dst - buf, stdout);
    fflush(stdout);
}



void game_loop() {
    long int elapsed_time = 0;
    bool running = true;
    uint64_t tp2 = current_timestamp_ms();
    uint64_t tp1;
    char key = 0;
    int f_counter = 0;
    while (running) {
        f_counter++;
        if (f_counter > 60) {
            update_screen_size();
            f_counter = 0;
        }
        tp1 = current_timestamp_ms();
        elapsed_time = tp1-tp2;
        tp2 = tp1;
        if (elapsed_time < FRAME_TIME_MS) {
            if (FRAME_TIME_MS - elapsed_time < 5) {
                Sleep(5);
            }else {
                Sleep(FRAME_TIME_MS - elapsed_time); // Windows API sleep in ms
            }
        }

        if (GetAsyncKeyState('W') & 0x8000) { // Forward
            player.x += sin(player.a) * player.speed * elapsed_time;
            player.y += cos(player.a) * player.speed * elapsed_time;
            if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                player.x -= sin(player.a) * player.speed * elapsed_time;
                player.y -= cos(player.a) * player.speed * elapsed_time;
            }
        }

        if (GetAsyncKeyState('S') & 0x8000) { // Backward
            player.x -= sin(player.a) * player.speed * elapsed_time;
            player.y -= cos(player.a) * player.speed * elapsed_time;
            if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                player.x += sin(player.a) * player.speed * elapsed_time;
                player.y += cos(player.a) * player.speed * elapsed_time;
            }
        }

        if (GetAsyncKeyState('A') & 0x8000) { // Turn left
            player.a -= player.turn_speed * elapsed_time;
        }

        if (GetAsyncKeyState('D') & 0x8000) { // Turn right
            player.a += player.turn_speed * elapsed_time;
        }

        if (GetAsyncKeyState('Q') & 0x8000) { // Quit
            running = false;
        }
        for (int x = 0;x < output_screen.width;x++) {
            calc_column(x);
        }
        render_screen();
        memset(output_screen.display, ' ', output_screen.width * output_screen.height);
    }
    return;
}

int main(void) {
    get_screen_size();
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
    return 0;
}