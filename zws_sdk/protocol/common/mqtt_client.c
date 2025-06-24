/** 
 * 功能说明：
 *     1.Mqtt publish函数实现。
 *
 * 修改历史：
 *     1.2017-11-24 李先静<lixianjing@zlg.cn> 创建。
 */

#include "mqtt_client.h"
#include "zlg_iot_config.h"
#include "zlg_iot_log.h"
#include "zlg_iot_version.h"
#include "zlg_iot_mqtt_client_interface.h"

static void print_buff(const char* buff, size_t size, const char* topic, int ret) {
    const char* p = buff;
    printf("topic:%s ret:%d content[", topic, ret);
    while(p < (buff + size)) {
        printf("%s:", p);
        p += strlen(p) + 1;
    }
    printf("]\n");
}

/**
 * 可在此处添加Publish失败时的处理。
 * 
 */
BOOL mqtt_publish(mqtt_client_t* stream, int qos, int r, const char* topic, const char* buff, size_t size) {
	  IoT_Error_t rc = FAILURE;
    IoT_Publish_Message_Params params;
    ZLG_IoT_Client* mq = (ZLG_IoT_Client*)stream->ctx;

    (void)r;
	  params.qos = qos;
	  params.isRetained = 0;
    params.payloadLen = size;
	  params.payload = (void *)buff;

    rc = zlg_iot_mqtt_publish(mq, topic, strlen(topic), &params);

	  print_buff(buff, size, topic, rc);

	  return SUCCESS == rc;
}

/*登录认证服务器获取MQTT服务器的信息*/
BOOL get_mqtt_info_impl(zlg_mqtt_info *info, const char *hostname, const char *path, int port,
                               dev_meta_info_t meta[], int meta_size, const char *username, const char *password)
{
  char **device_ids;
  char **device_types;

  device_ids = (char **)calloc(meta_size, sizeof(char *));
  device_types = (char **)calloc(meta_size, sizeof(char *));
  if (device_ids == NULL || device_types == NULL)
  {
    return FALSE;
  }

  for (int i = 0; i < meta_size; ++i)
  {
    device_ids[i] = (char *)calloc(64 + 1, sizeof(char));
    strncpy(device_ids[i], meta[i].id, 64);

    device_types[i] = (char *)calloc(64 + 1, sizeof(char));
    strncpy(device_types[i], meta[i].type, 64);
  }

  if(NULL == username)
    username = meta[0].type;
  if(NULL == password)
    password = meta[0].secret;

  info->username = username;
  info->password = password;
  info->devices_nr = meta_size;
  info->device_ids = (const char **)device_ids;
  info->device_types = (const char **)device_types;

  IOT_INFO("hostname:%s path:%s port:%d", hostname,path,port);
  #ifdef WITH_MBED_TLS
    zlg_mqtt_info_err rc = zlg_iot_https_get_mqtt_info(info, hostname, path, port);
  #else
    zlg_mqtt_info_err rc = zlg_iot_http_get_mqtt_info(info, hostname, path, port);
  #endif

  for (int i = 0; i < meta_size; ++i)
  {
    free(device_ids[i]);
    free(device_types[i]);
  }
  free(device_ids);
  free(device_types);

  if (rc == SUCCESS)
  {
    IOT_INFO("host:%s", info->host);
    IOT_INFO("port:%d", info->port);
    IOT_INFO("sslport:%d", info->sslport);
    IOT_INFO("uuid:%s", info->uuid);
    IOT_INFO("clientip:%s", info->clientip);
    IOT_INFO("owner:%s", info->owner);

    return TRUE;
  }

  return FALSE;
}
