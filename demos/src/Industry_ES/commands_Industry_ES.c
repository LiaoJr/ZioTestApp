#include "client.h"
#include "zlg_iot_http_upload_file_client.h"

#include "commands_Industry_ES.h"
#include "device_Industry_ES.h"

#define BAD_PARAMETERS "bad parameters"
#define goto_done_if_fail(p, msg) if(!(p)) {printf("%s:%d: %s\n", __func__, __LINE__, msg); message = msg; goto done;}

#define STR_NAME_MYDEV_SET "set"
#define STR_NAME_MYDEV_SET_CONFIG "set_config"
#define STR_NAME_MYDEV_SYNC_TIME "sync_time"
#define STR_NAME_MYDEV_NOTIFY_UPGRADE "notify_upgrade"
#define STR_NAME_MYDEV_EXEC_UPGRADE "exec_upgrade"
#define STR_NAME_MYDEV_REQ_REPORT "req_report"
#define STR_NAME_MYDEV_PASS_THROUGH "pass_through"
#define STR_NAME_MYDEV_SET_MULTICAST "set_multicast"
#define STR_NAME_MYDEV_RECALLFILELIST "recallFileList"
#define STR_NAME_MYDEV_RECALLFILE "recallFile"
#define STR_NAME_MYDEV_RECALLFILEDEL "recallFileDel"
#define STR_NAME_MYDEV_RECALLFILECANCEL "recallFileCancel"


static BOOL mydev_set_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	const char* name = NULL;
	const char* value = NULL;
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "name", &name), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "value", &value), BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_set_config_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_JSON);
	}
	return ret;
}

static BOOL mydev_sync_time_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	int time = 0;
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_int(params, "time", &time), BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_notify_upgrade_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	const char* version = NULL;
	const char* url = NULL;
	const char* upgradeType = NULL;
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "version", &version), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "url", &url), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "upgrade_type", &upgradeType), BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	if (strcmp(upgradeType, "notify_upgrade") == 0) {
		// notify_upgrade只是通知有更新，固件可以自行决定是否升级
		printf("receive notify_upgrade\n");
	}
	else if (strcmp(upgradeType, "exec_upgrade") == 0) {
		// exec_upgrade为执行升级，推荐固件立即执行升级
		printf("receive exec_upgrade\n");
	}
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_exec_upgrade_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	const char* version = NULL;
	const char* url = NULL;
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "version", &version), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "url", &url), BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_req_report_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static int hexstr2bytes(unsigned char* dst, const char* src, int maxsize) {
	if (dst != NULL && src != NULL) {
		int i;
		const char *psrc = src;
		char temp[5];

		for (i = 0; *psrc; i++) {
			strcpy(temp, "0X");
			strncat(temp, psrc, 2);
			if (i >= maxsize) break;
			dst[i] = strtol(temp, NULL, 16);
			psrc += 2;
		}

		return i;
	}
	else {
		return 0;
	}
}

static BOOL mydev_pass_through_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	const char* raw = NULL;
	const char* format = NULL;
	int rawlen = 0;
	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "format", &format), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "raw", &raw), BAD_PARAMETERS);
	goto_done_if_fail(params_get_int(params, "rawlen", &rawlen), BAD_PARAMETERS);
	(void)cmd;

	unsigned char *pb = NULL;

	if (strcmp(format, "hex") == 0) {
		pb = (unsigned char*)malloc(rawlen / 2);
		int len = hexstr2bytes(pb, raw, rawlen / 2);
		printf("bytes length = %d\n", len);
	}
	else if (strcmp(format, "ascii") == 0) {
		printf("ascii raw: %s\n", raw);
	}
	else if (strcmp(format, "base64") == 0) {
		printf("base64 raw: %s\n", raw);
	}

	printf("%s\n", __func__);
	params_dump(params);

	/*TODO: implement the command*/
done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}

	if (pb != NULL) free(pb);
	return ret;
}

static BOOL mydev_set_multicast_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	const char* action = NULL;
	const char* multicast_id = NULL;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "action", &action), BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "multicast_id", &multicast_id), BAD_PARAMETERS);
	printf("action: %s, multicast_id: %s\n", action, multicast_id);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);
	/*TODO: implement the command*/

