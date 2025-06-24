/** 
 * 功能说明：
 *     1.设备与服务器通讯协议的客户端实现。
 *
 * 修改历史：
 *     1.2017-8-6 李先静<lixianjing@zlg.cn> 创建。
 */

#include "stdbool.h"
#include "command.h"
#include "settings.h"
#include "mqtt_client.h"

#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#define DEVICE_ID_LENGTH 16

#ifndef ZLG_IOT_DEV_ID_LEN
#define ZLG_IOT_DEV_ID_LEN 64
#endif

#ifndef ZLG_IOT_DEV_TYPE_LEN
#define ZLG_IOT_DEV_TYPE_LEN 64
#endif

#ifndef ZLG_IOT_DEV_SECRET_LEN
#define ZLG_IOT_DEV_SECRET_LEN 64
#endif

BEGIN_C_DECLS

/**
 * 设备认证信息
 */
typedef struct _dev_meta_info_t {
  /*设备标识符*/
  char id[ZLG_IOT_DEV_ID_LEN];
  /*设备类型*/
  char type[ZLG_IOT_DEV_TYPE_LEN];
  /*设备密钥*/
  char secret[ZLG_IOT_DEV_SECRET_LEN];
}dev_meta_info_t;

/**
 * 客户端数据结构。
 */
struct _mqtt_client_t;
typedef struct _mqtt_client_t mqtt_client_t;
typedef struct _client_t {
    /*设备所有者*/
    const char* owner;
    /*设备ID*/
    const char* id;
    /*设备类型*/
    const char* type;
    /*设备密钥*/
    const char* secret;
    /*是否是子设备，当subdev为true时，gw_owner、gw_id、gw_type才有效*/
    BOOL subdev;
    const char* gw_owner;
    const char* gw_id;
    const char* gw_type;    
    /*命令分发表。*/
    command_t* cmds;
    /*设置接口。*/
    settings_t* settings;
    /*MQTT client接口。*/
    mqtt_client_t* mqtt_client;
    /*用户参数*/
    void* p_arg;
}client_t;

/**
 * 收到的消息类型。可以是回应或命令。
 */
typedef enum _resp_packet_type_t{
    PACKET_NONE = 0,
    PACKET_RESPONSE, /*回应*/
    PACKET_COMMAND,  /*命令*/
    PACKET_NR
}resp_packet_type_t;

/*透传数据头定义*/
typedef struct _raw_header_t {
  uint32_t magic:4; /*必须为0110*/
  uint32_t version:6; /*必须为000001*/
  uint32_t save:1; /*是否存保存数据，取值为0/1（不保存/保存）*/
  uint32_t reserved:21; /*保留位*/
} raw_header_t;

typedef enum _gw_bind_unbind_subdev_error_t{
  GW_BIND_UNBIND_SUBDEV_ERR_NIL = 0,
  GW_BIND_UNBIND_SUBDEV_ERR_INVALID_PARAMS
}gw_bind_unbind_subdev_error_t;

/*消息类型(类型int)*/
#define STR_KEY_MSG_TYPE     "mtype"

/*设备ID(类型string)*/
#define STR_KEY_DEVICE_ID    "devid"

/*子设备ID(类型string)*/
#define STR_KEY_SUBDEV_ID    "sub_devid"

/*设备类型(类型string)*/
#define STR_KEY_DEVICE_TYPE  "devtype"

/*子设备类型(类型string)*/
#define STR_KEY_SUBDEV_TYPE  "sub_devtype"

/*名称(类型string)。如命令的名称。*/
#define STR_KEY_NAME         "name"

/*命令的ID(类型int)。如果命令中带有CMDID，表示该命令需要回执。*/
#define STR_KEY_CMDID        "cmdid"

/*名称(类型string)。如函数的名称。*/
#define STR_KEY_FUNCTION         "function"

/*返回消息message的格式。*/
#define STR_MSG_FORMAT         "msg_format"
#define STR_JSON                "json"
#define STR_NULL                ""

/*函数返回错误码(类型int)。*/
#define STR_KEY_ERRORCODE          "errorcode"

/*函数返回错误信息(类型string)。*/
#define STR_KEY_ERRORMSG          "errormsg"

