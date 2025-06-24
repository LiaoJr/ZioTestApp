/*
 * log.c
 *
 *  Created on: 2023年9月4日
 *      Author: fengziwen
 */

#include "stdlib.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "log.h"


#ifndef PTHREAD
#define pthread_mutex_lock(x)
#define pthread_mutex_unlock(x)
#else
#include <pthread.h>
#include <sys/time.h>
pthread_mutex_t g_wlt_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

#define MAX_BYTE_PER_LINE   16
#define MAX_BYTE_LINE_SPLIT 8

#if LOG_COLOR_ENABLE
/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START "\033["
#define CSI_END   "\033[0m"
/* output log front color */
#define F_BLACK   "30;"
#define F_RED     "31;"
#define F_GREEN   "32;"
#define F_YELLOW  "33;"
#define F_BLUE    "34;"
#define F_MAGENTA "35;"
#define F_CYAN    "36;"
#define F_WHITE   "37;"
/* output log background color */
#define B_NULL
#define B_BLACK   "40;"
#define B_RED     "41;"
#define B_GREEN   "42;"
#define B_YELLOW  "43;"
#define B_BLUE    "44;"
#define B_MAGENTA "45;"
#define B_CYAN    "46;"
#define B_WHITE   "47;"
/* output log fonts style */
#define S_BOLD      "1m"
#define S_UNDERLINE "4m"
#define S_BLINK     "5m"
#define S_NORMAL    "22m"

/* output log default color definition: [front color] + [background color] + [show style] */
#ifndef ELOG_COLOR_ASSERT
#define ELOG_COLOR_ASSERT (F_MAGENTA B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_ERROR
#define ELOG_COLOR_ERROR (F_RED B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_WARN
#define ELOG_COLOR_WARN (F_YELLOW B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_INFO
#define ELOG_COLOR_INFO (F_CYAN B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_DEBUG
#define ELOG_COLOR_DEBUG (F_GREEN B_NULL S_NORMAL)
#endif
#ifndef ELOG_COLOR_VERBOSE
#define ELOG_COLOR_VERBOSE (F_BLUE B_NULL S_NORMAL)
#endif
#endif /* ELOG_COLOR_ENABLE */
#define CR "\r\n"

#ifdef ANDROID
static android_LogPriority prio = ANDROID_LOG_VERBOSE;
#define log_printf(...) __android_log_print(prio, LOG_TAG, __VA_ARGS__)
#endif
static unsigned char log_level = LOG_LVL_DEBUG;

/* level output info */
static const char *level_output_info[] = {
        [LOG_LVL_ASSERT]  = "A/",
        [LOG_LVL_ERROR]   = "E/",
        [LOG_LVL_WARN]    = "W/",
        [LOG_LVL_INFO]    = "I/",
        [LOG_LVL_DEBUG]   = "D/",
        [LOG_LVL_VERBOSE] = "V/",
};

#if LOG_COLOR_ENABLE
/* color output info */
static const char *color_output_info[] = {
        [LOG_LVL_ASSERT]  = ELOG_COLOR_ASSERT,
        [LOG_LVL_ERROR]   = ELOG_COLOR_ERROR,
        [LOG_LVL_WARN]    = ELOG_COLOR_WARN,
        [LOG_LVL_INFO]    = ELOG_COLOR_INFO,
        [LOG_LVL_DEBUG]   = ELOG_COLOR_DEBUG,
        [LOG_LVL_VERBOSE] = ELOG_COLOR_VERBOSE,
};
#endif /* ELOG_COLOR_ENABLE */

static bool log_can_output(unsigned char level)
{
#ifdef ANDROID
    if(LOG_LVL_ASSERT == level)
    {
        prio = ANDROID_LOG_FATAL;
    }
    else if(LOG_LVL_ERROR == level)
    {
        prio = ANDROID_LOG_ERROR;
    }
    else if(LOG_LVL_WARN == level)
    {
        prio = ANDROID_LOG_WARN;
    }
    else if(LOG_LVL_INFO == level)
    {
        prio = ANDROID_LOG_INFO;
    }
    else if(LOG_LVL_DEBUG == level)
    {
        prio = ANDROID_LOG_DEBUG;
    }
    else if(LOG_LVL_VERBOSE == level)
    {
        prio = ANDROID_LOG_VERBOSE;
    }
#endif
    return log_level >= level;
}

