// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifdef ENABLE_DEBUGMALLOC
#include "debugmalloc.h"
#endif

#include "screen.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __PSP__
#include <pspdebug.h>
#endif

#include "globals.h"

screen output_screen;

#if defined(__PSP__)

void get_screen_size(void)
{
    int cols = 68, rows = 34;

    output_screen.width = cols - 1;
    output_screen.height = rows;

    if (output_screen.display) {
        for (int i = 0; i < output_screen.height; i++)
            free(output_screen.display[i]);
        free(output_screen.display);
    }

    output_screen.display = malloc(output_screen.height * sizeof(char *));
    for (int i = 0; i < output_screen.height; i++) {
        output_screen.display[i] = malloc(output_screen.width * sizeof(char));
    }
}

void render_screen(void)
{
    pspDebugScreenSetXY(0, 0);

    for (int y = 0; y < output_screen.height; y++) {
        pspDebugScreenPrintf(
                "%.*s\n",
                output_screen.width,
                output_screen.display[y]);
    }
}

#else /* Windows / POSIX */

void get_screen_size(void)
{
    int cols = 80, rows = 24;
    platform_get_console_size(&cols, &rows);

    output_screen.width = cols - 1;
    output_screen.height = rows;

    if (output_screen.display) {
        for (int i = 0; i < output_screen.height; i++) {
            free(output_screen.display[i]);
        }
        free(output_screen.display);
    }

    output_screen.display = malloc(output_screen.height * sizeof(char *));
    for (int i = 0; i < output_screen.height; i++) {
        output_screen.display[i] = malloc(output_screen.width * sizeof(char));
    }
}

void update_screen_size(void)
{
    int cols = 80, rows = 24;
    platform_get_console_size(&cols, &rows);

    if (output_screen.display) {
        for (int i = 0; i < output_screen.height; i++) {
            free(output_screen.display[i]);
        }
        free(output_screen.display);
    }

    output_screen.width = cols - 1;
    output_screen.height = rows;

    output_screen.display = malloc(output_screen.height * sizeof(char *));
    for (int i = 0; i < output_screen.height; i++) {
        output_screen.display[i] = malloc(output_screen.width * sizeof(char));
    }
}

static char *render_buf = NULL;
static int render_buf_size = 0;

void render_screen(void)
{
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
        memcpy(dst, output_screen.display[y], output_screen.width);
        dst += output_screen.width;
        *dst++ = '\n';
    }

    fwrite(render_buf, 1, dst - render_buf, stdout);
    fflush(stdout);
}

#endif

void free_screen(void)
{
    if (output_screen.display) {
        for (int i = 0; i < output_screen.height; i++) {
            free(output_screen.display[i]);
        }
        free(output_screen.display);
        output_screen.display = NULL;
    }

#ifndef __PSP__
    free(render_buf);
    render_buf = NULL;
    render_buf_size = 0;
#endif
}
