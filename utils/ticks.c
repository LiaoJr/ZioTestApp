#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "ticks.h"


uint32_t bsp_get_system_tick(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int64_t get_tick_sys_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    return (((int64_t)ts.tv_sec*1000000000LL+(int64_t)ts.tv_nsec));
}

void bsp_delay_ms(uint32_t ms)
{
    uint32_t tick_now = bsp_get_system_tick();

    while(bsp_get_system_tick()  < tick_now + ms)
    {
        usleep(1000);
    }
}
