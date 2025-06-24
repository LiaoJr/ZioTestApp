/** 
 * 功能说明：
 *     1.HTTP 上传文件。
 *
 * 修改历史：
 *     1.2021-03-09 薛米样<xuemiyang@zlg.cn> 创建。
 */

#include <stdio.h>
#include "zlg_iot_http_upload_file_client.h"

IoT_Error_t url_parser(const char* url, url_info *info);

long zlg_iot_upload_file(const char* url, const char* token, const char* file_path, unsigned int block_size, unsigned long offset, const char* file_name, zlg_iot_http_client_on_body on_data, void *ctx) {
  if (NULL == url || NULL == token || NULL == on_data || block_size == 0) {
    return NULL_VALUE_ERROR;
  }

  char form_request[1024];
  size_t form_request_size = sprintf(form_request, "------ZLG_IOT_SDK_SRC_IOT_HTTP_UPLOAD_FILE_CLIENT_H\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n\r\n", file_name);
  char form_end[] = "\r\n------ZLG_IOT_SDK_SRC_IOT_HTTP_UPLOAD_FILE_CLIENT_H--\r\n";
  size_t form_end_size = sizeof(form_end);

  zlg_iot_http_client_t c;
  char cookies[1024] = {0x00};
  url_info uinfo;
  IoT_Error_t ret = url_parser(url, &uinfo);
  if (ret != SUCCESS) {
    return ret;
  }
  
  snprintf(cookies, sizeof(cookies), "jwt=%s", token);

  FILE* fp = fopen(file_path, "rb");
  if (fp == NULL) {
    return FAILURE;
  }
  
  size_t result = fseek(fp, 0l, SEEK_END);
  if (result != 0) {
    goto fexit;
  }
  long size = ftell(fp);
  if (size <= 0) {
    goto fexit;
  }
  result = fseek(fp, offset, SEEK_SET);
  if (result != 0) {
    goto fexit;
  }
  
  while (offset < size) {
    unsigned char* data = malloc(block_size);
    if (data == NULL) {
      break;
    }
    memset(data, 0x00, sizeof(data));
    size_t data_size = fread(data, 1, block_size, fp);
    if (data_size <= 0) {
      break;
    }

    char headers[256] = {0x00};
    size_t headers_size = sprintf(headers, "multipart/form-data; boundary=----ZLG_IOT_SDK_SRC_IOT_HTTP_UPLOAD_FILE_CLIENT_H\r\nZLG-Content-Offset: %ld\r\nZLG-Content-Size: %ld", offset, size);
    offset += data_size;

    unsigned char* body = malloc(headers_size + form_request_size + data_size + form_end_size);
    if (body == NULL) {
      free(data);
      data = NULL;
      break;
    }
    memset(body, 0x00, sizeof(body));
    memcpy(body, form_request, form_request_size);
    memcpy(body + form_request_size, data, data_size);
    memcpy(body + form_request_size + data_size, form_end, form_end_size);
    
    free(data);
    data = NULL;

    memset(&c, 0x00, sizeof(c));
    c.hostname = uinfo.hostname;
    c.path = uinfo.path;
    c.port = uinfo.port;
    c.cookie = cookies;
    c.ctx = ctx;
    c.on_body = on_data;
    c.protocol = uinfo.protocol;
    IoT_Error_t ret = SUCCESS;
    // 每个分块最多请求5次，5次都失败结束循环。
    for (size_t i = 0; i < 5; i++) {
      ret = zlg_iot_https_post(&c, body, form_request_size + data_size + form_end_size, headers);
      if (ret == SUCCESS) {
        break;
      }
    }
    free(body);
    body = NULL;
    if (ret != SUCCESS) {
      break;
    }
  }

fexit:
  fclose(fp);
  fp = NULL;

  return offset;
}