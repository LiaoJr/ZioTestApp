/*
 * log.h
 *
 *  Created on: 2023年9月4日
 *      Author: fengziwen
 */


#ifndef _LOG_H
#define _LOG_H


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define DEBUG                       // show all log

#define LOG_COLOR_ENABLE 1       	// log with color

#define log_printf(...) printf(__VA_ARGS__)   // 输出日志接口

#define CONSOLE_BUF_SIZE (1024 * 4)

#define LOG_LVL_ASSERT  0
#define LOG_LVL_ERROR   1
#define LOG_LVL_WARN    2
#define LOG_LVL_INFO    3
#define LOG_LVL_DEBUG   4
#define LOG_LVL_VERBOSE 5

void log_set_level(unsigned char level);
void log_output(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *format, ...);
void log_output_hex(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *msg,
                    unsigned char *dat, uint16_t dataLen);
void log_output_bin(unsigned char level, const char *tag, const char *file, const char *func, int line, const char *msg,
                    unsigned char *dat, uint16_t dataLen);

#ifndef NULL
#define NULL 0
#endif

#ifdef DEBUG
#define log_v(tag, ...) log_output(LOG_LVL_VERBOSE, tag, NULL, NULL, 0, __VA_ARGS__)
#define log_i(tag, ...) log_output(LOG_LVL_INFO, tag, NULL, NULL, 0, __VA_ARGS__)
#define log_d(tag, ...) log_output(LOG_LVL_DEBUG, tag, NULL, NULL, 0, __VA_ARGS__)
#define log_w(tag, ...) log_output(LOG_LVL_WARN, tag, NULL, NULL, 0, __VA_ARGS__)
#define log_e(tag, ...) log_output(LOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_a(tag, ...) log_output(LOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define log_bin_v(tag, msg, data, len) log_output_bin(LOG_LVL_VERBOSE, tag, NULL, NULL, 0, msg, data, len)
#define log_bin_i(tag, msg, data, len) log_output_bin(LOG_LVL_INFO, tag, NULL, NULL, 0, msg, data, len)
#define log_bin_d(tag, msg, data, len) log_output_bin(LOG_LVL_DEBUG, tag, NULL, NULL, 0, msg, data, len)
#define log_bin_w(tag, msg, data, len) log_output_bin(LOG_LVL_WARN, tag, NULL, NULL, 0, msg, data, len)
#define log_bin_e(tag, msg, data, len) log_output_bin(LOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, msg, data, len)
#define log_bin_a(tag, msg, data, len) log_output_bin(LOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, msg, data, len)

#define log_hex_v(tag, msg, data, len) log_output_hex(LOG_LVL_VERBOSE, tag, NULL, NULL, 0, msg, data, len)
#define log_hex_i(tag, msg, data, len) log_output_hex(LOG_LVL_INFO, tag, NULL, NULL, 0, msg, data, len)
#define log_hex_d(tag, msg, data, len) log_output_hex(LOG_LVL_DEBUG, tag, NULL, NULL, 0, msg, data, len)
#define log_hex_w(tag, msg, data, len) log_output_hex(LOG_LVL_WARN, tag, NULL, NULL, 0, msg, data, len)
#define log_hex_e(tag, msg, data, len) log_output_hex(LOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, msg, data, len)
#define log_hex_a(tag, msg, data, len) log_output_hex(LOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, msg, data, len)

#else

#define log_v(tag, ...)
#define log_i(tag, ...)
#define log_d(tag, ...)
#define log_w(tag, ...)
#define log_e(tag, ...)
#define log_a(tag, ...)

#define log_bin_v(tag, msg, data, len)
#define log_bin_i(tag, msg, data, len)
#define log_bin_d(tag, msg, data, len)
#define log_bin_w(tag, msg, data, len)
#define log_bin_e(tag, msg, data, len)
#define log_bin_a(tag, msg, data, len)

#define log_hex_v(tag, msg, data, len)
#define log_hex_i(tag, msg, data, len)
#define log_hex_d(tag, msg, data, len)
#define log_hex_w(tag, msg, data, len)
#define log_hex_e(tag, msg, data, len)
#define log_hex_a(tag, msg, data, len)

#endif

#define LOG_v(...) log_v(LOG_TAG, __VA_ARGS__)
#define LOG_I(...) log_i(LOG_TAG, __VA_ARGS__)
#define LOG_D(...) log_d(LOG_TAG, __VA_ARGS__)
#define LOG_W(...) log_w(LOG_TAG, __VA_ARGS__)
#define LOG_E(...) log_e(LOG_TAG, __VA_ARGS__)
#define LOG_A(...) log_a(LOG_TAG, __VA_ARGS__)

#define LOG_HEX_V(...) log_hex_v(LOG_TAG, __VA_ARGS__)
#define LOG_HEX_I(...) log_hex_i(LOG_TAG, __VA_ARGS__)
#define LOG_HEX_D(...) log_hex_d(LOG_TAG, __VA_ARGS__)
#define LOG_HEX_W(...) log_hex_w(LOG_TAG, __VA_ARGS__)
#define LOG_HEX_E(...) log_hex_e(LOG_TAG, __VA_ARGS__)
#define LOG_HEX_A(...) log_hex_a(LOG_TAG, __VA_ARGS__)

#define LOG_BIN_V(...) log_bin_v(LOG_TAG, __VA_ARGS__)
#define LOG_BIN_I(...) log_bin_i(LOG_TAG, __VA_ARGS__)
#define LOG_BIN_D(...) log_bin_d(LOG_TAG, __VA_ARGS__)
#define LOG_BIN_W(...) log_bin_w(LOG_TAG, __VA_ARGS__)
#define LOG_BIN_E(...) log_bin_e(LOG_TAG, __VA_ARGS__)
#define LOG_BIN_A(...) log_bin_a(LOG_TAG, __VA_ARGS__)

#define LOG_BIN_INF_D(...) log_bin_d(LOG_TAG, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif   //_LOG_H
