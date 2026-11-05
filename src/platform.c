//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#include "debugmalloc.h"

#include "platform.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int platform_get_console_size(int *cols, int *rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return 0;
    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 1;
}

uint64_t current_timestamp_ms(void) {
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
short platform_get_key_state(int k) { return GetAsyncKeyState(k); }

#else // POSIX

#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>

int platform_get_console_size(int *cols, int *rows) {
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

uint64_t current_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000);
}

void sleep_ms(int ms) { usleep(ms * 1000); }

short platform_get_key_state(int k) { (void)k; return 0; }

#endif