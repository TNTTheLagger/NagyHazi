// NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifndef INC_3DGAME_PLATFORM_H
#define INC_3DGAME_PLATFORM_H

#include <stdint.h>

#ifdef _WIN32
#ifndef strdup
#define strdup _strdup
#endif
#endif

/* Platform-independent key bitmask. Test with platform_get_keys() & KEY_*. */
#define KEY_UP       (1u << 0)
#define KEY_DOWN     (1u << 1)
#define KEY_LEFT     (1u << 2)
#define KEY_RIGHT    (1u << 3)
#define KEY_ESCAPE   (1u << 4)
#define KEY_ENTER    (1u << 5)

/* Console/terminal size in characters. */
int platform_get_console_size(int *cols, int *rows);

/* Monotonic-ish time in milliseconds. */
uint64_t current_timestamp_ms(void);

/* Sleep for ms milliseconds. */
void sleep_ms(int ms);

/* Bitmask of currently pressed keys (KEY_* constants). */
int platform_get_keys(void);

#endif // INC_3DGAME_PLATFORM_H
