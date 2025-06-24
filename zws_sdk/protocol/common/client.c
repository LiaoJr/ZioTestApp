/** 
 * 功能说明：
 *     1.设备的客户端
 *
 * 修改历史：
 *     1.2017-8-6 李先静<lixianjing@zlg.cn> 创建。
 */

#include "client.h"
#include "params.h"

static const char* fmt_topic(client_t* client, const char* fmt, char topic[MAX_TOPIC_LENGTH+1]) {
    if(client->subdev){
        char gw_fmt[MAX_TOPIC_LENGTH+1]={'\0'};
        snprintf(gw_fmt, MAX_TOPIC_LENGTH, STR_TOPIC_FORWARD_MESSAGE, client->gw_owner, client->gw_type, client->gw_id, fmt + 1);
        snprintf(topic, MAX_TOPIC_LENGTH, gw_fmt, client->owner, client->type, client->id);
    }
    else{
        snprintf(topic, MAX_TOPIC_LENGTH, fmt, client->owner, client->type, client->id);
    }
    topic[MAX_TOPIC_LENGTH] = '\0';
    return topic;
}

static const char* fmt_topic_multi(client_t* client, const char* fmt, char topic[MAX_TOPIC_LENGTH+1], const char* multi) {
    if(client->subdev){
        char gw_fmt[MAX_TOPIC_LENGTH+1]={'\0'};
        snprintf(gw_fmt, MAX_TOPIC_LENGTH, STR_TOPIC_FORWARD_MESSAGE, client->gw_owner, client->gw_type, client->gw_id, fmt + 1);
        snprintf(topic, MAX_TOPIC_LENGTH, gw_fmt, client->owner, client->type, client->id, multi);
    }
    else{
        snprintf(topic, MAX_TOPIC_LENGTH, fmt, client->owner, client->type, client->id, multi);
    }
    topic[MAX_TOPIC_LENGTH] = '\0';
    return topic;
}

client_t* client_init(client_t* client, const char* owner, const char* type, const char* id, const char *secret) {
    return_value_if_fail(client != NULL && id != NULL && type != NULL, NULL);
    memset(client, 0x00, sizeof(client_t));
    client->id = id;
    client->type = type;
    client->owner = owner;
    client->secret = secret;
    client->subdev = FALSE;
    
    return client;
}

void client_set_cmds(client_t* client, command_t* cmds) {
    return_if_fail(client != NULL && cmds != NULL);

    client->cmds = cmds;

    return;
}

void client_set_settings(client_t* client, settings_t* settings) {
    return_if_fail(client != NULL && settings != NULL);

    client->settings = settings;
    
    return;
}

void client_set_mqtt_client(client_t* client, mqtt_client_t* mqtt_client) {
    return_if_fail(client != NULL && mqtt_client != NULL);

    client->mqtt_client = mqtt_client;
    
    return;
}

BOOL setServerTime(client_t* client, params_t* params){ 
    int err = 0;

    if(params_get_int(params, STR_KEY_ERRORCODE, &err) && err == 0) {
        const char* curent_time = params_get(params, STR_KEY_TIME); 
        if(curent_time) {
            settings_set(client->settings, STR_KEY_TIME, curent_time);
            printf("%s: Device SET TIME: %s\n", __func__, curent_time);
        }

        return TRUE;
    } 

    return FALSE;
}

BOOL setFirmWareInfo(client_t* client, params_t* params) {
    int err = 0;

    if(params_get_int(params, STR_KEY_ERRORCODE, &err) && err == 0) {
        const char* lastfmversion = params_get(params, STR_KEY_LAST_FMVERSION); 
        if(lastfmversion) {
            settings_set(client->settings, STR_KEY_LAST_FMVERSION, lastfmversion);
            printf("%s: Device SET Last Firmware Version: %s\n", __func__, lastfmversion);
        }

        const char* lastfmuri = params_get(params, STR_KEY_LAST_FMURI); 
        if(lastfmuri) {
            settings_set(client->settings, STR_KEY_LAST_FMURI, lastfmuri);
        }

        const char* lastfmdesc = params_get(params, STR_KEY_LAST_FMDESC); 
        if(lastfmdesc) {
            settings_set(client->settings, STR_KEY_LAST_FMDESC, lastfmdesc);
        }
    } else {
        const char* errmsg = params_get(params, STR_KEY_ERRORMSG); 
        printf("%s: errmsg: %s\n", __func__, errmsg);

        return FALSE;
    }

    return TRUE;
}

