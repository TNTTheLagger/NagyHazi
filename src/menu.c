// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include <pspctrl.h>
#include "menu.h"
#include "globals.h"
#include "map.h"
#include "screen.h"
#include "platform.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pspiofilemgr.h>

#ifdef _WIN32
#else
#include <dirent.h>
#endif

menu_t main_menu = {0};
menu_t maps_menu = {0};
menu_t *active_menu = NULL;
bool menu_active = false;
static volatile int request_quit = 0;
static char *current_map_file = NULL;

void free_menus()
{
    if (current_map_file)
    {
        free(current_map_file);
        current_map_file = NULL;
    }
    menu_free(&main_menu);
    menu_free(&maps_menu);
}

void menu_init(menu_t *m)
{
    if (m->items)
    {
        for (int i = 0; i < m->count; ++i)
            free(m->items[i].text);
        free(m->items);
    }
    m->count = 0;
    m->capacity = 4;
    m->selected = 0;
    m->items = malloc(sizeof(menu_item) * m->capacity);
}

void menu_free(menu_t *m)
{
    if (!m)
        return;
    for (int i = 0; i < m->count; ++i)
        free(m->items[i].text);
    free(m->items);
    m->items = NULL;
    m->count = m->capacity = m->selected = 0;
}

void menu_add_item(menu_t *m, const char *text, menu_action_t action)
{
    if (m->count >= m->capacity)
    {
        m->capacity *= 2;
        m->items = realloc(m->items, sizeof(menu_item) * m->capacity);
    }
    m->items[m->count].text = strdup(text);
    m->items[m->count].action = action;
    m->count++;
}

static void action_resume(void)
{
    menu_active = false;
    active_menu = NULL;
}

static int menu_find_item_index(menu_t *m, const char *text)
{
    if (!m || !m->items || !text)
        return -1;
    for (int i = 0; i < m->count; ++i)
    {
        if (m->items[i].text && strcmp(m->items[i].text, text) == 0)
            return i;
    }
    return -1;
}

static void menu_insert_item(menu_t *m, int index, const char *text, menu_action_t action)
{
    if (!m)
        return;
    if (index < 0)
        index = 0;
    if (index > m->count)
        index = m->count;
    if (m->count >= m->capacity)
    {
        m->capacity *= 2;
        m->items = realloc(m->items, sizeof(menu_item) * m->capacity);
    }
    for (int i = m->count; i > index; --i)
    {
        m->items[i] = m->items[i - 1];
    }
    m->items[index].text = strdup(text);
    m->items[index].action = action;
    m->count++;
}

static void menu_remove_item_by_text(menu_t *m, const char *text)
{
    if (!m || !m->items || !text)
        return;
    int idx = menu_find_item_index(m, text);
    if (idx < 0)
        return;
    free(m->items[idx].text);
    for (int i = idx; i + 1 < m->count; ++i)
    {
        m->items[i] = m->items[i + 1];
    }
    m->count--;
}

static void action_save_map(void)
{
    save_map_to_file(current_map_file);
    menu_active = false;
    active_menu = NULL;
}

static void ensure_resume_state(void)
{
    bool has_map = (game_map.m != NULL);
    int idx = menu_find_item_index(&main_menu, "Resume");
    if (has_map && idx == -1)
    {
        menu_insert_item(&main_menu, 0, "Resume", action_resume);
        menu_insert_item(&main_menu, 2, "Save map", action_save_map);
    }
    else if (!has_map && idx != -1)
    {
        menu_remove_item_by_text(&main_menu, "Resume");
        menu_remove_item_by_text(&main_menu, "Save map");
        if (main_menu.selected >= main_menu.count)
            main_menu.selected = main_menu.count - 1;
        if (main_menu.selected < 0)
            main_menu.selected = 0;
    }
}

map load_map(char file_path[]);

static void action_show_maps(void);
static void action_maps_back(void);

void setup_player_global();

static void action_load_selected_map(void)
{
    if (maps_menu.count == 0)
        return;
    int idx = maps_menu.selected;
    if (idx < 0 || idx >= maps_menu.count)
        return;
    char *fname = maps_menu.items[idx].text;
    if (!fname)
        return;

    free(game_map.m);
    game_map = load_map(fname);
    setup_player_global();

    if (current_map_file)
    {
        free(current_map_file);
        current_map_file = NULL;
    }
    if (fname)
        current_map_file = strdup(fname);

    ensure_resume_state();

    menu_active = false;
    active_menu = NULL;
    menu_free(&maps_menu);

    if (output_screen.display)
    {
        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);
    }
}

void menu_render(menu_t *menu);

static void action_maps_back(void)
{
    active_menu = &main_menu;
    menu_active = true;

    if (output_screen.display)
    {
        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);
    }

    menu_render(active_menu);
}

