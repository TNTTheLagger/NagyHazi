#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <stdint.h>

 
 
#ifndef strdup
#define strdup _strdup
#endif


 
static int gettimeofday(struct timeval *tp, void *tzp) {
    (void)tzp;
    FILETIME ft;
    unsigned long long tmpres = 0;
     
    GetSystemTimeAsFileTime(&ft);
    tmpres |= ((unsigned long long)ft.dwHighDateTime) << 32;
    tmpres |= ft.dwLowDateTime;
     
    tmpres /= 10;
     
    tmpres -= 11644473600000000ULL;
    tp->tv_sec = (long)(tmpres / 1000000ULL);
    tp->tv_usec = (long)(tmpres % 1000000ULL);
    return 0;
}
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <dirent.h>  
 
typedef struct { int Left, Top, Right, Bottom; } SMALL_RECT;
typedef struct { int X, Y; } COORD;
typedef struct {
    COORD dwSize;
    COORD dwCursorPosition;
    SMALL_RECT srWindow;
} CONSOLE_SCREEN_BUFFER_INFO;

#define STD_OUTPUT_HANDLE  ((int)1)
static int GetStdHandle(int dummy) { return 1; }
static int GetConsoleScreenBufferInfo(int h, CONSOLE_SCREEN_BUFFER_INFO *csbi) {
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) == -1) {
        csbi->srWindow.Left = 0;
        csbi->srWindow.Top = 0;
        csbi->srWindow.Right = 79;
        csbi->srWindow.Bottom = 23;
    } else {
        csbi->srWindow.Left = 0;
        csbi->srWindow.Top = 0;
        csbi->srWindow.Right = (int)w.ws_col - 1;
        csbi->srWindow.Bottom = (int)w.ws_row - 1;
    }
    return 1;
}
 
#define VK_ESCAPE 0x1B
#define VK_UP     0x26
#define VK_DOWN   0x28
#define VK_RETURN 0x0D
static short GetAsyncKeyState(int vkey) { (void)vkey; return 0; }
static void Sleep(int ms) { usleep((useconds_t)ms * 1000); }
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

 
 
const char FLOOR_SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};
const char SHADING[] = {' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'i', 'l', 'I', '!', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w', 'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'};
const int FRAME_TIME_MS = 1000 / 60;
map game_map;
player_model player;
screen output_screen;


 
typedef void (*menu_action_t)(void);

typedef struct menu_item {
    char *text;
    menu_action_t action;
} menu_item;

typedef struct menu_t {
    menu_item *items;
    int count;
    int capacity;
    int selected;
} menu_t;

static menu_t main_menu = {0};
 
static menu_t maps_menu = {0};
 
static menu_t *active_menu = NULL;

static bool menu_active = false;
static volatile int request_quit = 0;  

void menu_init(menu_t *m) {
    m->count = 0;
    m->capacity = 4;
    m->selected = 0;
    m->items = malloc(sizeof(menu_item) * m->capacity);
}

void menu_free(menu_t *m) {
    if (!m) return;
    for (int i = 0; i < m->count; ++i) free(m->items[i].text);
    free(m->items);
    m->items = NULL;
    m->count = m->capacity = m->selected = 0;
}

 
void menu_add_item(menu_t *m, const char *text, menu_action_t action) {
    if (m->count >= m->capacity) {
        m->capacity *= 2;
        m->items = realloc(m->items, sizeof(menu_item) * m->capacity);
    }
    m->items[m->count].text = strdup(text);
    m->items[m->count].action = action;
    m->count++;
}

static void action_resume(void) {
    menu_active = false;
    active_menu = NULL;
}

 
static int menu_find_item_index(menu_t *m, const char *text) {
    if (!m || !m->items || !text) return -1;
    for (int i = 0; i < m->count; ++i) {
        if (m->items[i].text && strcmp(m->items[i].text, text) == 0) return i;
    }
    return -1;
}

 
static void menu_insert_item(menu_t *m, int index, const char *text, menu_action_t action) {
    if (!m) return;
    if (index < 0) index = 0;
    if (index > m->count) index = m->count;
    if (m->count >= m->capacity) {
        m->capacity *= 2;
        m->items = realloc(m->items, sizeof(menu_item) * m->capacity);
    }
     
    for (int i = m->count; i > index; --i) {
        m->items[i] = m->items[i - 1];
    }
    m->items[index].text = strdup(text);
    m->items[index].action = action;
    m->count++;
}

 
static void menu_remove_item_by_text(menu_t *m, const char *text) {
    if (!m || !m->items || !text) return;
    int idx = menu_find_item_index(m, text);
    if (idx < 0) return;
    free(m->items[idx].text);
     
    for (int i = idx; i + 1 < m->count; ++i) {
        m->items[i] = m->items[i + 1];
    }
    m->count--;
     
}