BOOL GetOneSubdev(client_t* client, params_t* params) {
    // 消除client未引用的告警：warning: unused parameter ‘client’
    (void)(client);
    
    int err = 0;
    int exist = 0;

    if(params_get_int(params, STR_KEY_ERRORCODE, &err) && err == 0) {   
        params_get_int(params, STR_KEY_EXISTDEV, &exist);  
        if(exist == 1){
            printf("%s: subdev exist on zlgcloud: %d\n", __func__, exist);
            return TRUE;
        } else {
            return FALSE;
        }

    } else {
        const char* errmsg = params_get(params, STR_KEY_ERRORMSG); 
        printf("%s: errmsg: %s\n", __func__, errmsg);

        return FALSE;
    }
}

BOOL GetAllSubdevs(client_t* client, params_t* params) {
    // 消除client未引用的告警：warning: unused parameter ‘client’
    (void)(client);

    int err = 0;
    if(params_get_int(params, STR_KEY_ERRORCODE, &err) && err == 0) {
        const char* subdevs = params_get(params, STR_KEY_SUBDEVS); 
        if(subdevs != NULL) {
            printf("%s: subdevs: %s\n", __func__, subdevs);
        }
    } else {
        const char* errmsg = params_get(params, STR_KEY_ERRORMSG); 
        printf("%s: errmsg: %s\n", __func__, errmsg);

        return FALSE;
    }

    return TRUE;
}

BOOL GetDTUIDAddrMaps(client_t* client, params_t* params) {
    // 消除client未引用的告警：warning: unused parameter ‘client’
    (void)(client);

    int err = 0;
    if(params_get_int(params, STR_KEY_ERRORCODE, &err) && err == 0) {
        const char* maps = params_get(params, STR_KEY_IDADDRMAPS); 
        if(maps != NULL) {
            printf("%s: maps: %s\n", __func__, maps);
        }
    } else {
        const char* errmsg = params_get(params, STR_KEY_ERRORMSG); 
        printf("%s: errmsg: %s\n", __func__, errmsg);

        return FALSE;
    }

    return TRUE;
}

static BOOL client_on_response(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);
     const char* functionname = params_get(params, STR_KEY_FUNCTION);
    return_value_if_fail(functionname != NULL, FALSE);

    if(strcmp(functionname, STR_FUNC_GETSERVERTIME) == 0) {
        setServerTime(client, params);
    } else if(strcmp(functionname, STR_FUNC_GETFIRMWAREINFO) == 0) {
        setFirmWareInfo(client, params);
    } else if(strcmp(functionname, STR_FUNC_GETONESUBDEV) == 0) {
        GetOneSubdev(client, params);
    } else if(strcmp(functionname, STR_FUNC_GETALLSUBDEVS) == 0) {
        GetAllSubdevs(client, params);
    } else if(strcmp(functionname, STR_FUNC_GETDTUIDADDR) == 0) {
        GetDTUIDAddrMaps(client, params);
    } else {
        const char* errmsg = params_get(params, STR_KEY_ERRORMSG); 
        printf("%s: Error Massage: %s\n", __func__, errmsg);
    }

    return TRUE;
}

