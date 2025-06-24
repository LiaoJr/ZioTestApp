/** 
 * 功能说明：
 *     1.HTTP 协议简单包装。
 *
 * 修改历史：
 *     1.2017-11-24 李先静<lixianjing@zlg.cn> 创建。
 *     2.2021-6-22  薛米样<xuemiyang@zlg.cn> 新增transfer_encoding声明
 */

#ifndef ZLG_IOT_SDK_SRC_IOT_HTTP_CLIENT_H
#define ZLG_IOT_SDK_SRC_IOT_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "zlg_iot_error.h"

#include "network_interface.h"
#include "timer_interface.h"
#include "zlg_iot_config.h"

#ifdef _ENABLE_THREAD_SUPPORT_
#include "threads_interface.h"
#endif

#define PROTOCOL_SIZE 8
#define HOSTNAME_SIZE 64

struct _zlg_iot_http_client_t;
typedef  struct _zlg_iot_http_client_t zlg_iot_http_client_t;

typedef void (*zlg_iot_http_client_on_header)(zlg_iot_http_client_t* c);
typedef IoT_Error_t (*zlg_iot_http_client_on_body)(zlg_iot_http_client_t* c);

struct _zlg_iot_http_client_t {
    unsigned char* body;
    size_t offset;
    size_t size;
    bool finish;

    char* cookie;
    char set_cookie[ZLG_IOT_HTTP_COOKIE_SIZE];

    /*response header*/
    int status;
    size_t content_length;
    char content_type[32];
    char transfer_encoding[16];

    /*url*/
    int port;
    const char* path;
    const char* protocol;
    const char* hostname;
   
    /*callback*/
    void *ctx;
    zlg_iot_http_client_on_body on_body;
    zlg_iot_http_client_on_header on_header;
};

IoT_Error_t zlg_iot_https_get(zlg_iot_http_client_t* c);
IoT_Error_t zlg_iot_https_post(zlg_iot_http_client_t* c, unsigned char* body, size_t body_size, 
        const char* type);

typedef struct _url_info {
  int port;
  char protocol[PROTOCOL_SIZE];
  char hostname[HOSTNAME_SIZE];
  char path[ZLG_IOT_HTTP_URL_PATH_LEN];
} url_info;

#ifdef __cplusplus
}
#endif

#endif