/*时间戳(类型int)*/
#define STR_KEY_TIME         "time"

/*当前用于表示网关添加子设备的错误码(类型int)*/
#define STR_KEY_ERROR_NO     "errno"

/*最新固件版本*/
#define STR_KEY_LAST_FMVERSION  "lastfmversion"
/*申请下载最新固件*/
#define STR_KEY_LAST_FMURI      "lastfmuri"
/*最新固件信息*/
#define STR_KEY_LAST_FMDESC     "lastfmdesc"

/*版本(类型string)*/
#define STR_KEY_VERSION      "version"

/*网址(类型string)。如固件的网址。*/
#define STR_KEY_URL          "url"

/*消息(类型string)。如命令执行的详情。*/
#define STR_KEY_MSG          "msg"

/*结果(类型BOOL)。如命令执行的结果。*/
#define STR_KEY_RESULT       "result"

 /*子设备*/
#define STR_KEY_EXISTDEV   "existdev"
 /*多个子设备*/
#define STR_KEY_SUBDEVS    "subdevs"

#define STR_KEY_IDADDRMAPS    "IDAddrMaps"

/*设备上线的主题，设备 --> 服务器*/
#define STR_TOPIC_ONLINE             "/d2s/%s/%s/%s/online"

/*设备下线的主题，设备 --> 服务器*/
#define STR_TOPIC_OFFLINE            "/d2s/%s/%s/%s/offline"

/*设备请求服务器接口的主题，设备 --> 服务器*/
#define STR_TOPIC_FUNCTION            "/d2s/%s/%s/%s/function"

/*设备上报数据的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_DATA        "/d2s/%s/%s/%s/data"

/*设备上报数据的主题(多data)，设备 --> 服务器*/
#define STR_TOPIC_REPORT_DATA_MULTI  "/d2s/%s/%s/%s/data/%s"

/*设备上报透传数据的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_RAW         "/d2s/%s/%s/%s/raw"

/*设备上报错误的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_ERROR       "/d2s/%s/%s/%s/error"

/*设备上报状态的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_STATUS      "/d2s/%s/%s/%s/status"

/*设备上报警告的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_WARNING     "/d2s/%s/%s/%s/warning"

/*设备上报日志的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_LOG         "/d2s/%s/%s/%s/log"

/*设备上报日志的主题(多log)，设备 --> 服务器*/
#define STR_TOPIC_REPORT_LOG_MULTI   "/d2s/%s/%s/%s/log/%s"

/*设备上报命令执行结果的主题，设备 --> 服务器*/
#define STR_TOPIC_REPORT_RESULT      "/d2s/%s/%s/%s/result"

/*网关添加子设备，设备 --> 服务器*/
#define STR_TOPIC_BIND_SUBDEV    "/g2s/%s/%s/%s/bind"

/*网关删除子设备，设备 --> 服务器*/
#define STR_TOPIC_UNBIND_SUBDEV    "/g2s/%s/%s/%s/unbind"

/*网关转发子设备消息的主题，网关 --> 服务器*/
#define STR_TOPIC_FORWARD_MESSAGE    "/g2s/%s/%s/%s/%s"

/*internal定义了用来完成内部业务逻辑的主题*/
/*
refresh_token:
    GATEWAY/APPUSER类型的设备，当添加/删除子设备后，重新从auth获取新的token，然后发送该主题通知mqtt刷新token
    消息payload：token内容
*/
#define STR_TOPIC_REFRESH_TOKEN      "/internal/%s/%s/refresh_token"

/*服务器发送广播到所有的主题， 服务器 --> 设备*/
#define STR_TOPIC_BROADCAST          "/s2d/b/all"
#define STR_TOPIC_BROADCAST_PREFIX   "/s2d/b/"

/*服务器发送广播到指定类型(当前可以填入devtype)的设备的主题， 服务器 --> 设备*/
#define STR_TOPIC_BROADCAST_DEVTYPE  "/s2d/b/%s"

/*服务器发送广播到指定所有者的指定类型(当前可以填入devtype)的设备的主题， 服务器 --> 设备*/
#define STR_TOPIC_BROADCAST_OWNER_DEVTYPE  "/s2d/b/%s/%s"