done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_recallFileList_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
  const char* message = "ok";
	const char* filter = NULL;
  int page_size = 10;
  int current_page = 0;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	
	(void)cmd;
	params_dump(params);

  params_get_string(params, "filter", &filter);
	params_get_int(params, "page_size", &page_size);
  params_get_int(params, "current_page", &current_page);

done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
    // 构造文件列表，json格式，这里只是示例，实际场景请自行构造文件列表
    message = "{\"total_size\":2,\"page_size\":10,\"current_page\":1,\"fileList\":[{\"name\":\"upload_file_expl.c\",\"size\":1863,\"time\":1675647825393},{\"name\":\"device_expl.c\",\"size\":12388,\"time\":1675647825393}]}";
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static IoT_Error_t on_data(zlg_iot_http_client_t* c) {
  IOT_INFO("uploadfile content:");
  IOT_INFO("%s", c->body);

  return SUCCESS;
}


static BOOL mydev_recallFile_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = FALSE;
  const char* message = "ok";
	const char* filename = NULL;
  const char* url = NULL;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_string(params, "filename", &filename), BAD_PARAMETERS);
  goto_done_if_fail(params_get_string(params, "url", &url), BAD_PARAMETERS);

	(void)cmd;
	params_dump(params);

  IoT_Error_t rc = FAILURE;
  
  dev_meta_info_t meta;
  memset(&meta, 0x00, sizeof(meta));
  strncpy(meta.type, client->type, strlen(client->type));
  strncpy(meta.id, client->id, strlen(client->id));
  strncpy(meta.secret, client->secret, strlen(client->secret));

  zlg_mqtt_info mqtt_info;
  zlg_mqtt_info_init(&mqtt_info);

  if (!get_mqtt_info(&mqtt_info, ZLG_IOT_AUTH_HOST, ZLG_IOT_TERMINAL_AUTH_PATH, ZLG_IOT_AUTH_PORT, &meta)) {
    message = "get mqtt info error"; 
    IOT_ERROR("%s", message);
    goto done;
  }
  // file_path是要上传文件的路径，这里只是示例，从demos/src/读取文件，实际场景请自行到具体路径读取
  char file_path[256] = { 0 };
  sprintf(file_path, "%s%s", "demos/src/",filename);
  IOT_INFO("file_path:%s",file_path);

  FILE* fp = fopen(file_path, "rb");
  if (fp == NULL) {
    message = "open file error"; 
    IOT_ERROR("%s", message);
    goto done;
  }
  
  fseek(fp, 0l, SEEK_END);
  long size = ftell(fp);
  fclose(fp);

  char allurl[256] = { 0 };
  #ifdef WITH_MBED_TLS
    sprintf(allurl, "https://%s:%d%s", ZLG_IOT_ZWS_HOST,ZLG_IOT_ZWS_PORT,url);
  #else
    sprintf(allurl, "http://%s:%d%s", ZLG_IOT_ZWS_HOST,ZLG_IOT_ZWS_PORT,url);
  #endif

  // 云端下发url中的文件名称，上传文件时需要使用该名称作为上传的名称
  char* filename_url =  strrchr(url, '/');
  if (filename_url == NULL)
  {
    message = "url format error"; 
    IOT_ERROR("%s", message);
    goto done;
  } else {
    filename_url++; // 向后移一位，跳过filename_url中的/
  }
  
  IOT_INFO("all url:%s,filename_url:%s",allurl, filename_url);

  rc = zlg_iot_upload_file(allurl, mqtt_info.token, file_path, 1000, 0, filename_url, on_data, NULL);
  if (rc < size) {
    message = "upload file failure"; 
    IOT_ERROR("%s", message);
    goto done;
  }
  
  message = "ok"; 
  ret = TRUE;
done:
  zlg_mqtt_info_deinit(&mqtt_info);
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_recallFileDel_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
  const char* message = "ok";
	const char* filter = NULL;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);

	(void)cmd;
	params_dump(params);

  params_get_string(params, "filter", &filter);