static void action_save_map(void) {
     
    menu_active = false;
    active_menu = NULL;
}

 
static void ensure_resume_state(void) {
    bool has_map = (game_map.m != NULL);
    int idx = menu_find_item_index(&main_menu, "Resume");
    if (has_map && idx == -1) {
         
        menu_insert_item(&main_menu, 0, "Resume", action_resume);
        menu_insert_item(&main_menu, 2,"Save map", action_save_map);
    } else if (!has_map && idx != -1) {
         
        menu_remove_item_by_text(&main_menu, "Resume");
        menu_remove_item_by_text(&main_menu, "Save map");
         
        if (main_menu.selected >= main_menu.count) main_menu.selected = main_menu.count - 1;
        if (main_menu.selected < 0) main_menu.selected = 0;
    }
}

map load_map(char file_path[]);

 
static void action_show_maps(void);
 
static void action_maps_back(void);

void setup_player_global(void);

 
static void action_load_selected_map(void) {
    if (maps_menu.count == 0) return;
    int idx = maps_menu.selected;
    if (idx < 0 || idx >= maps_menu.count) return;
    char *fname = maps_menu.items[idx].text;
    if (!fname) return;
    free(game_map.m);
    game_map = load_map(fname);
    setup_player_global();
     
    ensure_resume_state();
     
    menu_active = false;
    active_menu = NULL;
    menu_free(&maps_menu);
     
    if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
}

void menu_render(menu_t * menu);

 
static void action_maps_back(void) {
     
    active_menu = &main_menu;
    menu_active = true;
     
    if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
    menu_render(active_menu);
}

void menu_render(menu_t * menu);

 
static void menu_show_maps(void) {
     
    if (maps_menu.items) menu_free(&maps_menu);
    menu_init(&maps_menu);

     
    menu_add_item(&maps_menu, "< Back", action_maps_back);

#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("*.csv", &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                menu_add_item(&maps_menu, fd.cFileName, action_load_selected_map);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR *d = opendir(".");
    if (d) {
        struct dirent *entry;
        while ((entry = readdir(d)) != NULL) {
            const char *name = entry->d_name;
            size_t len = strlen(name);
            if (len > 4 && strcmp(name + len - 4, ".csv") == 0) {
                menu_add_item(&maps_menu, name, action_load_selected_map);
            }
        }
        closedir(d);
    }
#endif

     
    if (maps_menu.count == 1) {
        menu_add_item(&maps_menu, "No maps found", NULL);
    }

     
    active_menu = &maps_menu;
    menu_active = true;
    if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
    menu_render(active_menu);
}

static void action_load_map(void) {
     
    menu_show_maps();
}
static void action_quit(void) {
    request_quit = 1;
}

void render_screen(void);

 
void menu_render(menu_t *m) {
    if (!m || m->count == 0) return;
     
    memset(output_screen.display, ' ', output_screen.width * output_screen.height);

     
    int maxw = 0;
    for (int i = 0; i < m->count; ++i) {
        int len = (int)strlen(m->items[i].text);
        if (len > maxw) maxw = len;
    }
    int total_height = m->count;
    int start_y = (output_screen.height - total_height) / 2;
    for (int i = 0; i < m->count; ++i) {
        const char *it = m->items[i].text;
        int len = (int)strlen(it);
        int start_x = (output_screen.width - maxw) / 2;
        int y = start_y + i;
        if (y < 0 || y >= output_screen.height) continue;
        int base = y * output_screen.width + start_x;
         
        if (i == m->selected) {
             
            if (start_x > 1) output_screen.display[base - 2] = '>';
            if (start_x + maxw + 1 < output_screen.width) output_screen.display[base + len + 1] = '<';
        }
         
        for (int x = 0; x < len && (start_x + x) < output_screen.width; ++x) {
            output_screen.display[base + x] = it[x];
        }
    }
    render_screen();
}

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

     
    if (fgets(buff, sizeof(buff), fp)) {
        for (int i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == ',') {
                m.width++;
            }
        }
        m.width++;
    }

     
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

             
            index = FLOOR_SHADING_COUNT - 1 - index;

            output_screen.display[y * output_screen.width + x] = FLOOR_SHADING[index];
        }
    }
}