/*服务器发送消息到指定设备的主题， 服务器 --> 设备*/
#define STR_TOPIC_DEVICE             "/s2d/d/%s/%s/%s"
#define STR_TOPIC_DEVICE_RAW         "/s2d/d/%s/%s/%s/raw"
#define STR_TOPIC_DEVICE_PREFIX      "/s2d/d"

/*服务器发送消息到指定网关的指定节点的主题， 服务器 --> 网关设备*/
#define STR_TOPIC_GATEWAY            "/s2g/d/%s/%s/%s/%s"
#define STR_TOPIC_GATEWAY_PREFIX     "/s2g/d/"



/*配置设备参数*/
#define STR_CMD_SET_CONFIG           "set_config"

/*时间同步命令*/
#define STR_CMD_SYNC_TIME            "sync_time"

/*通知设备有新固件，设备收到通知后，可提示用户有更新但不执行更新，也可后台自动更新固件*/
#define STR_CMD_NOTIFY_UPGRADE       "notify_upgrade"

/*通知设备更新固件，设备收到通知后，需立即更新固件*/
#define STR_CMD_EXEC_UPGRADE         "exec_upgrade"

/*添加设备到组播/从组播中删除设备*/
#define STR_CMD_SET_MULTICAST         "set_multicast"

/*透传命令*/
#define STR_CMD_PASS_THROUGH         "pass_through"

/*请求设备上报最新数据。*/
#define STR_CMD_REQ_REPORT           "req_report"

/*远程请求服务器接口，获取时间*/
#define STR_FUNC_GETSERVERTIME          "GetServerTime"

/*远程请求服务器接口，获取固件信息。*/
#define STR_FUNC_GETFIRMWAREINFO         "GetFirmWareInfo"

/*DTU设备获取本用户下所有devid和短地址映射*/
#define STR_FUNC_GETDTUIDADDR         "GetDTUIDAddr"

/*网关远程请求服务器接口，获取指定子设备的信息。*/
#define STR_FUNC_GETONESUBDEV       "GetOneSubdev"

/*网关远程请求服务器接口，获取所有子设备的信息。*/
#define STR_FUNC_GETALLSUBDEVS      "GetAllSubdevs"

/**获取设备影子信息*/
#define STR_FUNC_GETSHADOW          "GetShadow"

#define MAX_TOPIC_LENGTH             127
#define MAX_MESSAGE_LENGTH           256

/*组播时使用的设备类型，固定为all。*/
#define STR_MULTICAST_DEVTYPE         "all"

/**日志等级key*/
#define STR_KEY_LOG_LEVEL "level"

/**日志信息key*/
#define STR_KEY_LOG_INFO "info"

/**日志时间key*/
#define STR_KEY_LOG_TIME "time"

/**schema中日志固定的字段名字*/
#define STR_SCHEMA_NAME_LOG "loginfo"

/**日志等级：普通日志*/
#define LOG_LEVEL_INFO    0
/**日志等级：告警日志*/
#define LOG_LEVEL_WARN   1
/**日志等级：错误日志*/
#define LOG_LEVEL_ERROR  2

/**
 * \brief 初始客户端。
 * \param[in] client   : 客户端对象。
 * \param[in] type     : 设备类型
 * \param[in] id       : 设备ID 
 *
 * \retval 客户端对象。
 */
client_t* client_init(client_t* client, const char* owner, const char* type, const char* id, const char *secret);

/**
 * \brief 指定命令分发表。
 * \param[in] client : 客户端对象。
 * \param[in] cmds   : 命令分发表。
 *
 * \retval 无。
 */
void client_set_cmds(client_t* client, command_t* cmds);

/**
 * \brief 指定设置接口。
 * \param[in] client   : 客户端对象。
 * \param[in] settings : 设置接口。
 *
 * \retval 无。
 */
void client_set_settings(client_t* client, settings_t* settings);

/**
 * \brief 指定MQTT接口。
 * \param[in] client      : 客户端对象。
 * \param[in] mqtt_client : MQTT Client。
 *
 * \retval 无。
 */
void client_set_mqtt_client(client_t* client, mqtt_client_t* mqtt_client);

