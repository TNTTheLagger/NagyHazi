//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include "screen.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"

screen output_screen;

void get_screen_size() {
    int cols = 80, rows = 24;
    platform_get_console_size(&cols, &rows);
    output_screen.width = cols - 1;
    output_screen.height = rows;
    if (output_screen.display) free(output_screen.display);
    output_screen.display = malloc(output_screen.width * output_screen.height * sizeof(char));
}

void update_screen_size() {
    int cols = 80, rows = 24;
    platform_get_console_size(&cols, &rows);
    output_screen.width = cols - 1;
    output_screen.height = rows;
    free(output_screen.display);
    output_screen.display = malloc(output_screen.width * output_screen.height * sizeof(char));
}

static char *render_buf = NULL;
static int render_buf_size = 0;

void render_screen() {
    fwrite("\x1b[H\x1b[J", 1, 6, stdout);

    int total_size = output_screen.width * output_screen.height;
    int needed = total_size + output_screen.height;

    if (render_buf_size < needed) {
        char *tmp = realloc(render_buf, needed);
        if (!tmp) return;
        render_buf = tmp;
        render_buf_size = needed;
    }

    char *dst = render_buf;
    for (int y = 0; y < output_screen.height; y++) {
        memcpy(dst, &output_screen.display[y * output_screen.width], output_screen.width);
        dst += output_screen.width;
        *dst++ = '\n';
    }

    fwrite(render_buf, 1, dst - render_buf, stdout);
    fflush(stdout);
}

void free_screen() {
    if (output_screen.display) {
        free(output_screen.display);
        output_screen.display = NULL;
    }
    free(render_buf);
    render_buf = NULL;
    render_buf_size = 0;
}