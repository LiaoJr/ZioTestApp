#include "upload_data.h"
#include "test_order.h"
#include "database.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "client.h"
#include "device_consts.h"

#include "zlg_iot_log.h"
#include "zlg_iot_config.h"
#include "zlg_iot_version.h"
#include "zlg_iot_http_auth_client.h"
#include "zlg_iot_mqtt_client.h"
#include "zlg_iot_mqtt_client_interface.h"

#include <time.h>

static upload_info_t __g_upload_info = {0};
/*设备对象*/
static client_t a_device;
static zlg_mqtt_info mqtt_info;
static char client_id[128] = {0x00};

upload_info_t* get_upload_info_obj()
{
    return &__g_upload_info;
}
int upload_info_init(upload_info_t* upload_info, const char* meta_id, const char* meta_type, const char* meta_secret)
{
    if (!upload_info) {
        return -1;
    }
    memset(upload_info->meta.id, 0, sizeof(upload_info->meta.id));
    memset(upload_info->meta.type, 0, sizeof(upload_info->meta.type));
    memset(upload_info->meta.secret, 0, sizeof(upload_info->meta.secret));
    memcpy(upload_info->meta.id, meta_id, strlen(meta_id));
    memcpy(upload_info->meta.type, meta_type, strlen(meta_type));
    memcpy(upload_info->meta.secret, meta_secret, strlen(meta_secret));
    
    return sem_init(&upload_info->sem, 0, 0);
}

int upload_info_wait_for_data(upload_info_t* upload_info, uint64_t ms)
{
    struct timespec ts;

    if (!upload_info) {
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &ts); 
    ts.tv_sec += ms / 1000;
    return sem_timedwait(&upload_info->sem, &ts);
}

int uplaod_info_notify_new_data(upload_info_t* upload_info)
{
    if (!upload_info) {
        return -1;
    }
    return sem_post(&upload_info->sem);
}

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
BOOL mqtt_publish(mqtt_client_t *client, int qos, int r, const char *topic, const char *buff, size_t size);
static void init_devices(ZLG_IoT_Client *mqclient, mqtt_client_t *mq, const char *owner, const char* devtype, const char *devid, const char *secret) {
  params_t params;
  char buff[256] = {0};

//   mydev_init(&a_device, owner, devtype, devid, secret);
  client_init(&a_device, owner, devtype, devid, secret);

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
  //p_mqttInitParams->pHostURL = "mqtt.zlgcloud.com";

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
BOOL report_device_data(client_t* client, upload_info_t* upload_info) 
{
  params_t params;
  char buff[1024] = {0};

  if (!upload_info) {
      return FALSE;
  }

  request_init(&params, buff, sizeof(buff));
  params_append_float(&params, "current", 0.1234);

  const char* data_multi = "data";
  return client_report_data_multi(client, data_multi, &params);
}
BOOL report_test_order(client_t* client, const char* buf)
{
    params_t params;
    char info[1024] = {0};
    time_t timestamp;
    return_value_if_fail(client != NULL, FALSE);

    request_init(&params, info, sizeof(info));
    
    // 日志上报的字段和格式固定，不能修改
    timestamp = time(NULL);
    params_append_string(&params, STR_KEY_NAME, STR_SCHEMA_NAME_LOG);
    params_append_int(&params, STR_KEY_LOG_LEVEL, LOG_LEVEL_INFO);
    params_append_string(&params, STR_KEY_LOG_INFO, buf);
    params_append_uint(&params, STR_KEY_LOG_TIME, (long)(timestamp * 1000));
    // 上报不同等级的日志通过设置STR_KEY_LOG_LEVEL进行区分，client_report_error,client_report_warning接口废弃。
    return client_report_log(client, &params);
}

static const char* test_order_pack(test_order_t* test_order) 
{
    static char* buf[1024] = {0};
    char* tmp_buf = NULL;
    cJSON* root = NULL;
    
    if (!test_order) {
        return NULL;
    }
    
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "product_id", test_order->product_id);
    cJSON_AddNumberToObject(root, "serial_no", test_order->serial_no);
    cJSON_AddNumberToObject(root, "uid", test_order->uid);
    cJSON_AddNumberToObject(root, "date", test_order->date);

    cJSON *objArray = cJSON_CreateArray();
    for (uint32_t i = 0; i < test_order->test_reslut_size; i++) {
        cJSON *obj = cJSON_CreateObject();
        test_reslut_t* test_reslut = test_order->test_reslut + i;
        cJSON_AddStringToObject(obj, "type", test_reslut->type);
        cJSON_AddNumberToObject(obj, "num", (const double)test_reslut->num);
        cJSON_AddStringToObject(obj, "desc", test_reslut->desc);
        cJSON_AddItemToArray(objArray, obj);
    }
    cJSON_AddItemToObject(root, "test_reult", objArray);

    tmp_buf = cJSON_Print(root);
    memset(buf, 0, sizeof(buf));
    memcpy(buf, tmp_buf, strlen(tmp_buf));
    cJSON_Delete(root);
    return buf;
}

static int zws_upload_thread() 
{
    char buff[256];
    params_t params;
    mqtt_client_t mq;
    ZLG_IoT_Client mqclient;
    IoT_Error_t rc = FAILURE;
    // dev_meta_info_t meta;
    test_order_t* test_order = NULL;
    upload_info_t* upload_info = get_upload_info_obj();

    // memset(&meta, 0x00, sizeof(meta));
    // strncpy(meta.type, upload_info->meta.type, strlen(upload_info->meta.type));   //设备类型
    // strncpy(meta.id, upload_info->meta.id, strlen(upload_info->meta.id));     //设备ID
    // strncpy(meta.secret, upload_info->meta.secret, strlen(upload_info->meta.secret)); //设备密钥
    rc = connect_server(&mq, &mqclient, &upload_info->meta);
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

        if (report_device_data(&a_device, upload_info) == FALSE) {
            upload_info_wait_for_data(upload_info, 2000);
            continue;
        }
        
        /* 获取一条数据库中未上传的测试单 */
        if (!test_order) {
            database_get_test_order(upload_info->db, &test_order);
        }
        if (!test_order) {
            upload_info_wait_for_data(upload_info, 2000);
            continue;
        }
        const char* buf = test_order_pack(test_order);
        if (report_test_order(&a_device, buf) == FALSE) {
            upload_info_wait_for_data(upload_info, 2000);
            continue;
        }
        /* 更新上传状态 */
        database_update_order_upload_status(upload_info->db, test_order->id, UPLOADED);
        test_order_destroy(test_order);
        test_order = NULL;
        /* 等待数据库新数据插入 */
        int ret = upload_info_wait_for_data(upload_info, 2000);

    }
}
int upload_task_entry()
{
    upload_info_t* upload_info = get_upload_info_obj();
    int result = pthread_create(&upload_info->thread_id, NULL, zws_upload_thread, NULL);
    if (result != 0) {
        return -1;
    }
    pthread_join(upload_info->thread_id, NULL);
    return 0;
}