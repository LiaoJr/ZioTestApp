#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#define BUFF_SIZE 256

BOOL myinvert_report_temperature_too_high(client_t* client, float value){
    params_t params;
    char buff[BUFF_SIZE];
    return_value_if_fail(client != NULL, FALSE);

    char info[64] = { 0x00 };
    memset(info, 0x00, sizeof(info));
    snprintf(info, sizeof(info), "temperature = %0.2f too high", value);
    request_init(&params, buff, sizeof(buff));
    // 日志上报的字段和格式固定，不能修改
    params_append_string(&params, STR_KEY_NAME, STR_SCHEMA_NAME_LOG);
    params_append_int(&params, STR_KEY_LOG_LEVEL, LOG_LEVEL_ERROR);
    params_append_string(&params, STR_KEY_LOG_INFO, info);
    params_append_uint(&params, STR_KEY_LOG_TIME, 1676615749296);
    // 上报不同等级的日志通过设置STR_KEY_LOG_LEVEL进行区分，client_report_error,client_report_warning接口废弃。
    return client_report_log(client, &params);
}

BOOL myinvert_report_log_multi(client_t* client, const char* log_multi){
    params_t params;
    char buff[BUFF_SIZE];
    return_value_if_fail(client != NULL, FALSE);

    float value = 100;
    char info[64] = { 0x00 };
    memset(info, 0x00, sizeof(info));
    snprintf(info, sizeof(info), "temperature = %0.2f too high", value);
    request_init(&params, buff, sizeof(buff));
    // 日志上报的字段需要和云端定义的日志字段一致
    params_append_string(&params, STR_KEY_NAME, STR_SCHEMA_NAME_LOG);
    params_append_int(&params, STR_KEY_LOG_LEVEL, LOG_LEVEL_ERROR);
    params_append_string(&params, STR_KEY_LOG_INFO, info);
    params_append_uint(&params, STR_KEY_LOG_TIME, 1676615749296);
    // 上报不同等级的日志通过设置STR_KEY_LOG_LEVEL进行区分，client_report_error,client_report_warning接口废弃。
    return client_report_log_multi(client, log_multi, &params);
}

