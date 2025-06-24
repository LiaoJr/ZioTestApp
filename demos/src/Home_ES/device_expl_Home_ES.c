/** 
 * 功能说明：
 *     1.模拟工商储能(类型为Industry_ES)设备。只用于示例，切不可直接拷贝用于实际开发中。
 *
 * 修改历史：
 *     1.2023-11-16 创建。
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "client.h"
#include "device_Home_ES.h"

#include "zlg_iot_log.h"
#include "zlg_iot_config.h"
#include "zlg_iot_version.h"
#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_mqtt_client.h"
#include "zlg_iot_mqtt_client_interface.h"
#include "../commands.h"

/*设备对象*/
static client_t a_device;
static zlg_mqtt_info mqtt_info;
static char client_id[128] = {0x00};

BOOL mqtt_publish(mqtt_client_t *client, int qos, int r, const char *topic, const char *buff, size_t size);

static void on_message(ZLG_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                       IoT_Publish_Message_Params *params, void *pData) {
  //char topic[64] = {0x00};
  char topic[64];
  const char *payload = params->payload;
  size_t payloadlen = params->payloadLen;

  assert(topicNameLen < sizeof(topic));
  if (topicNameLen >= sizeof(topic)) {
    IOT_WARN("topic is too long");
    return;
  }

  //strncpy(topic, topicName, topicNameLen);
  memset(topic,0x00,sizeof(topic));
  memcpy(topic, topicName, topicNameLen);
  IOT_INFO("on message: %s", topic);
  
  if (strstr(topic, "raw")) {
  	IOT_INFO("receive raw data: device[%s-%s], buffer[%s]\n", a_device.type, a_device.id, payload);
  } else if (strcmp(topic, STR_TOPIC_BROADCAST) == 0) {
    IOT_INFO("broadcast message %s", topic);
    client_dispatch(&a_device, payload, payloadlen);
  } else if (strstr(topic, a_device.type) || strstr(topic, a_device.id)) {
    client_dispatch(&a_device, payload, payloadlen);
  } else {
  	IOT_INFO("receive data: %s", payload);
  }

  IOT_UNUSED(pData);
  IOT_UNUSED(pClient);
}

static void client_register_topics(client_t *c, ZLG_IoT_Client *mq) {
  int qos = QOS1;
  char topic[64] = {0x00};

  memset(topic,0,sizeof(topic));
  snprintf(topic, sizeof(topic), STR_TOPIC_DEVICE, c->owner, c->type, c->id);
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);
  
  memset(topic,0,sizeof(topic));
  snprintf(topic, sizeof(topic), STR_TOPIC_DEVICE_RAW, c->owner, c->type, c->id);
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);

  memset(topic,0,sizeof(topic));
  snprintf(topic, sizeof(topic), STR_TOPIC_BROADCAST_OWNER_DEVTYPE, c->owner, c->type);
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);
  
  memset(topic,0,sizeof(topic));
  snprintf(topic, sizeof(topic), STR_TOPIC_BROADCAST_DEVTYPE, c->type);
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);

  IOT_INFO("subscrite: %s", STR_TOPIC_BROADCAST);
  zlg_iot_mqtt_subscribe(mq, STR_TOPIC_BROADCAST, strlen(STR_TOPIC_BROADCAST), qos, on_message, NULL);
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


/*断开重连【时】重新登录认证服务器获取新的MQTT登录密码（MQTT的登录密码具有时效性）*/
static void reconnectingCallbackHandler(ZLG_IoT_Client *pClient, void *data) {
  IOT_INFO("MQTT Will Reconnecting");

  dev_meta_info_t *meta = (dev_meta_info_t *)data;

  if (NULL == pClient || data == NULL) {
    return;
  }

  get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT, meta);

  pClient->clientData.options.pPassword = mqtt_info.token;
  pClient->clientData.options.passwordLen = (uint16_t)strlen(pClient->clientData.options.pPassword);
}