void menu_render(menu_t *m)
{
    if (!m || m->count == 0)
        return;

    for (int y = 0; y < output_screen.height; y++)
        memset(output_screen.display[y], ' ', output_screen.width);

    int maxw = 0;
    for (int i = 0; i < m->count; ++i)
    {
        int len = (int)strlen(m->items[i].text);
        if (len > maxw)
            maxw = len;
    }

    int total_height = m->count;
    int start_y = (output_screen.height - total_height) / 2;

    for (int i = 0; i < m->count; ++i)
    {
        const char *it = m->items[i].text;
        int len = (int)strlen(it);
        int start_x = (output_screen.width - maxw) / 2;
        int y = start_y + i;
        if (y < 0 || y >= output_screen.height)
            continue;

        if (i == m->selected)
        {
            if (start_x > 1)
                output_screen.display[y][start_x - 2] = '>';
            if (start_x + maxw + 1 < output_screen.width)
                output_screen.display[y][start_x + len + 1] = '<';
        }

        for (int x = 0; x < len && (start_x + x) < output_screen.width; ++x)
        {
            output_screen.display[y][start_x + x] = it[x];
        }
    }

    render_screen();
}

static void action_show_maps(void)
{
    if (maps_menu.items)
        menu_free(&maps_menu);

    menu_init(&maps_menu);
    menu_add_item(&maps_menu, "< Back", action_maps_back);

#ifdef _WIN32
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = FindFirstFile("*.*", &fdFile); // list all files

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                menu_add_item(&maps_menu, fdFile.cFileName, action_load_selected_map);
            }
        } while (FindNextFile(hFind, &fdFile));
        FindClose(hFind);
    }

#elif defined(__PSP__)
    const char *scan_path = "ms0:/PSP/GAME/3dGame/maps";

    // Ensure folders exist
    sceIoMkdir("ms0:/PSP/GAME/3dGame", 0777);
    sceIoMkdir(scan_path, 0777);

    SceUID dfd = sceIoDopen(scan_path);
    int found = 0;

    if (dfd >= 0)
    {
        SceIoDirent dir;
        memset(&dir, 0, sizeof(dir));

        while (sceIoDread(dfd, &dir) > 0)
        {
            // Ignore directories
            if (!(dir.d_stat.st_attr & FIO_SO_IFDIR))
            {
                // Add filename to menu
                menu_add_item(&maps_menu, dir.d_name, action_load_selected_map);
                found++;
            }
            memset(&dir, 0, sizeof(dir));
        }

        sceIoDclose(dfd);
    }

    if (found == 0)
        menu_add_item(&maps_menu, "No maps found", NULL);

#else
    DIR *d = opendir(".");
    if (d)
    {
        struct dirent *entry;
        int found = 0;
        while ((entry = readdir(d)) != NULL)
        {
            if (entry->d_type != DT_DIR) // ignore directories
            {
                menu_add_item(&maps_menu, entry->d_name, action_load_selected_map);
                found++;
            }
        }
        closedir(d);
        if (found == 0)
            menu_add_item(&maps_menu, "No maps found", NULL);
    }
#endif

    active_menu = &maps_menu;
    menu_active = true;

    // Clear the screen buffer
    if (output_screen.display)
    {
        for (int y = 0; y < output_screen.height; y++)
            memset(output_screen.display[y], ' ', output_screen.width);
    }

    menu_render(active_menu);
}

static void action_load_map(void)
{
    action_show_maps();
}

static void action_quit(void)
{
    request_quit = 1;
}

void setup_main_menu()
{
    menu_init(&main_menu);
    if (game_map.m != NULL)
    {
        menu_add_item(&main_menu, "Resume", action_resume);
        menu_add_item(&main_menu, "Save map", action_save_map);
    }
    menu_add_item(&main_menu, "Load map", action_load_map);
    menu_add_item(&main_menu, "Quit", action_quit);
    ensure_resume_state();
}

void menu_update_input(void)
{
    if (!active_menu)
        return;

    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);

    // ---- UP ----
    if (pad.Buttons & PSP_CTRL_UP)
    {
        active_menu->selected--;
        if (active_menu->selected < 0)
            active_menu->selected = active_menu->count - 1;

        menu_render(active_menu);
        sleep_ms(100); // debounce
    }

    // ---- DOWN ----
    if (pad.Buttons & PSP_CTRL_DOWN)
    {
        active_menu->selected++;
        if (active_menu->selected >= active_menu->count)
            active_menu->selected = 0;

        menu_render(active_menu);
        sleep_ms(100); // debounce
    }

    // ---- X (ENTER) ----
    if (pad.Buttons & PSP_CTRL_CROSS)
    {
        if (active_menu->count > 0)
        {
            menu_action_t act =
                active_menu->items[active_menu->selected].action;

            if (act)
                act();

            if (request_quit)
                running = false;

            if (!menu_active)
            {
                if (output_screen.display)
                {
                    for (int y = 0; y < output_screen.height; y++)
                        memset(output_screen.display[y], ' ', output_screen.width);
                }

                if (active_menu == &maps_menu)
                    menu_free(&maps_menu);

                active_menu = NULL;
            }
            else
            {
                menu_render(active_menu);
            }
        }
        sleep_ms(150); // debounce
    }
}