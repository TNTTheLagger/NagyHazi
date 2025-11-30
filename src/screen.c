#include "screen.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// note: output_screen is defined in globals.c (via globals.h extern)
#include "globals.h"

screen output_screen;

void get_screen_size() {
    int cols = 80, rows = 24;
    platform_get_console_size(&cols, &rows);
    output_screen.width = cols - 1;
    output_screen.height = rows;
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

void render_screen() {
    // clear screen and write buffer with newlines
    fwrite("\x1b[H\x1b[J", 1, 6, stdout);

    int total_size = output_screen.width * output_screen.height;
    static char *buf = NULL;
    static int buf_size = 0;

    int needed = total_size + output_screen.height; // account for newlines
    if (buf_size < needed) {
        buf = realloc(buf, needed);
        buf_size = needed;
    }

    char *dst = buf;
    for (int y = 0; y < output_screen.height; y++) {
        memcpy(dst, &output_screen.display[y * output_screen.width], output_screen.width);
        dst += output_screen.width;
        *dst++ = '\n';
    }

    fwrite(buf, 1, dst - buf, stdout);
    fflush(stdout);
}