static BOOL client_on_command(client_t* client, params_t* params) {
    int32_t i = 0;
    const char* name = NULL;

    return_value_if_fail(client != NULL && params != NULL, FALSE);
    name = params_get(params, STR_KEY_NAME);
    return_value_if_fail(name != NULL, FALSE);
    
    for(i = 0; client->cmds[i].exec; i++) {
        command_t* iter = client->cmds+i;
        if(strcmp(iter->name, name) == 0) {
            return iter->exec(iter, client, params);
        }
    }

    printf("%s: not found %s\n", __func__, name);

    return FALSE;
}

BOOL client_dispatch(client_t* client, const char* buff, size_t size) {
    params_t params;
    BOOL ret = FALSE;
    int32_t type = PACKET_NONE;

    response_init(&params, (char*)buff, size);
    return_value_if_fail(client != NULL && buff != NULL && size > 1, ret);
    return_value_if_fail(params_get_int(&params, STR_KEY_MSG_TYPE, &type), FALSE);

    params_dump(&params);
    if(type == PACKET_RESPONSE) {
        ret = client_on_response(client, &params);    
    }else if(type == PACKET_COMMAND) {
        ret = client_on_command(client, &params);    
    }

    return ret;
}

static BOOL client_send(client_t* client, const char* topic_fmt, params_t* params, int qos) {
    int retain = 0;
    char topic[MAX_TOPIC_LENGTH+1]={'\0'};

    fmt_topic(client, topic_fmt, topic);
    return mqtt_client_publish(client->mqtt_client, qos, retain, topic, params->buff, params->size);
}

BOOL client_report_data(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_REPORT_DATA, params, 1);
}

BOOL client_report_data_multi(client_t* client, const char* data_multi, params_t* params){
  return_value_if_fail(client != NULL && params != NULL, FALSE);

  int retain = 0;
  char topic[MAX_TOPIC_LENGTH+1]={'\0'};

  fmt_topic_multi(client, STR_TOPIC_REPORT_DATA_MULTI, topic, data_multi);
  return mqtt_client_publish(client->mqtt_client, 1, retain, topic, params->buff, params->size);
}

BOOL client_req_servfunc(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_FUNCTION, params, 1);
}

BOOL client_report_raw(client_t* client, const unsigned char* buff, size_t size) {
    BOOL ret = FALSE;
    params_t params;
    char* temp_buff = NULL;
    return_value_if_fail(client != NULL && buff != NULL, FALSE);

    temp_buff = (char *)calloc(size, sizeof(char));
    return_value_if_fail(temp_buff != NULL, FALSE);

    memcpy(temp_buff, buff, size);

    params.buff = temp_buff;
    params.size = size;

    ret = client_send(client, STR_TOPIC_REPORT_RAW, &params, 1);
    free(temp_buff);

    return ret;
}

BOOL client_report_raw_with_header(client_t* client, const unsigned char* buff, size_t size, BOOL save) {
    BOOL ret = FALSE;
    params_t params;
    raw_header_t header;
    char* temp_buff = NULL;
    return_value_if_fail(client != NULL && buff != NULL, FALSE);

    temp_buff = (char *)calloc(sizeof(raw_header_t) + size, sizeof(char));
    return_value_if_fail(temp_buff != NULL, FALSE);

    header.magic = 6;
    header.version = 1;
    header.save = save ? 1 : 0;
    memcpy(temp_buff, &header, sizeof(raw_header_t));
    memcpy(temp_buff + sizeof(raw_header_t), buff, size);

    params.buff = temp_buff;
    params.size = sizeof(raw_header_t) + size;

    ret = client_send(client, STR_TOPIC_REPORT_RAW, &params, 1);
    free(temp_buff);

    return ret;
}

BOOL client_report_status(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_REPORT_STATUS, params, 1); 
}

BOOL client_report_error(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_REPORT_ERROR, params, 1); 
}

BOOL client_report_warning(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_REPORT_WARNING, params, 1);    
}

BOOL client_report_log(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL && params != NULL, FALSE);

    return client_send(client, STR_TOPIC_REPORT_LOG, params, 1);    
}