done:
  // 这里只是示例，实际场景请先删除具体文件，然后返回执行结果
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_recallFileCancel_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
  const char* message = "ok";
	const char* filename = NULL;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);

	(void)cmd;
	params_dump(params);
  
  params_get_string(params, "filename", &filename);
done:
  // 这里只是示例，实际场景请先取消文件上传，然后返回执行结果
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static BOOL mydev_set_light_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
	int light_value = 0;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
	goto_done_if_fail(params_get_int(params, "light_value", &light_value), BAD_PARAMETERS);
	// 这里通过打印表示灯控制，实际情况请执行开灯关灯动作
	printf("light_value: %d\n", light_value);
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);

done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

// 云端下发(云端配置的)[削峰填谷]策略给设备
static BOOL Industry_ES_peak_shaving_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";
  const char* cfg_info = NULL;

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
  goto_done_if_fail(params_get_string(params, "cfg_info", &cfg_info), BAD_PARAMETERS);
  printf("Industry_ES_peak_shaving_exec cfg_info:%s\n",cfg_info);
  // 把云端下发的[削峰填谷]策略保存到本地(这里模拟保存在内存变量Industry_ES_peak_shaving_buff中)
  memset(Industry_ES_peak_shaving_buff, 0, sizeof(Industry_ES_peak_shaving_buff));
  memcpy(Industry_ES_peak_shaving_buff, cfg_info ,strlen(cfg_info));
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);

done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

// 云端请求设备上报设备当前[所有]策略
static BOOL Industry_ES_query_strategy_exec(command_t* cmd, struct _client_t* client, params_t* params) {
	int cmdid = 0;
	BOOL ret = TRUE;
	const char* message = "ok";

	goto_done_if_fail(cmd != NULL && client != NULL && params != NULL, BAD_PARAMETERS);
  printf("query_strategy\n");
  //printf("Industry_ES_peak_shaving_buff:%s\n", Industry_ES_peak_shaving_buff);
  Industry_ES_query_strategy = 1;
	(void)cmd;
	printf("%s\n", __func__);
	params_dump(params);

done:
	if (params_get_int(params, STR_KEY_CMDID, &cmdid)) {
		client_reply_cmd(client, cmdid, ret, message, STR_NULL);
	}
	return ret;
}

static command_t Industry_ES_cmds[] = {
	{ STR_NAME_MYDEV_SET, mydev_set_exec },
	{ STR_NAME_MYDEV_SET_CONFIG, mydev_set_config_exec },
	{ STR_NAME_MYDEV_SYNC_TIME, mydev_sync_time_exec },
	{ STR_NAME_MYDEV_NOTIFY_UPGRADE, mydev_notify_upgrade_exec },
	{ STR_NAME_MYDEV_EXEC_UPGRADE, mydev_exec_upgrade_exec },
	{ STR_NAME_MYDEV_REQ_REPORT, mydev_req_report_exec },
	{ STR_NAME_MYDEV_PASS_THROUGH, mydev_pass_through_exec },
	{ STR_NAME_MYDEV_SET_MULTICAST, mydev_set_multicast_exec },
  { STR_NAME_MYDEV_RECALLFILELIST, mydev_recallFileList_exec },
  { STR_NAME_MYDEV_RECALLFILE, mydev_recallFile_exec },
  { STR_NAME_MYDEV_RECALLFILEDEL, mydev_recallFileDel_exec },
  { STR_NAME_MYDEV_RECALLFILECANCEL, mydev_recallFileCancel_exec },
  { "set_light", mydev_set_light_exec },//set_light是演示命令，实际项目中可以删除 
  { "peak_shaving", Industry_ES_peak_shaving_exec },      // 云端下发(云端配置的)[削峰填谷]策略给设备
  { "query_strategy", Industry_ES_query_strategy_exec },  // 云端请求设备上报设备当前[所有]策略
	{ NULL, NULL }
};

command_t* Industry_ES_dev_get_commands() {
	return Industry_ES_cmds;
}
