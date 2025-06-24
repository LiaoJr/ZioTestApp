/** 
 * 功能说明：
 *     1.模拟网关。只用于示例，切不可直接拷贝用于实际开发中。
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
#include "mqtt_client.h"

#include "zlg_iot_log.h"
#include "zlg_iot_config.h"
#include "zlg_iot_version.h"
#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_mqtt_client.h"
#include "zlg_iot_mqtt_client_interface.h"

#ifndef _WIN32
#include <unistd.h>

#define Sleep sleep
#endif 

/*网关对象*/
static client_t a_gateway;
static client_t a_subdev;
static zlg_mqtt_info mqtt_info;

/*子设备信息*/
static const char *sub_type = "invert_test";
static const char *sub_id = "sdk_sub_id";

BOOL mqtt_publish(mqtt_client_t *client, int qos, int r, const char *topic, const char *buff, size_t size);

BOOL subdev_attach_gw(client_t* sub_dev, client_t* gw){
    sub_dev->gw_id = gw->id;
    sub_dev->gw_type = gw->type;
    sub_dev->gw_owner = gw->owner;
    
    client_set_mqtt_client(sub_dev, gw->mqtt_client);
    sub_dev->subdev = TRUE;
}

void split(char *src,const char *separator,char **dest,int *num) {
	/*
		src 源字符串的首地址(buf的地址) 
		separator 指定的分割字符
		dest 接收子字符串的数组
		num 分割后子字符串的个数
	*/
     char *pNext;
     int count = 0;

     if (src == NULL || strlen(src) == 0) 
      return;

     if (separator == NULL || strlen(separator) == 0) 
      return;

     pNext = (char *)strtok(src,separator);

     while(pNext != NULL) {
      *dest++ = pNext;
      ++count;
      pNext = (char *)strtok(NULL,separator);  
    }  
    
    *num = count;
} 	

static char* get_multicast_id(char* topic){
  char *revbuf[3] = {0};
  int num = 0;
  split(topic, "/", revbuf, &num);
  if(num == 3){
    return revbuf[2];
  }

  return NULL;
}

static void on_message(ZLG_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData) {
  char topic[256] = {0x00};
  const char *payload = params->payload;
  size_t payloadlen = params->payloadLen;

  assert(topicNameLen < sizeof(topic));
  if (topicNameLen >= sizeof(topic)) {
    IOT_WARN("topic is too long");
    return;
  }

  strncpy(topic, topicName, topicNameLen);
  IOT_INFO("on_message: %s", topic);

  if (strstr(topic, STR_TOPIC_GATEWAY_PREFIX)) {
    client_t a_device;
    
    char sub_topic[64] = {0x00};

    strncpy(sub_topic, strstr(topic, "/s2d"), 64);
    IOT_INFO("handle subdevice topics: %s", sub_topic);

    mydev_init(&a_device, a_gateway.owner, sub_type, sub_id, NULL);
    subdev_attach_gw(&a_device, &a_gateway);
    
    if (strstr(sub_topic, "raw")) {
      IOT_INFO("receive raw data: device[%s-%s], buffer[%s]\n", a_device.type, a_device.id, payload);

    } else if (strcmp(sub_topic, STR_TOPIC_BROADCAST) == 0) {
      IOT_INFO("broadcast message %s", topic);
      client_dispatch(&a_device, payload, payloadlen);

    } else if (strstr(sub_topic, STR_MULTICAST_DEVTYPE)) {
      char* castid = get_multicast_id(sub_topic);
        if(castid != NULL) IOT_INFO("muticast_devtype: all, multicast_id: %s", castid);
        //组播时，把设备ID设备为组播ID，设备类型设置为STR_MULTICAST_DEVTYPE，方便复用device的命令处理函数，具体如何处理，由开发者自己实现网关给子设备群发消息。
        a_device.id = castid;
        a_device.type = STR_MULTICAST_DEVTYPE;
        client_dispatch(&a_device, payload, payloadlen);

    } else if (strstr(sub_topic, a_device.type) && strstr(sub_topic, a_device.id)) {
      client_dispatch(&a_device, payload, payloadlen);

    } else {
      IOT_INFO("receive data: %s", payload);

    }
  } else {
    if (strstr(topic, "raw")) {
  	  IOT_INFO("receive raw data: device[%s-%s], buffer[%s]\n", a_gateway.type, a_gateway.id, payload);

    } else if (strcmp(topic, STR_TOPIC_BROADCAST) == 0) {
      IOT_INFO("broadcast message %s", topic);
      client_dispatch(&a_gateway, payload, payloadlen);

    } else if (strstr(topic, a_gateway.type) && strstr(topic, a_gateway.id)) {
      client_dispatch(&a_gateway, payload, payloadlen);

    } else {
  	  IOT_INFO("receive data: %s", payload);

    }
  }

  IOT_UNUSED(pData);
  IOT_UNUSED(pClient);
}

