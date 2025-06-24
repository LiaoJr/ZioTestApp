/** 
 * 功能说明：
 *     1.HTTP 登录auth服务, 获取mqtt信息。
 *
 * 修改历史：
 *     1.2020-03-05 余晓蓝<yuxiaolan@zlg.cn> 创建。
 */

#ifndef ZLG_IOT_SDK_SRC_IOT_HTTP_AUTH_CLIENT_H
#define ZLG_IOT_SDK_SRC_IOT_HTTP_AUTH_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "zlg_iot_http_client.h"
#include "zlg_iot_log.h"

typedef enum _zlg_mqtt_info_err {
  Success = 0,
  InvalidParams = 1,
  DeviceNotFound = 2,
  ServerInternalError = 3
}zlg_mqtt_info_err;

typedef struct _zlg_mqtt_info {
    /*input*/
    const char* username;
    const char* password;

    size_t devices_nr;
    const char** device_ids;
    const char** device_types;

    /*output*/
    char host[32];
    int port;
    int sslport;
    char clientip[32];
    char uuid[40];
    char* token;
    char owner[32];
} zlg_mqtt_info;

void zlg_mqtt_info_init(zlg_mqtt_info* info);
void zlg_mqtt_info_deinit(zlg_mqtt_info* info);

zlg_mqtt_info_err zlg_iot_http_get_mqtt_info(zlg_mqtt_info* info, const char* hostname, const char* path, int port);
zlg_mqtt_info_err zlg_iot_https_get_mqtt_info(zlg_mqtt_info* info, const char* hostname, const char* path, int port);

#ifdef __cplusplus
}
#endif

#endif