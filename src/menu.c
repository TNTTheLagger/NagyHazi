//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include "menu.h"
#include "globals.h"
#include "map.h"
#include "screen.h"
#include "platform.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#define KEY_UP    'w'
#define KEY_DOWN  's'
#define KEY_ENTER '\n'

menu_t main_menu = {0};
menu_t maps_menu = {0};
menu_t *active_menu = NULL;
bool menu_active = false;
static volatile int request_quit = 0;
static char *current_map_file = NULL;

void free_menus() {
    if (current_map_file) {
        free(current_map_file);
        current_map_file = NULL;
    }
    menu_free(&main_menu);
    menu_free(&maps_menu);
}

void menu_init(menu_t *m) {
    if (m->items) {
        for (int i = 0; i < m->count; ++i) free(m->items[i].text);
        free(m->items);
    }
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
    save_map_to_file(current_map_file);
    menu_active = false;
    active_menu = NULL;
}

static void ensure_resume_state(void) {
    bool has_map = (game_map.m != NULL);
    int idx = menu_find_item_index(&main_menu, "Resume");
    if (has_map && idx == -1) {
        menu_insert_item(&main_menu, 0, "Resume", action_resume);
        menu_insert_item(&main_menu, 2, "Save map", action_save_map);
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

void setup_player_global();

static void action_load_selected_map(void) {
    if (maps_menu.count == 0) return;
    int idx = maps_menu.selected;
    if (idx < 0 || idx >= maps_menu.count) return;
    char *fname = maps_menu.items[idx].text;
    if (!fname) return;
    free(game_map.m);
    game_map = load_map(fname);
    setup_player_global();
    if (current_map_file) { free(current_map_file); current_map_file = NULL; }
    if (fname) current_map_file = strdup(fname);

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

static void action_show_maps(void) {
    if (maps_menu.items) menu_free(&maps_menu);
    menu_init(&maps_menu);
    menu_add_item(&maps_menu, "< Back", action_maps_back);

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

    if (maps_menu.count == 1) {
        menu_add_item(&maps_menu, "No maps found", NULL);
    }

    active_menu = &maps_menu;
    menu_active = true;

    if (output_screen.display)
        memset(output_screen.display, ' ', output_screen.width * output_screen.height);

    menu_render(active_menu);
}

static void action_load_map(void) {
    action_show_maps();
}
static void action_quit(void) {
    request_quit = 1;
}

void setup_main_menu() {
    menu_init(&main_menu);
    if (game_map.m != NULL) {
        menu_add_item(&main_menu, "Resume", action_resume);
        menu_add_item(&main_menu, "Save map", action_save_map);
    }
    menu_add_item(&main_menu, "Load map", action_load_map);
    menu_add_item(&main_menu, "Quit", action_quit);
    ensure_resume_state();
}

void menu_update_input() {
    if (!active_menu) return;
    if ((platform_get_key_state(VK_UP) & 0x8000) || (platform_get_key_state('W') & 0x8000)) {
        active_menu->selected--;
        if (active_menu->selected < 0) active_menu->selected = active_menu->count - 1;
        menu_render(active_menu);
        sleep_ms(100);
    }
    if ((platform_get_key_state(VK_DOWN) & 0x8000) || (platform_get_key_state('S') & 0x8000)) {
        active_menu->selected++;
        if (active_menu->selected >= active_menu->count) active_menu->selected = 0;
        menu_render(active_menu);
        sleep_ms(100);
    }
    if (platform_get_key_state(VK_RETURN) & 0x8000) {
        if (active_menu->count > 0) {
            menu_action_t act = active_menu->items[active_menu->selected].action;
            if (act) act();
            if (request_quit) running = false;
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
        sleep_ms(150);
    }
}