// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifdef ENABLE_DEBUGMALLOC
#include "debugmalloc.h"
#endif

#include "platform.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int platform_get_console_size(int *cols, int *rows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(h, &csbi))
        return 0;
    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 1;
}

uint64_t current_timestamp_ms(void)
{
    FILETIME ft;
    unsigned long long tmpres = 0;
    GetSystemTimeAsFileTime(&ft);
    tmpres |= ((unsigned long long)ft.dwHighDateTime) << 32;
    tmpres |= ft.dwLowDateTime;
    tmpres /= 10;
    tmpres -= 11644473600000000ULL;
    return (uint64_t)(tmpres / 1000ULL);
}

void sleep_ms(int ms) { Sleep(ms); }

int platform_get_keys(void)
{
    int keys = 0;
    if (GetAsyncKeyState(VK_UP) & 0x8000)      keys |= KEY_UP;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)    keys |= KEY_DOWN;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)    keys |= KEY_LEFT;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)   keys |= KEY_RIGHT;
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)  keys |= KEY_ESCAPE;
    if (GetAsyncKeyState(VK_RETURN) & 0x8000)  keys |= KEY_ENTER;
    if (GetAsyncKeyState('W') & 0x8000)        keys |= KEY_UP;
    if (GetAsyncKeyState('S') & 0x8000)        keys |= KEY_DOWN;
    if (GetAsyncKeyState('A') & 0x8000)        keys |= KEY_LEFT;
    if (GetAsyncKeyState('D') & 0x8000)        keys |= KEY_RIGHT;
    return keys;
}

#elif defined(__PSP__)
#include <pspctrl.h>
#include <pspkernel.h>
#include <stdio.h>

int platform_get_console_size(int *cols, int *rows)
{
    (void)cols;
    (void)rows;
    /* PSP screen size is fixed; get_screen_size() uses its own constants. */
    return 1;
}

uint64_t current_timestamp_ms(void)
{
    /* sceKernelGetSystemTimeLow() is in microseconds. */
    return (uint64_t)sceKernelGetSystemTimeLow() / 1000ULL;
}

void sleep_ms(int ms)
{
    sceKernelDelayThread(ms * 1000);
}

int platform_get_keys(void)
{
    static int first = 1;
    static SceCtrlData pad;
    if (first) {
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
        first = 0;
    }
    sceCtrlReadBufferPositive(&pad, 1);

    int keys = 0;
    if (pad.Buttons & PSP_CTRL_UP)    keys |= KEY_UP;
    if (pad.Buttons & PSP_CTRL_DOWN)  keys |= KEY_DOWN;
    if (pad.Buttons & PSP_CTRL_LEFT)  keys |= KEY_LEFT;
    if (pad.Buttons & PSP_CTRL_RIGHT) keys |= KEY_RIGHT;
    if (pad.Buttons & PSP_CTRL_START) keys |= KEY_ESCAPE; /* START opens menu */
    if (pad.Buttons & PSP_CTRL_CROSS) keys |= KEY_ENTER;
    return keys;
}

#else /* POSIX (Linux, macOS, etc.) */
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

int platform_get_console_size(int *cols, int *rows)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        *cols = 80;
        *rows = 24;
        return 0;
    }
    *cols = w.ws_col;
    *rows = w.ws_row;
    return 1;
}

uint64_t current_timestamp_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000);
}

void sleep_ms(int ms) { usleep(ms * 1000); }

/* POSIX terminals have no convenient non-blocking key input without
 * termios manipulation, so the POSIX desktop build reports no keys.
 * (This matches the pre-merge behaviour on Linux.) */
int platform_get_keys(void)
{
    return 0;
}

#endif
