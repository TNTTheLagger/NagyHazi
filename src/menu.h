//
// Created by TNT on 11/30/2025.
//

#ifndef INC_3DGAME_MENU_H
#define INC_3DGAME_MENU_H

#include "menu.h" // placeholder to satisfy tools if needed
#include <stdbool.h>

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

extern menu_t main_menu;
extern bool menu_active;
extern menu_t *active_menu;

void menu_init(menu_t *m);
void menu_free(menu_t *m);
void menu_add_item(menu_t *m, const char *text, menu_action_t action);
void menu_render(menu_t *m);
void setup_main_menu();
void free_menus();

#endif //INC_3DGAME_MENU_H