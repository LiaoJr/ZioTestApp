/**
* 功能说明：
*     1.HTTP 登录auth服务, 获取mqtt信息。
*
* 修改历史：
*     1.2020-03-05 余晓蓝<yuxiaolan@zlg.cn> 创建。
*/

#include <assert.h>
#include <stdlib.h>
#include "zlg_iot_http_auth_client.h"

#include "jsmn.h"

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
		strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

void zlg_mqtt_info_init(zlg_mqtt_info* info) {
	memset(info, 0x00, sizeof(*info));
}

void zlg_mqtt_info_deinit(zlg_mqtt_info* info) {
	if (NULL != info->token) {
		free(info->token);
	}
	memset(info, 0x00, sizeof(*info));
}

static size_t prepare_request(zlg_mqtt_info* info, char* buff, size_t buff_size) {
	int ret = 0;
	size_t i = 0;
	size_t size = 0;

	ret = snprintf(buff, buff_size, "{\"username\": \"%s\", \"password\": \"%s\", \"devices\": [",
		info->username, info->password);

	size += ret;
	assert(size < buff_size);
	if (size >= buff_size) {
		return buff_size;
	}

	for (i = 0; i < info->devices_nr; i++) {
		const char* devid = info->device_ids[i];
		const char* devtype = info->device_types[i];
		if (i > 0) {
			ret = snprintf(buff + size, buff_size - size, ",{\"devtype\": \"%s\", \"devid\": \"%s\"}",
				devtype, devid);
		}
		else {
			ret = snprintf(buff + size, buff_size - size, "{\"devtype\": \"%s\", \"devid\": \"%s\"}",
				devtype, devid);
		}
		size += ret;
		assert(size < buff_size);
		if (size >= buff_size) {
			break;
		}
	}

	ret = snprintf(buff + size, buff_size - size, "]}");
	size += ret;
	assert(size < buff_size);
	if (size >= buff_size) {
		return buff_size;
	}

	return size;
}

static IoT_Error_t mqtt_info_on_body(zlg_iot_http_client_t* c) {
	static int cur_size = 0;
	static char*sp = NULL;
	IoT_Error_t bRet = SUCCESS;

	do {
		if (c->content_length > c->size && sp == NULL) {
			sp = (char*)malloc(c->content_length + 1); //申请一块内存用于缓存数据
			if (sp == NULL) {//内存不够，申请失败
				IOT_ERROR("Failed to malloc memory! ");
				bRet = FAILURE;
				break;
			}
			memset(sp, 0x00, c->content_length + 1);
		}

		if (sp != NULL) {
			memcpy(sp + cur_size, c->body, c->size);
			cur_size += c->size;
			if (cur_size >= c->content_length) {
				c->body = sp;
				c->size = cur_size;
			}
			else {
				return SUCCESS;
			}
		}

		int i = 0;
		int r = 0;
		char* str;
		jsmn_parser p;
		size_t size = 0;
		jsmntok_t t[64];
		char* JSON_STRING = (char*)(c->body);
		zlg_mqtt_info* info = (zlg_mqtt_info*)c->ctx;

		jsmn_init(&p);
		r = jsmn_parse(&p, JSON_STRING, c->size, t, sizeof(t) / sizeof(t[0]));
		if (r < 0) {
			IOT_ERROR("Failed to parse JSON: %d\n", r);
			bRet = FAILURE;
			break;
		}

		for (i = 1; i < r; i++) {
			if (jsoneq(JSON_STRING, &t[i], "token") == 0) {
				str = JSON_STRING + t[i + 1].start;
				size = t[i + 1].end - t[i + 1].start;
				str[size] = '\0';
				// 释放上次info->token申请的内容 
				if(info->token != NULL){
					free(info->token);
					info->token = NULL;
				}
				// 赋值新的token
				info->token = strdup(str);

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "clientip") == 0) {
				str = JSON_STRING + t[i + 1].start;
				size = t[i + 1].end - t[i + 1].start;
				str[size] = '\0';
				strncpy(info->clientip, str, sizeof(info->clientip));

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "host") == 0) {
				str = JSON_STRING + t[i + 1].start;
				size = t[i + 1].end - t[i + 1].start;
				str[size] = '\0';
				strncpy(info->host, str, sizeof(info->host));

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "port") == 0) {
				str = JSON_STRING + t[i + 1].start;
				info->port = atoi(str);

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "sslport") == 0) {
				str = JSON_STRING + t[i + 1].start;
				info->sslport = atoi(str);

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "uuid") == 0) {
				str = JSON_STRING + t[i + 1].start;
				size = t[i + 1].end - t[i + 1].start;
				str[size] = '\0';
				strncpy(info->uuid, str, sizeof(info->uuid));

				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "owner") == 0) {
				str = JSON_STRING + t[i + 1].start;
				size = t[i + 1].end - t[i + 1].start;
				str[size] = '\0';
				strncpy(info->owner, str, sizeof(info->owner));
			}
		}
	} while (false);

	//释放内存
	if (sp != NULL) {
		free(sp);
		sp = NULL;
	}
	cur_size = 0;
	return bRet;
}

static zlg_mqtt_info_err zlg_iot_get_mqtt_info(zlg_mqtt_info* info, const char* protocol, const char* hostname, const char* path, int port) {
	IoT_Error_t err = 0;
	size_t buff_size = 0;
	zlg_iot_http_client_t c;
	unsigned char buff[ZLG_IOT_HTTP_BUFF_SIZE + 1];

	if (NULL == info || NULL == hostname || path == NULL) {
		return InvalidParams;
	}

	memset(&c, 0x00, sizeof(c));

	c.ctx = info;
	c.protocol = protocol;
	c.hostname = hostname;
	c.port = port;
	c.path = path;
	c.on_body = mqtt_info_on_body;

	buff_size = prepare_request(info, (char*)buff, ZLG_IOT_HTTP_BUFF_SIZE);
	buff[ZLG_IOT_HTTP_BUFF_SIZE] = '\0';

	err = zlg_iot_https_post(&c, buff, buff_size, "application/json");
	if (err != SUCCESS) {
		return ServerInternalError;
	}

	if (c.status == 200) {
		return SUCCESS;
	}
	else if (c.status == 400) {
		return InvalidParams;
	}
	else if (c.status == 404) {
		return DeviceNotFound;
	}
	else {
		return ServerInternalError;
	}
}

zlg_mqtt_info_err zlg_iot_http_get_mqtt_info(zlg_mqtt_info* info, const char* hostname, const char* path, int port) {
	return zlg_iot_get_mqtt_info(info, "http", hostname, path, port);
}

zlg_mqtt_info_err zlg_iot_https_get_mqtt_info(zlg_mqtt_info* info, const char* hostname, const char* path, int port) {
	return zlg_iot_get_mqtt_info(info, "https", hostname, path, port);
}




