/** 
 * 功能说明：
 *     1.APP登录MQTT。只用于示例，切不可直接拷贝用于实际开发中。
 *
 * 修改历史：
 *     1.2017-11-24 李先静<lixianjing@zlg.cn> 创建。
 *     2.2019-04-24 王文坤<wangwenkun@zlg.cn> 修改。
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "client.h"
#include "device.h"

#include "zlg_iot_log.h"
#include "zlg_iot_config.h"
#include "zlg_iot_version.h"
#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_mqtt_client.h"
#include "zlg_iot_mqtt_client_interface.h"

static zlg_mqtt_info mqtt_info;

BOOL mqtt_publish(mqtt_client_t *client, int qos, int r, const char *topic, const char *buff, size_t size);

static void on_message(ZLG_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                       IoT_Publish_Message_Params *params, void *pData) {
  // 消除topicNameLen未引用的告警
  (void)(topicNameLen);

  char *payload = params->payload;
  size_t payloadlen = params->payloadLen;

  IOT_INFO("on message:");
  IOT_INFO("topic: %s", topicName);

  params_t pa;
  response_init(&pa, payload, payloadlen);
  params_dump(&pa);

  IOT_UNUSED(pData);
  IOT_UNUSED(pClient);
}

static void client_register_topics(ZLG_IoT_Client *mq, const char *owner, dev_meta_info_t meta[], int meta_size) {
  int qos = QOS1;
  char topic[256] = {0x00};

  for (int i = 0; i < meta_size; ++i) {
    snprintf(topic, sizeof(topic), STR_TOPIC_REPORT_DATA, owner, meta[i].type, meta[i].id);
    IOT_INFO("subscrite: %s", topic);
    zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);
  }
}

static void client_unregister_topics(ZLG_IoT_Client *mq) {
  if(mq != NULL){
    uint32_t i = 0;
    for(i = 0; i < ZLG_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
      if(mq->clientData.messageHandlers[i].topicName != NULL) {
        const char * topic = mq->clientData.messageHandlers[i].topicName;
        zlg_iot_mqtt_unsubscribe(mq, topic, strlen(topic));
        free((char *)topic);
      }
    }
  }
}


static void disconnectCallbackHandler(ZLG_IoT_Client *pClient, void *data) {
  IOT_WARN("MQTT Disconnect");
  IoT_Error_t rc = FAILURE;

  if (NULL == pClient) {
    return;
  }

  IOT_UNUSED(data);

  if (zlg_iot_is_autoreconnect_enabled(pClient)) {
    IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
  } else {
    IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
    rc = zlg_iot_mqtt_attempt_reconnect(pClient);
    if (NETWORK_RECONNECTED == rc) {
      IOT_WARN("Manual Reconnect Successful");
    } else {
      IOT_WARN("Manual Reconnect Failed - %d", rc);
    }
  }
}

int main(int argc, const char* argv[]) {
  char client_id[64] = {0x00};
  dev_meta_info_t meta[2];
  ZLG_IoT_Client mqclient;
  IoT_Error_t rc = FAILURE;
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  //mqtt_client_t mq;
  //mq.ctx = &mqclient;
  //mq.publish = mqtt_publish;

  IOT_INFO("Usage: %s user passwd", argv[0]);
  IOT_INFO("Readme: there must be device invert-k100 and invert-k200 under the user account");

  if (argc < 3) {
    IOT_ERROR("Invalid arguments");
    exit(1);
  }

  strncpy(meta[0].type, "invert", strlen("invert"));
  strncpy(meta[0].id, "k100", strlen("k100"));
  strncpy(meta[1].type, "invert", strlen("invert"));
  strncpy(meta[1].id, "k200", strlen("k200"));

  zlg_mqtt_info_init(&mqtt_info);

  if (!get_mqtt_info_impl(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_APP_USER_AUTH_PATH, ZLG_IOT_AUTH_PORT,
                          meta, 2, argv[1], argv[2]))
  {
    IOT_ERROR("get mqtt info error");
    exit(1);
  }

  IOT_INFO("\nZLG IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = true;
  mqttInitParams.pHostURL = mqtt_info.host;
  mqttInitParams.port = mqtt_info.port;
  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 5000;
  mqttInitParams.isSSLHostnameVerify = false;
  mqttInitParams.disconnectHandlerData = NULL;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;

  rc = zlg_iot_mqtt_init(&mqclient, &mqttInitParams);
  if (SUCCESS != rc) {
    IOT_ERROR("zlg_iot_mqtt_init returned error : %d ", rc);
    return rc;
  }

  memset(client_id, 0x00, sizeof(client_id));
  snprintf(client_id, sizeof(client_id), "appuser:%s", argv[1]);

  connectParams.isCleanSession = true;
  connectParams.isWillMsgPresent = false;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.keepAliveIntervalInSec = 60;
  connectParams.pClientID = client_id; /*注意clientId必须为devtype:devid的格式*/
  connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);
  connectParams.pUsername = strdup(argv[1]);
  connectParams.usernameLen = (uint16_t)strlen(connectParams.pUsername);
  connectParams.pPassword = mqtt_info.token;
  connectParams.passwordLen = (uint16_t)strlen(connectParams.pPassword);

  IOT_INFO("Connecting...");
  rc = zlg_iot_mqtt_connect(&mqclient, &connectParams);
  if (SUCCESS != rc) {
    IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
    return rc;
  }

  rc = zlg_iot_mqtt_autoreconnect_set_status(&mqclient, true);
  if (SUCCESS != rc) {
    IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
    return rc;
  }

  client_register_topics(&mqclient, argv[1], meta, 2);

  volatile BOOL bExit = FALSE;  // 根据实际业务设置bExit为TRUE退出while循环
  while (bExit == FALSE) {
    /*处理来自服务器的消息，以及定时发送心跳包*/
    rc = zlg_iot_mqtt_yield(&mqclient, 200);
     if (NETWORK_ATTEMPTING_RECONNECT == rc) {       // 正在重连
      IOT_ERROR("zlg_iot_mqtt_yield NETWORK_ATTEMPTING_RECONNECT");
      continue;
    }
    else if (rc < 0 ) { // 小于0是连接异常，大于等于0是连接正常
      IOT_ERROR("zlg_iot_mqtt_yield error rc:%d", rc);
      msleep(2000); // 遇到未知错误，休息一下，然后重新尝试通信
      continue;
    }

    // 添加其他自定义的业务
  }

  if (SUCCESS != rc) {
    IOT_ERROR("An error occurred in the loop:%d\n", rc);
  } else {
    IOT_INFO("Quit\n");
  }

  // 程序结束时才需要取消mqtt的订阅
  client_unregister_topics(&mqclient);
  zlg_mqtt_info_deinit(&mqtt_info);

  return rc;
}