/*重连【成功后】的回调函数*/
static void reconnectedCallbackHandler(ZLG_IoT_Client *pClient, void *data) {
  IOT_INFO("MQTT Reconnected");

  params_t params;
  char buff[256] = {0};
  client_t *c = (client_t *)data;

  if (NULL == pClient || data == NULL) {
    return;
  }

  //TODO
  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_CLIENTIP, mqtt_info.clientip);
  client_online(c, &params);
}

static void init_devices(ZLG_IoT_Client *mqclient, mqtt_client_t *mq, const char *owner, const char* devtype, const char *devid, const char *secret) {
  params_t params;
  char buff[256] = {0};

  Home_ES_dev_init(&a_device, owner, devtype, devid, secret);

  client_set_mqtt_client(&a_device, mq);

  client_register_topics(&a_device, mqclient);

  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_CLIENTIP, mqtt_info.clientip);
  client_online(&a_device, &params);

  //上报固件版本方式一
  params_append_string(&params, "currentfm.version", "0.2.0");
  //上报固件版本方式二
  //params_append_string(&params, "currentfm", "{\"version\": \"0.2.0\"}");
  client_report_status(&a_device, &params);
}

static IoT_Error_t connect_server(mqtt_client_t *mq, ZLG_IoT_Client *mqclient, dev_meta_info_t* meta)
{
  IoT_Client_Init_Params* p_mqttInitParams = &iotClientInitParamsDefault;
  IoT_Client_Connect_Params* p_connectParams = &iotClientConnectParamsDefault;
  IoT_Error_t rc = FAILURE;
  mq->ctx = mqclient;
  mq->publish = mqtt_publish;

  zlg_mqtt_info_init(&mqtt_info);
  if (!get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT,  meta))
  {
    IOT_ERROR("get mqtt info error");
    return FAILURE;
  }
  IOT_INFO("\nZLG IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
  p_mqttInitParams->enableAutoReconnect = true;
  p_mqttInitParams->pHostURL = mqtt_info.host;
#ifdef WITH_MQTT_SSL
  p_mqttInitParams->port = mqtt_info.sslport;
  //p_mqttInitParams->port = 4883;

  // isSSLHostnameVerify为false，不对mqtt服务器的证书进行合法性验证；
  // 注意，即使isSSLHostnameVerify为false，只要开启WITH_MQTT_SSL了，mqtt通信依旧是加密通信
  p_mqttInitParams->isSSLHostnameVerify = false;
  p_mqttInitParams->pRootCALocation = NULL;

  // isSSLHostnameVerify为true，对mqtt服务器的证书进行合法性验证；
  // p_mqttInitParams->isSSLHostnameVerify = true;
  // 为了验证mqtt服务器的证书，需要使用到公钥证书，pRootCALocation设置为公钥证书的路径即可
  // 下面./bin/cert.cer为测试环境中公钥证书的相对路径，执行demo时当前目录必须和./bin文件夹在同一目录，如./bin/device_example devtype devid devpassword
  // 实测时可以把./bin/cert.cer替换为自己的相对路径或者绝对路径
  // pRootCALocation最终通过fopen、fread读取(见mbedtls_pk_load_file函数)，实测时请确认自己的操作系统对fopen、fread函数的支持
  // 注意./bin/cert.cer有效期是截止2035-06-04，当开启isSSLHostnameVerify = true，2035-06-04之后证书过期，需要再次更新证书才行，否则验证失败
  // 也可以只开启WITH_MQTT_SSL，并设置isSSLHostnameVerify = false，此时通信依旧是加密的，只是不再验证服务器的证书，此时就没有2035-06-04证书时效的问题了
  // p_mqttInitParams->pRootCALocation = "./bin/cert.cer";
#else
  p_mqttInitParams->port = mqtt_info.port;
  p_mqttInitParams->isSSLHostnameVerify = false;
  p_mqttInitParams->pRootCALocation = NULL;
#endif

  p_mqttInitParams->mqttCommandTimeout_ms = 20000;
  p_mqttInitParams->tlsHandshakeTimeout_ms = 50;
  p_mqttInitParams->disconnectHandlerData = NULL;
  p_mqttInitParams->disconnectHandler = disconnectCallbackHandler;
  p_mqttInitParams->reconnectingHandler = reconnectingCallbackHandler;
  p_mqttInitParams->reconnectingHandlerData = meta;
  p_mqttInitParams->reconnectedHandler = reconnectedCallbackHandler;
  p_mqttInitParams->reconnectedHandlerData = &a_device;
  //mqttInitParams.interfaceName = "eth0";  //绑定指定的网卡

  rc = zlg_iot_mqtt_init(mqclient, p_mqttInitParams);
  if (SUCCESS != rc)
  {
    IOT_ERROR("zlg_iot_mqtt_init returned error : %d ", rc);
    return rc;
  }

  memset(client_id, 0x00, sizeof(client_id));
  snprintf(client_id, sizeof(client_id), "%s:%s", meta->type, meta->id);

  p_connectParams->isCleanSession = true;
  p_connectParams->isWillMsgPresent = false;
  p_connectParams->MQTTVersion = MQTT_3_1_1;
  p_connectParams->keepAliveIntervalInSec = 60;
  p_connectParams->pClientID = client_id; /*注意clientId必须为devtype:devid的格式*/
  p_connectParams->clientIDLen = (uint16_t)strlen(p_connectParams->pClientID);
  p_connectParams->pUsername = mqtt_info.clientip;
  p_connectParams->usernameLen = (uint16_t)strlen(p_connectParams->pUsername);
  p_connectParams->pPassword = mqtt_info.token;
  p_connectParams->passwordLen = (uint16_t)strlen(p_connectParams->pPassword);

  IOT_INFO("MQTT Connecting...");
  rc = zlg_iot_mqtt_connect(mqclient, p_connectParams);
  if (SUCCESS != rc)
  {
    IOT_ERROR("Error(%d) MQTT connecting to %s:%d", rc, p_mqttInitParams->pHostURL, p_mqttInitParams->port);
    return rc;
  }

  rc = zlg_iot_mqtt_autoreconnect_set_status(mqclient, true);
  if (SUCCESS != rc)
  {
    IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
    return rc;
  }
  IOT_INFO("MQTT Connect Success...");
  init_devices(mqclient, mq, mqtt_info.owner, meta->type, meta->id, meta->secret);
  return SUCCESS;
}

