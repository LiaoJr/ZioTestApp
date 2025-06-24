/** 
 * 功能说明：
 *     1.模拟固件下载。只用于示例，切不可直接拷贝用于实际开发中。
 *
 * 修改历史：
 *     1.2017-11-24 李先静<lixianjing@zlg.cn> 创建。
 *     2.2019-04-24 王文坤<wangwenkun@zlg.cn> 修改。
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "client.h"
#include "device.h"

#include "zlg_iot_version.h"
#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_http_firmware_client.h"
#include "zlg_iot_mqtt_client.h"
#include "zlg_iot_mqtt_client_interface.h"

static zlg_mqtt_info mqtt_info;
size_t firmware_sum_len = 0;  // 已接收到的固件总长度

IoT_Error_t on_data(zlg_iot_http_client_t* c) {
  // c->body是接收到的http body的内容，指向文件的二进制内容
  // c->size是接收到的http body的长度
  IOT_INFO("body size:%ld", c->size);
  IOT_INFO("body content:%s", c->body);

  // 注意：
  // 由于接收http数据时的缓冲大小为ZLG_IOT_HTTP_BUFF_SIZE，当固件大于ZLG_IOT_HTTP_BUFF_SIZE时，文件内容会分多次接收，on_data会被回调多次
  // 上面只是把接收到固件的内容打印出来，实际场景可以自行把body内容写入文件，所有on_data回调中的c->body之和，才是完整的固件内容
  
  firmware_sum_len += c->size;
  IOT_INFO("firmware_sum_len:%ld\n",firmware_sum_len);
  return SUCCESS;
}

// 示例
// ./bin/download_firmware_example "http://zws.zlgcloud.com:80/v1/out-firmware-mgmt/download/url?tenant_id=722&owner_id=1102&model_id=7&version=00.02.03&sign=2933b25928b485eff4bc75cf079e28ac&things_tenant_id=722&things_id=1072226" dev_type dev_id dev_secret
// 参数1：云端下发的固件升级的url
// 参数2: 设备类型
// 参数3: 设备ID
// 参数4: 设备密钥
int main(int argc, const char* argv[]) {
  IoT_Error_t rc = FAILURE;
  dev_meta_info_t meta;

  IOT_INFO("Usage: %s download_url type id secret", argv[0]);

  if (argc < 5) {
    IOT_ERROR("invalid arguments");
    exit(1);
  }

  memset(&meta, 0x00, sizeof(meta));
  strncpy(meta.type, argv[2], strlen(argv[2]));
  strncpy(meta.id, argv[3], strlen(argv[3]));
  strncpy(meta.secret, argv[4], strlen(argv[4]));

  zlg_mqtt_info_init(&mqtt_info);

  if (!get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT,&meta))
  {
    IOT_ERROR("get mqtt info error");
    exit(1);
  }
  
  firmware_sum_len = 0;  // 已接收到的固件总长度

  // on_data可能被回调单次或者多次
  rc = zlg_iot_download_firmware(argv[1], mqtt_info.token, on_data, NULL);
  // 当zlg_iot_download_firmware返回后，表示所有数据都被接收完毕，on_data的回调也结束了
  if (rc != SUCCESS) {
    IOT_ERROR("download firmware failure");
  }

  zlg_mqtt_info_deinit(&mqtt_info);

  return rc;
}