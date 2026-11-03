// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include "screen.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pspdebug.h>

#include "globals.h"

screen output_screen;

void get_screen_size()
{
    int cols = 68, rows = 34;

    output_screen.width = cols - 1;
    output_screen.height = rows;

    if (output_screen.display)
    {
        for (int i = 0; i < output_screen.height; i++)
        {
            free(output_screen.display[i]);
        }
        free(output_screen.display);
    }

    output_screen.display = malloc(output_screen.height * sizeof(char *));
    for (int i = 0; i < output_screen.height; i++)
    {
        output_screen.display[i] = malloc(output_screen.width * sizeof(char));
    }
}

static char *render_buf = NULL;
static int render_buf_size = 0;

void render_screen(void)
{
    // Clear screen and reset cursor
    pspDebugScreenSetXY(0, 0);

    for (int y = 0; y < output_screen.height; y++)
    {
        // Print exactly `width` characters from each row
        pspDebugScreenPrintf(
            "%.*s\n",
            output_screen.width,
            output_screen.display[y]);
    }
}

void free_screen()
{
    if (output_screen.display)
    {
        for (int i = 0; i < output_screen.height; i++)
        {
            free(output_screen.display[i]);
        }
        free(output_screen.display);
        output_screen.display = NULL;
    }

    free(render_buf);
    render_buf = NULL;
    render_buf_size = 0;
}