const char *log_port_get_time(void)
{
    static char cur_system_time[ 32 ] = {0};
    time_t      now;
    struct tm * p_tm, *p_cm;
    time(&now);
    p_cm = localtime(&now);
    p_tm = gmtime(&now);
    struct timeval time;
    gettimeofday(&time, NULL);
    snprintf(cur_system_time, 32, "%04d-%02d-%02d %02d:%02d:%02d:%03ld", p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday, p_tm->tm_hour + 8, p_tm->tm_min, p_tm->tm_sec, (time.tv_usec / 1000) % 1000);
    return cur_system_time;
}

static char rt_log_buf[ CONSOLE_BUF_SIZE ] = {0};
void        log_output_bin(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *msg, unsigned char *dat, uint16_t dataLen)
{
    size_t length;

    uint16_t iLoop           = dataLen / MAX_BYTE_PER_LINE;
    uint16_t iByteInLastLine = dataLen % MAX_BYTE_PER_LINE;

    uint16_t x = 0;
    uint16_t y = 0;
    memset(rt_log_buf, 0, sizeof(rt_log_buf));
    if(!log_can_output(level))
    {
        return;
    }
    if(!tag)
    {
        return;
    }
    pthread_mutex_lock(&g_wlt_lock);
#if LOG_COLOR_ENABLE
    /* add CSI start sign and color info */
    log_printf(CSI_START);
    log_printf("%s", color_output_info[ level ]);
#endif
    length = sprintf(rt_log_buf, "[%s%s]", level_output_info[ level ], tag);
    length += sprintf(rt_log_buf + length, "[%s]", log_port_get_time());
    length += sprintf(rt_log_buf + length, "%s [ %04X / %d ]\r\n", msg, dataLen, dataLen);
    rt_log_buf[ length ] = 0;
    log_printf("%s", rt_log_buf);

    for(; x < iLoop; ++x)
    {
        length = 0;
        memset(rt_log_buf, 0, sizeof(rt_log_buf));
        length += sprintf(rt_log_buf + length, "%08X  ", x * MAX_BYTE_PER_LINE);
        for(y = 0; y < MAX_BYTE_PER_LINE; ++y)
        {
            if(MAX_BYTE_LINE_SPLIT == y)
            {
                length += sprintf(rt_log_buf + length, "  ");
            }
            length += sprintf(rt_log_buf + length, "%02X ", dat[ (x * MAX_BYTE_PER_LINE) + y ]);
        }
        length += sprintf(rt_log_buf + length, "   ");
        for(y = 0; y < MAX_BYTE_PER_LINE; ++y)
        {
            if((dat[ (x * MAX_BYTE_PER_LINE) + y ] >= 0x20) && (dat[ (x * MAX_BYTE_PER_LINE) + y ] <= 0x7e))
            {
                length += sprintf(rt_log_buf + length, "%c", dat[ (x * MAX_BYTE_PER_LINE) + y ]);
            }
            else
            {
                length += sprintf(rt_log_buf + length, ".");
            }
        }
        rt_log_buf[ length ] = 0;
        log_printf("%s", rt_log_buf);
        log_printf(CR);
        length = 0;
    }
    if(iByteInLastLine > 0)
    {
        uint16_t padLoop    = 0;
        uint16_t paddingLen = (MAX_BYTE_PER_LINE - iByteInLastLine) * 3;
        if(iByteInLastLine <= MAX_BYTE_LINE_SPLIT)
        {
            paddingLen += 2;
        }
        length = 0;
        length += sprintf(rt_log_buf + length, "%08X  ", x * MAX_BYTE_PER_LINE);
        for(y = 0; y < iByteInLastLine; ++y)
        {
            if(MAX_BYTE_LINE_SPLIT == y)
            {
                length += sprintf(rt_log_buf + length, "  ");
            }
            length += sprintf(rt_log_buf + length, "%02X ", dat[ (x * MAX_BYTE_PER_LINE) + y ]);
        }
        for(padLoop = 0; padLoop < paddingLen; ++padLoop)
        {
            length += sprintf(rt_log_buf + length, " ");
        }
        length += sprintf(rt_log_buf + length, "   ");
        for(y = 0; y < iByteInLastLine; ++y)
        {
            //	if(isprint((int) (dat[i + (x * MAX_BYTE_PER_LINE) + y])))
            if((dat[ (x * MAX_BYTE_PER_LINE) + y ] >= 0x20) && (dat[ (x * MAX_BYTE_PER_LINE) + y ] <= 0x7e))
            {
                length += sprintf(rt_log_buf + length, "%c", dat[ (x * MAX_BYTE_PER_LINE) + y ]);
            }
            else
            {
                length += sprintf(rt_log_buf + length, ".");
            }
        }
    }
    rt_log_buf[ length ] = 0;
    log_printf("%s", rt_log_buf);
    length = 0;
    if(file)
    {
        log_printf("[ function: %s at file:( %s ) in line ( %d ) ]", func, file, line);
    }
    log_printf(CR);
#if LOG_COLOR_ENABLE
    /* add CSI end sign */
    log_printf(CSI_END);
#endif
    pthread_mutex_unlock(&g_wlt_lock);
}

