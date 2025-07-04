﻿/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_SHADOW_IOT_SHADOW_CONFIG_H_
#define SRC_SHADOW_IOT_SHADOW_CONFIG_H_

// 是否开启TLS库的编译，默认不开启
//#define WITH_MBED_TLS true

// 是否连接mqtt加密端口加密通信数据，默认不开启
//#define WITH_MQTT_SSL true

// Get from console
// =================================================
#define ZLG_IOT_AUTH_HOST              "zws.zlgcloud.com"       // 新版服务器
// #define ZLG_IOT_AUTH_HOST              "www.zlgcloud.com"     // 旧版服务器
// #define ZLG_IOT_AUTH_HOST              "192.168.24.10"        // 测试服务器

#define ZLG_IOT_TERMINAL_AUTH_PATH     "/v1/login"
#define ZLG_IOT_APP_USER_AUTH_PATH     "/v1/userLogin"

#ifdef WITH_MBED_TLS
  #define ZLG_IOT_ZWS_PORT 443  // 使用HTTPS端口
#else
  #define ZLG_IOT_ZWS_PORT 80  // 默认使用HTTP端口
#endif

#define ZLG_IOT_ZWS_HOST               "zws.zlgcloud.com" 
//#define ZLG_IOT_ZWS_HOST               "www.zlgcloud.com" 
//#define ZLG_IOT_ZWS_HOST               "192.168.24.10" 

#define ZLG_IOT_MQTT_HOST              "zws.zlgcloud.com"
//#define ZLG_IOT_MQTT_HOST              "www.zlgcloud.com" 
//#define ZLG_IOT_MQTT_HOST              "192.168.24.10"


#define ZLG_IOT_MQTT_PORT              1883 ///< default port for MQTT/S
#define ZLG_IOT_MQTT_CLIENT_ID         "zlg%s" ///< MQTT client ID should be unique for every device
#define ZLG_IOT_MY_THING_NAME 		   "AWS-IoT-C-SDK" ///< Thing Name of the Shadow this device is associated with
#define ZLG_IOT_ROOT_CA_FILENAME       "rootCA.crt" ///< Root CA file name
#define ZLG_IOT_CERTIFICATE_FILENAME   "cert.pem" ///< device signed certificate file name
#define ZLG_IOT_PRIVATE_KEY_FILENAME   "privkey.pem" ///< Device private key filename
// =================================================

// MQTT PubSub
#ifndef ZLG_IOT_MQTT_TX_BUF_LEN
#define ZLG_IOT_MQTT_TX_BUF_LEN 2048 ///< Any time a message is sent out through the MQTT layer. The message is copied into this buffer anytime a publish is done. This will also be used in the case of Thing Shadow
#endif//ZLG_IOT_MQTT_TX_BUF_LEN

#ifndef ZLG_IOT_MQTT_RX_BUF_LEN
#define ZLG_IOT_MQTT_RX_BUF_LEN 102400 ///< Any message that comes into the device should be less than this buffer size. If a received message is bigger than this buffer size the message will be dropped.
#endif//ZLG_IOT_MQTT_RX_BUF_LEN

#ifdef WITH_MBED_TLS
  //#define ZLG_IOT_AUTH_PORT 8143  // 使用HTTPS端口
  #define ZLG_IOT_AUTH_PORT 443  // 使用HTTPS端口
#else
  #define ZLG_IOT_AUTH_PORT 80  // 默认使用HTTP端口
#endif

#ifndef ZLG_IOT_HTTP_BUFF_SIZE
#define ZLG_IOT_HTTP_BUFF_SIZE 2047
#endif

#ifndef ZLG_IOT_HTTP_COOKIE_SIZE
#define ZLG_IOT_HTTP_COOKIE_SIZE 1024
#endif

#ifndef ZLG_IOT_HTTP_URL_PATH_LEN
#define ZLG_IOT_HTTP_URL_PATH_LEN 512
#endif

#ifndef ZLG_IOT_DEV_ID_LEN 
#define ZLG_IOT_DEV_ID_LEN  64
#endif

#ifndef ZLG_IOT_DEV_TYPE_LEN
#define ZLG_IOT_DEV_TYPE_LEN    64
#endif

#ifndef ZLG_IOT_DEV_SECRET_LEN
#define ZLG_IOT_DEV_SECRET_LEN  64
#endif

#ifndef ZLG_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS
#define ZLG_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 5 ///< Maximum number of topic filters the MQTT client can handle at any given time. This should be increased appropriately when using Thing Shadow
#endif//ZLG_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS

// Thing Shadow specific configs
#define SHADOW_MAX_SIZE_OF_RX_BUFFER ZLG_IOT_MQTT_RX_BUF_LEN+1 ///< Maximum size of the SHADOW buffer to store the received Shadow message
#define MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES 80  ///< Maximum size of the Unique Client Id. For More info on the Client Id refer \ref response "Acknowledgments"
#define MAX_SIZE_CLIENT_ID_WITH_SEQUENCE MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES + 10 ///< This is size of the extra sequence number that will be appended to the Unique client Id
#define MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE MAX_SIZE_CLIENT_ID_WITH_SEQUENCE + 20 ///< This is size of the the total clientToken key and value pair in the JSON
#define MAX_ACKS_TO_COMEIN_AT_ANY_GIVEN_TIME 10 ///< At Any given time we will wait for this many responses. This will correlate to the rate at which the shadow actions are requested
#define MAX_THINGNAME_HANDLED_AT_ANY_GIVEN_TIME 10 ///< We could perform shadow action on any thing Name and this is maximum Thing Names we can act on at any given time
#define MAX_JSON_TOKEN_EXPECTED 120 ///< These are the max tokens that is expected to be in the Shadow JSON document. Include the metadata that gets published
#define MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME 60 ///< All shadow actions have to be published or subscribed to a topic which is of the format $aws/things/{thingName}/shadow/update/accepted. This refers to the size of the topic without the Thing Name
#define MAX_SIZE_OF_THING_NAME 20 ///< The Thing Name should not be bigger than this value. Modify this if the Thing Name needs to be bigger
#define MAX_SHADOW_TOPIC_LENGTH_BYTES MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME + MAX_SIZE_OF_THING_NAME ///< This size includes the length of topic with Thing Name

// Auto Reconnect specific config
#define ZLG_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL 1000 ///< Minimum time before the First reconnect attempt is made as part of the exponential back-off algorithm
#define ZLG_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL 128000 ///< Maximum time interval after which exponential back-off will stop attempting to reconnect.

// #define ZLG_IOT_HTTP_BUFF_SIZE 1023 //如需获取全部安装包信息，请自行调整buff大小

#ifdef _WIN32
#define snprintf _snprintf
#define strdup _strdup
#endif

#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */
