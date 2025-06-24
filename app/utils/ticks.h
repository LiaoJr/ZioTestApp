#ifndef _TICKS_H
#define _TICKS_H


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


uint32_t bsp_get_system_tick(void);
void bsp_delay_ms(uint32_t ms);
int64_t get_tick_sys_ns(void);
#endif // _TICKS_H
