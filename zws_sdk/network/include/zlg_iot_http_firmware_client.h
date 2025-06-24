/** 
 * 功能说明：
 *     1.HTTP 获取云端固件。
 *
 * 修改历史：
 *     1.2020-03-05 余晓蓝<yuxiaolan@zlg.cn> 创建。
 */

#ifndef ZLG_IOT_SDK_SRC_IOT_HTTP_FIRMWARE_CLIENT_H
#define ZLG_IOT_SDK_SRC_IOT_HTTP_FIRMWARE_CLIENT_H

#include "zlg_iot_http_client.h"
#include "zlg_iot_log.h"
#include "zlg_iot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

IoT_Error_t zlg_iot_download_firmware(const char* url, const char* token, zlg_iot_http_client_on_body on_data, void *ctx);

#ifdef __cplusplus
}
#endif

#endif