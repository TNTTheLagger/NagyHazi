//NEPTUN_COD:FF64XM NEV:Kaba Kevin Zsolt
#ifndef INC_3DGAME_PLATFORM_H
#define INC_3DGAME_PLATFORM_H

#include <stdint.h>

int platform_get_console_size(int *cols, int *rows);
uint64_t current_timestamp_ms(void);
void sleep_ms(int ms);
short platform_get_key_state(int key);

#endif //INC_3DGAME_PLATFORM_H