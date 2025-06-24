/** 
 * 功能说明：
 *     1.HTTP 上传文件。
 *
 * 修改历史：
 *     1.2021-03-09 薛米样<xuemiyang@zlg.cn> 创建。
 */

#ifndef ZLG_IOT_SDK_SRC_IOT_HTTP_UPLOAD_FILE_CLIENT_H
#define ZLG_IOT_SDK_SRC_IOT_HTTP_UPLOAD_FILE_CLIENT_H

#include "zlg_iot_error.h"
#include "zlg_iot_http_client.h"

#ifdef __cplusplus
extern "C" {
#endif

long zlg_iot_upload_file(const char* url, const char* token, const char* file_path, unsigned int block_size, unsigned long offset, const char* file_name, zlg_iot_http_client_on_body on_data, void *ctx);

#ifdef __cplusplus
}
#endif
#endif