// 示例：
// ./bin/device_example_Home_ES Home_ES your_dev_id your_dev_password
// 参数1: 设备类型，家庭储能设备类型为Home_ES
// 参数2: 设备ID
// 参数3: 设备密钥
int main(int argc, const char* argv[]) {
  char buff[10240];
  params_t params;
  mqtt_client_t mq;
  ZLG_IoT_Client mqclient;
  IoT_Error_t rc = FAILURE;

  IOT_INFO("Usage: %s devtype devid devsecret", argv[0]);
  //IOT_INFO("Readme: The basetype must be \"invert\", type must be \"myinvert\"");

  if (argc < 4) {
    IOT_ERROR("Invalid arguments");
    exit(1);
  }
  dev_meta_info_t meta;
  memset(&meta, 0x00, sizeof(meta));
  strncpy(meta.type, argv[1], strlen(argv[1]));   //设备类型
  strncpy(meta.id, argv[2], strlen(argv[2]));     //设备ID
  strncpy(meta.secret, argv[3], strlen(argv[3])); //设备密钥

  rc = connect_server(&mq, &mqclient, &meta);

  if (rc != SUCCESS)
  {
    IOT_ERROR("Connect server ERROR!");
    return rc;
  }
  //远程调用服务器API获取时间
  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_FUNCTION, STR_FUNC_GETSERVERTIME);
  client_req_servfunc(&a_device, &params);

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

    //示例中对大部分函数的返回值都不做处理，调用者实际使用中应该要根据返回值进行相应的处理
    //比如client_report_data返回false时，表明该消息发送失败，应做相应重发处理

    // 获取模拟的户用储能的电池数据(bms)然后上报到云端，数据格式是key1\0vluae1\0key2\0vluae2\0
    request_init(&params, buff, sizeof(buff));    // 初始化buff内容为空
    Home_ES_dev_get_bms(&params);
    const char* group_bms = "bms";  // 数据分组对应Industry_ES定义中的数据分组，见云端Web平台的[设备建模]
    if (!client_report_data_multi(&a_device, group_bms, &params)) {
      // 发送失败后，这里的示例代码，简单的重发一次
      // 根据实际场景自行决定是否重发，具体重发机制由调用者自行实现：
      //    可以使用队列先保存要重发的内容，后续从队列中读取出来，再次重复
      //    也可以在mqtt_publish中简单的重发若干次
      if (!client_report_data_multi(&a_device, group_bms, &params)) {
        continue; // 可能网络存在问题，重新调用zlg_iot_mqtt_yield
      }
    }

    // 获取模拟的户用储能的逆变器-额定条件数据(rated)然后上报到云端，数据格式是key1\0vluae1\0key2\0vluae2\0
    request_init(&params, buff, sizeof(buff));    // 初始化buff内容为空
    Home_ES_dev_get_rated(&params);
    const char* group_rated = "rated";  // 数据分组对应Industry_ES定义中的数据分组，见云端Web平台的[设备建模]
    if (!client_report_data_multi(&a_device, group_rated, &params)) {
      // 发送失败后，这里的示例代码，简单的重发一次
      // 根据实际场景自行决定是否重发，具体重发机制由调用者自行实现：
      //    可以使用队列先保存要重发的内容，后续从队列中读取出来，再次重复
      //    也可以在mqtt_publish中简单的重发若干次
      if (!client_report_data_multi(&a_device, group_rated, &params)) {
        continue; // 可能网络存在问题，重新调用zlg_iot_mqtt_yield
      }
    }
    
    // 当设备状态改变时，设备主动上报设备状态给云端
    request_init(&params, buff, sizeof(buff));
    // 这里演示程序假设operating_state状态已经改变，主动上报operating_state状态
    // 0：初始化 1：停机 2：待机 3：运行 4：故障 5：重故障
    params_append_int(&params, "operating_state", 3);
    // 这里演示程序假设CSQ状态已经改变，主动上报CSQ状态
    params_append_int(&params, "CSQ", 10);
    // 这里演示程序假设caijibang_version状态已经改变，主动上报caijibang_version状态
    params_append_string(&params, "caijibang_version", "v1.1.1");
    if (!client_report_status(&a_device, &params)) {
      // 发送失败后，这里的示例代码，简单的重发一次
      if (!client_report_status(&a_device, &params)) {
        continue; // 可能网络存在问题，重新调用zlg_iot_mqtt_yield
      }
    }

    //上报日志:根据业务需求上报日志
    //myinvert_report_temperature_too_high(&a_device, 200);

    // 自定义日志组上报，其中log_multi="log1"是用户在云端Web控制台自定义的日志组名称
    //const char* log_multi = "log1";
    //myinvert_report_log_multi(&a_device, log_multi);

    msleep(2000);
  }

  if (SUCCESS != rc) {
    IOT_ERROR("An error occurred in the loop:%d", rc);
  } else {
    IOT_INFO("Quit");
  }
  client_offline(&a_device);
  // 程序结束时才需要取消mqtt的订阅
  client_unregister_topics(&mqclient);
  zlg_mqtt_info_deinit(&mqtt_info);

  return rc;
}