void render_screen() {
     
    fwrite("\x1b[H\x1b[J", 1, 6, stdout);

    int total_size = output_screen.width * output_screen.height;
    static char *buf = NULL;
    static int buf_size = 0;

    int needed = total_size + output_screen.height;  
    if (buf_size < needed) {
        buf = realloc(buf, needed);
        buf_size = needed;
    }

     
    char *dst = buf;
    for (int y = 0; y < output_screen.height; y++) {
        memcpy(dst, &output_screen.display[y * output_screen.width],
               output_screen.width);
        dst += output_screen.width;
        *dst++ = '\n';
    }

     
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
         
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            menu_active = !menu_active;
            if (menu_active) {
                 
                active_menu = &main_menu;
                if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
                menu_render(active_menu);
            } else {
                 
                active_menu = NULL;
                if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
            }
            Sleep(150);
        }

        if (menu_active && active_menu) {
             
            if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) {
                active_menu->selected--;
                if (active_menu->selected < 0) active_menu->selected = active_menu->count - 1;
                menu_render(active_menu);
                Sleep(100);
            }
            if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) {
                active_menu->selected++;
                if (active_menu->selected >= active_menu->count) active_menu->selected = 0;
                menu_render(active_menu);
                Sleep(100);
            }
            if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                 
                if (active_menu->count > 0) {
                    menu_action_t act = active_menu->items[active_menu->selected].action;
                    if (act) act();
                    if (request_quit) return;  
                    if (!menu_active) {
                         
                        if (output_screen.display) memset(output_screen.display, ' ', output_screen.width * output_screen.height);
                         
                        if (active_menu == &maps_menu) {
                            menu_free(&maps_menu);
                        }
                        active_menu = NULL;
                    } else {
                         
                        menu_render(active_menu);
                    }
                }
                Sleep(150);
            }
             
            Sleep(20);
            continue;
        }

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
                Sleep(FRAME_TIME_MS - elapsed_time);  
            }
        }

        if (GetAsyncKeyState('W') & 0x8000) {  
            player.x += sin(player.a) * player.speed * elapsed_time;
            player.y += cos(player.a) * player.speed * elapsed_time;
            if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                player.x -= sin(player.a) * player.speed * elapsed_time;
                player.y -= cos(player.a) * player.speed * elapsed_time;
            }
        }

        if (GetAsyncKeyState('S') & 0x8000) {  
            player.x -= sin(player.a) * player.speed * elapsed_time;
            player.y -= cos(player.a) * player.speed * elapsed_time;
            if (game_map.m[(int)player.y * game_map.width + (int)player.x] == '#') {
                player.x += sin(player.a) * player.speed * elapsed_time;
                player.y += cos(player.a) * player.speed * elapsed_time;
            }
        }

        if (GetAsyncKeyState('A') & 0x8000) {  
            player.a -= player.turn_speed * elapsed_time;
        }

        if (GetAsyncKeyState('D') & 0x8000) {  
            player.a += player.turn_speed * elapsed_time;
        }

        if (GetAsyncKeyState('Q') & 0x8000) {  
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
     
     
     
     
    setup_player_global();
     
     
    menu_init(&main_menu);
     
    if (game_map.m != NULL) {
        menu_add_item(&main_menu, "Resume", action_resume);
        menu_add_item(&main_menu, "Save map", action_save_map);
    }
    menu_add_item(&main_menu, "Load map", action_load_map);  
    menu_add_item(&main_menu, "Quit", action_quit);

     
    ensure_resume_state();

     
    menu_active = true;
    active_menu = &main_menu;
    if (output_screen.display) {
        memset(output_screen.display, ' ', output_screen.width * output_screen.height);
        menu_render(active_menu);
    }

    game_loop();
    menu_free(&main_menu);
     
    menu_free(&maps_menu);
    free(output_screen.display);
    free(game_map.m);
    return 0;
}