/**
 * \brief 分发来自MQTT的消息，一般在MQTT的消息回调函数中调用。
 * \param[in] client : 客户端对象。
 * \param[in] buff   : 数据。
 * \param[in] size   : 数据长度。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_dispatch(client_t* client, const char* buff, size_t size);

/**
 * XXX: params需要预留不小于64字节的空间，下面的函数会追加设备ID和类型到params中。
 */

/**
 * \brief 上报数据。
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_data(client_t* client, params_t* params);

/**
 * \brief (多data)上报数据。
 * \param[in] client : 客户端对象。
 * \param[in] data_multi : 多data的主题名称
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_data_multi(client_t* client, const char* data_multi, params_t* params);

/**
 * \brief 上报透传数据。
 * \param[in] client : 客户端对象。
 * \param[in] buff :  数据。
 * \param[in] size :  数据长度。
 * \param[in] save :  标志服务器是否保存该数据。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_raw(client_t* client, const unsigned char* buff, size_t size);
/**
 * \brief 上报透传数据,同时附带ZWS约定的header；设备上报的透传数据，可以带header也可以不带，根据设备的业务自行决定
 * \param[in] client : 客户端对象。
 * \param[in] buff :  数据。
 * \param[in] size :  数据长度。
 * \param[in] save :  标志服务器是否保存该数据。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_raw_with_header(client_t* client, const unsigned char* buff, size_t size, BOOL save);

/**
 * \brief 上报状态。
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_status(client_t* client, params_t* params);

/**
 * \brief 上报错误事件。废弃，统一使用client_report_log接口上报，通过等级区分
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_error(client_t* client, params_t* params);

/**
 * \brief 上报警告事件。废弃，统一使用client_report_log接口上报，通过等级区分
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_warning(client_t* client, params_t* params);

/**
 * \brief 上报日志事件。
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_log(client_t* client, params_t* params);

/**
 * \brief 上报日志事件(多log)。
 * \param[in] client : 客户端对象。
 * \param[in] log_multi : 多log的主题名称
 * \param[in] params : 参数列表。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_report_log_multi(client_t* client, const char* log_multi, params_t* params);

/**
 * \brief 设备上线。
 * \param[in] client : 客户端对象。
 * \param[in] params : 参数列表。可以包含设备本身的一些信息，如厂家和版本号。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_online(client_t* client, params_t* params);

/**
 * \brief 设备下线。
 * \param[in] client : 客户端对象。
 *
 * \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_offline(client_t* client);

/**
 * \brief 上报命令执行的结果。
 * 注意：命令参数中带有cmdid时，才要求设备执行命令后，上报执行的结果。
 * \param[in] client : 客户端对象。
 * \param[in] cmdid : 命令ID。
 * \param[in] result : 执行结果。
 * \param[in] msg : 执行结果的详情(长度< 32)。
 *
 * \retval 注册过返回TRUE，否则返回FALSE。
 */
BOOL client_reply_cmd(client_t* client, int cmdid, BOOL result, const char* msg,const char* msg_format);

/**
 *  \brief 网关添加子设备
 *  \param[in] client : 客户端对象。
 *  \param[in] subdev_type : 子设备类型。
 *  \param[in] subdev_id : 子设备id。
 *  \param[in] params : 参数列表。
 * 
 *  \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_bind_subdev(client_t* client, const char* subdev_type, const char* subdev_id, params_t* params);

/**
 *  \brief 网关删除子设备
 *  \param[in] client : 客户端对象。
 *  \param[in] subdev_type : 子设备类型。
 *  \param[in] subdev_id : 子设备id。
 *  \param[in] params : 参数列表。
 * 
 *  \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_unbind_subdev(client_t* client, const char* subdev_type, const char* subdev_id);

/**
 *  \brief 网关转发子设备消息
 *  \param[in] client : 客户端对象。
 *  \param[in] topic : 要转发到哪个主题。
 *  \param[in] params : 参数列表。
 * 
 *  \retval 成功返回TRUE，失败返回FALSE。
 */
BOOL client_forward_message(client_t* client, const char* forward_topic, params_t* params);
BOOL client_req_servfunc(client_t* client, params_t* params);

END_C_DECLS

#endif/*DEVICE_CLIENT_H*/