BOOL client_report_log_multi(client_t* client, const char* log_multi, params_t* params){
  return_value_if_fail(client != NULL && params != NULL, FALSE);

  int retain = 0;
  char topic[MAX_TOPIC_LENGTH+1]={'\0'};

  fmt_topic_multi(client, STR_TOPIC_REPORT_LOG_MULTI, topic, log_multi);
  return mqtt_client_publish(client->mqtt_client, 1, retain, topic, params->buff, params->size);
}

BOOL client_online(client_t* client, params_t* params) {
    return_value_if_fail(client != NULL, FALSE);
    return_value_if_fail(params != NULL, FALSE);

    return client_send(client, STR_TOPIC_ONLINE, params, 1); 
}

BOOL client_reply_cmd(client_t* client, int cmdid, BOOL result, const char* msg,const char* msg_format) {
    params_t params;
    char buff[MAX_MESSAGE_LENGTH];
    return_value_if_fail(client != NULL, FALSE);
    request_init(&params, buff, sizeof(buff));
    return_value_if_fail(params_append_int(&params, STR_KEY_CMDID, cmdid), FALSE);
    return_value_if_fail(params_append_int(&params, STR_KEY_RESULT, result), FALSE);
    return_value_if_fail(params_append_string(&params, STR_KEY_MSG, msg), FALSE);
    if(msg_format && (strcmp(msg_format, STR_NULL) != 0)){
        return_value_if_fail(params_append_string(&params, STR_MSG_FORMAT, msg_format), FALSE);
    }

    return client_send(client, STR_TOPIC_REPORT_RESULT, &params, 1); 
}

BOOL client_offline(client_t* client) {
    params_t params;
    char buff[MAX_MESSAGE_LENGTH];
    return_value_if_fail(client != NULL, FALSE);
    request_init(&params, buff, sizeof(buff));

    return client_send(client, STR_TOPIC_OFFLINE, &params, 1); 
}

BOOL client_bind_subdev(client_t* client, const char* subdev_type, const char* subdev_id, params_t* params) {
    return_value_if_fail(client != NULL, FALSE);
    return_value_if_fail(subdev_type != NULL, FALSE);
    return_value_if_fail(subdev_id != NULL, FALSE);
    return_value_if_fail(params != NULL, FALSE);

    return_value_if_fail(params_append_string(params, STR_KEY_SUBDEV_ID, subdev_id), FALSE);
    return_value_if_fail(params_append_string(params, STR_KEY_SUBDEV_TYPE, subdev_type), FALSE);

    return client_send(client, STR_TOPIC_BIND_SUBDEV, params, 1);
}

BOOL client_unbind_subdev(client_t* client, const char* subdev_type, const char* subdev_id) {
    params_t params;
    char buff[MAX_MESSAGE_LENGTH];
    return_value_if_fail(client != NULL, FALSE);
    return_value_if_fail(subdev_type != NULL, FALSE);
    return_value_if_fail(subdev_id != NULL, FALSE);

    request_init(&params, buff, sizeof(buff));
    return_value_if_fail(params_append_string(&params, STR_KEY_SUBDEV_ID, subdev_id), FALSE);
    return_value_if_fail(params_append_string(&params, STR_KEY_SUBDEV_TYPE, subdev_type), FALSE);


    return client_send(client, STR_TOPIC_UNBIND_SUBDEV, &params, 1);
}

BOOL client_forward_message(client_t* client, const char* forward_topic, params_t* params) {
    char topic[256] = {0};
    return_value_if_fail(client != NULL, FALSE);
    return_value_if_fail(forward_topic != NULL, FALSE);
    return_value_if_fail(params != NULL, FALSE);

    snprintf(topic, 256, STR_TOPIC_FORWARD_MESSAGE, client->owner, client->type, client->id, forward_topic + 1);

    return mqtt_client_publish(client->mqtt_client, 1, 0, topic, params->buff, params->size);
}
