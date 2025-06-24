/** 
 * 功能说明：
 *     1.模拟文件上传。只用于示例，切不可直接拷贝用于实际开发中。
 *
 * 修改历史：
 *     1.2021-3-10 薛米样<xuemiyang@zlg.cn> 创建。
 */

#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_http_upload_file_client.h"
#include "client.h"
#ifdef _WIN32
#include <io.h>
#include <process.h>
#elif __linux__
#include <unistd.h>
#endif

IoT_Error_t on_data(zlg_iot_http_client_t* c) {
  IOT_INFO("uploadfile content:");
  IOT_INFO("%s", c->body);

  return SUCCESS;
}

/* 
  demo运行示例：
  ./bin/upload_file_example http://zws.zlgcloud.com/v1/filemanager/upload/commonfile/1155020_0202201311331.log DemoV1 321 123456789
*/
int main(int argc, char *argv[]) {
  IoT_Error_t rc = FAILURE;
  dev_meta_info_t meta;
  zlg_mqtt_info mqtt_info;

  IOT_INFO("Usage: %s upload_host type id secret", argv[0]);

  if (argc < 5) {
    IOT_ERROR("invalid arguments");
    exit(1);
  }

  memset(&meta, 0x00, sizeof(meta));
  strncpy(meta.type, argv[2], strlen(argv[2]));
  strncpy(meta.id, argv[3], strlen(argv[3]));
  strncpy(meta.secret, argv[4], strlen(argv[4]));

  zlg_mqtt_info_init(&mqtt_info);

  if (!get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT, &meta)) {
    IOT_ERROR("get mqtt info error");
    exit(1);
  }
  // 模拟把本地的upload_file_expl.c上传上去
  const char* file_path = "demos/src/upload_file_expl.c";
  FILE* fp = fopen(file_path, "rb");
  if (fp == NULL) {
    IOT_ERROR("open file error");
    exit(1);
  }
  
  fseek(fp, 0l, SEEK_END);
  long size = ftell(fp);
  fclose(fp);

  const char* url = argv[1];  // 通过该url把文件上传到服务器，该url由云端动态下发
  // 动态从url中提取最后的1155020_0202201311331.log作为文件名，该文件名由[设备id_召回文件名]组成，可避免不同设备下文件名冲突的问题
  char* file_name = strrchr(url, '/');
  file_name++;  // 跳过'/'本身

  rc = zlg_iot_upload_file(url, mqtt_info.token, file_path, 1000, 0, file_name, on_data, NULL);
  if (rc < size) {
    IOT_ERROR("upload file failure");
  }

  zlg_mqtt_info_deinit(&mqtt_info);

  return SUCCESS;
}