static void client_register_topics(client_t *c, ZLG_IoT_Client *mq) {
  int qos = QOS1;
  char topic[64] = {0x00};

  /*订阅网关相关的主题*/
  snprintf(topic, sizeof(topic), STR_TOPIC_GATEWAY, c->owner, c->type, c->id, "#");
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);

  /*订阅设备相关的主题*/
  snprintf(topic, sizeof(topic), STR_TOPIC_DEVICE, c->owner, c->type, c->id);
  IOT_INFO("subscrite: %s", topic);
  zlg_iot_mqtt_subscribe(mq, strdup(topic), strlen(topic), qos, on_message, NULL);
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

static void init_gateway(ZLG_IoT_Client *mqclient, mqtt_client_t *mq, const char *owner, const char* type, const char *devid, const char *secret) {
  params_t params;
  char buff[256] = {0};

  mygw_init(&a_gateway, owner, type, devid, secret);

  client_set_mqtt_client(&a_gateway, mq);

  client_register_topics(&a_gateway, mqclient);

  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_CLIENTIP, mqtt_info.clientip);
  client_online(&a_gateway, &params);
}

int main(int argc, const char* argv[]) {
  int first = 1;
  char buff[512];
  char client_id[64];
  dev_meta_info_t meta;
  params_t params;
  mqtt_client_t mq;
  ZLG_IoT_Client mqclient;
  IoT_Error_t rc = FAILURE;
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  mq.ctx = &mqclient;
  mq.publish = mqtt_publish;

  IOT_INFO("Usage: %s devtype devid devsecret", argv[0]);
  //IOT_INFO("Readme: The basetype must be \"zggw\", type must be \"myzggw0\"");
  IOT_INFO("Readme: The basetype must be \"gw_zigbee\", type must be \"gw_zigbee0\"");

  if (argc < 4) {
    IOT_ERROR("invalid arguments");
    exit(1);
  }

  memset(&meta, 0x00, sizeof(meta));
  strncpy(meta.type, argv[1], strlen(argv[1])); //设备类型
  strncpy(meta.id, argv[2], strlen(argv[2])); //设备ID
  strncpy(meta.secret, argv[3], strlen(argv[3])); //设备密钥

  zlg_mqtt_info_init(&mqtt_info);

  if (!get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT, &meta)) {
    IOT_ERROR("get mqtt info error");
    exit(1);
  }

  IOT_INFO("\nZLG IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  mqttInitParams.enableAutoReconnect = true;
  mqttInitParams.pHostURL = mqtt_info.host;
#ifdef WITH_MQTT_SSL
  mqttInitParams.port = mqtt_info.sslport;
  // isSSLHostnameVerify为false，不对mqtt服务器的证书进行合法性验证；
  // 注意，即使isSSLHostnameVerify为false，只要开启WITH_MQTT_SSL了，mqtt通信依旧是加密通信
  mqttInitParams.isSSLHostnameVerify = false;
  mqttInitParams.pRootCALocation = NULL;
  
  // isSSLHostnameVerify为true，对mqtt服务器的证书进行合法性验证；
  // mqttInitParams.isSSLHostnameVerify = true;
  // 为了验证mqtt服务器的证书，需要使用到公钥证书，pRootCALocation设置为公钥证书的路径即可
  // 下面./bin/cert.cer为测试环境中公钥证书的相对路径，执行demo时当前目录必须和./bin文件夹在同一目录，如./bin/device_example devtype devid devpassword
  // 实测时可以把./bin/cert.cer替换为自己的相对路径或者绝对路径
  // pRootCALocation最终通过fopen、fread读取(见mbedtls_pk_load_file函数)，实测时请确认自己的操作系统对fopen、fread函数的支持
  // 注意./bin/cert.cer有效期是截止2035-06-04，当开启isSSLHostnameVerify = true，2035-06-04之后证书过期，需要再次更新证书才行，否则验证失败
  // 也可以只开启WITH_MQTT_SSL，并设置isSSLHostnameVerify = false，此时通信依旧是加密的，只是不再验证服务器的证书，此时就没有2035-06-04证书时效的问题了
  // mqttInitParams.pRootCALocation = "./bin/cert.cer";
#else
  mqttInitParams.port = mqtt_info.port;
  mqttInitParams.isSSLHostnameVerify = false;
  mqttInitParams.pRootCALocation = NULL;
#endif
  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 50;
  mqttInitParams.disconnectHandlerData = NULL;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;
  mqttInitParams.reconnectingHandler = reconnectingCallbackHandler;
  mqttInitParams.reconnectingHandlerData = &meta;
  mqttInitParams.reconnectedHandler = reconnectedCallbackHandler;
  mqttInitParams.reconnectedHandlerData = &a_gateway;

  rc = zlg_iot_mqtt_init(&mqclient, &mqttInitParams);
  if (SUCCESS != rc) {
    IOT_ERROR("zlg_iot_mqtt_init returned error : %d ", rc);
    return rc;
  }

  memset(client_id, 0x00, sizeof(client_id));
  snprintf(client_id, sizeof(client_id), "%s:%s", meta.type, meta.id);

  connectParams.isCleanSession = true;
  connectParams.isWillMsgPresent = false;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.keepAliveIntervalInSec = 60;
  connectParams.pClientID = client_id; /*注意clientId必须为devtype:devid的格式*/
  connectParams.clientIDLen = (uint16_t)strlen(connectParams.pClientID);
  connectParams.pUsername = mqtt_info.clientip;
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

  init_gateway(&mqclient, &mq, mqtt_info.owner, argv[1], argv[2], argv[3]);

  //远程调用服务器API获取指定子设备
  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_FUNCTION, STR_FUNC_GETONESUBDEV);
  params_append_string(&params, STR_KEY_SUBDEV_TYPE, sub_type);
  params_append_string(&params, STR_KEY_SUBDEV_ID, sub_id);
  client_req_servfunc(&a_gateway, &params);
  //远程调用服务器API获取获取所有子设备
  request_init(&params, buff, sizeof(buff));
  params_append_string(&params, STR_KEY_FUNCTION, STR_FUNC_GETALLSUBDEVS);
  client_req_servfunc(&a_gateway, &params);

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

    if (first) {
      first = 0;
      /*网关添加子设备(此步骤非必选，网关也可以不添加子设备)*/
      request_init(&params, buff, sizeof(buff));
      client_bind_subdev(&a_gateway, sub_type, sub_id, &params);
    }
    
    Sleep(1);

    /*网关上报当前的时间*/
    request_init(&params, buff, sizeof(buff));
    params_append_uint(&params, STR_KEY_TIME, 1563174449);
    client_report_status(&a_gateway, &params);

    /*网关转发子设备的数据主题，其他主题可类比，这里不一一列举*/

    // 1. 初始化子设备
    mydev_init(&a_subdev, a_gateway.owner, sub_type, sub_id, NULL);
    subdev_attach_gw(&a_subdev, &a_gateway);

    // 2. 上报子设备信息
    request_init(&params, buff, sizeof(buff));
    mydev_get_all_data(&params);
    client_report_data(&a_subdev, &params);
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