void log_output_hex(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *msg, unsigned char *dat, uint16_t dataLen)
{
    size_t   length = 0;
    uint32_t i      = 0;
    if(!log_can_output(level))
    {
        return;
    }

    if(!tag)
    {
        return;
    }
    pthread_mutex_lock(&g_wlt_lock);
#if LOG_COLOR_ENABLE
    /* add CSI start sign and color info */
    length += sprintf(rt_log_buf + length, "%s", CSI_START);
    length += sprintf(rt_log_buf + length, "%s", color_output_info[ level ]);
#endif
    length += sprintf(rt_log_buf + length, "[%s%s]", level_output_info[ level ], tag);
    length += sprintf(rt_log_buf + length, "[%s]", log_port_get_time());
    length += sprintf(rt_log_buf + length, "%s [ %04X / %d ]" CR, msg, dataLen, dataLen);
    for(i = 0; i < dataLen; i++)
    {
        length += sprintf(rt_log_buf + length, "%02X ", dat[ i ]);
        if(((i + 1) % 0x10) == 0)
        {
            rt_log_buf[ length ] = 0;
            log_printf("%s", rt_log_buf);
            length = 0;
        }
    }
    rt_log_buf[ length ] = 0;
    log_printf("%s", rt_log_buf);

    if(file)
    {
        log_printf("[ function: %s at file:( %s ) in line ( %d ) ]", func, file, line);
    }

//    log_printf(CR);
#if LOG_COLOR_ENABLE
    /* add CSI end sign */
    log_printf(CSI_END);
#endif
    pthread_mutex_unlock(&g_wlt_lock);
}

void log_set_level(unsigned char level)
{
    log_level = level;
}

void log_output(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *format, ...)
{
    size_t  length = 0;
    va_list args;
    if(!log_can_output(level))
    {
        return;
    }
    if(!tag)
    {
        return;
    }
    pthread_mutex_lock(&g_wlt_lock);
#if LOG_COLOR_ENABLE
    /* add CSI start sign and color info */
    length += sprintf(rt_log_buf + length, "%s", CSI_START);
    length += sprintf(rt_log_buf + length, "%s", color_output_info[ level ]);
#endif
    length += sprintf(rt_log_buf + length, "[%s%s]", level_output_info[ level ], tag);
    length += sprintf(rt_log_buf + length, "[%s]", log_port_get_time());
    va_start(args, format);
    length += vsprintf(rt_log_buf + length, format, args);
    if(rt_log_buf[ strlen(rt_log_buf) - 1 ] == '\n')
    {
        rt_log_buf[ strlen(rt_log_buf) - 1 ] = 0;
    }
    va_end(args);

    if(file)
    {
        length += sprintf(rt_log_buf + length, "[ function: %s at file:( %s ) in line ( %d ) ]", func, file, line);
    }

    log_printf("%s", rt_log_buf);
    log_printf(CR);
#if LOG_COLOR_ENABLE
    /* add CSI end sign */
    log_printf(CSI_END);
#endif
    pthread_mutex_unlock(&g_wlt_lock);
}
