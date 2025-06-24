/** 
 * 功能说明：
 *     1.HTTP 获取云端固件。
 *
 * 修改历史：
 *     1.2020-03-05 余晓蓝<yuxiaolan@zlg.cn> 创建。
 */


#include "zlg_iot_http_firmware_client.h"

#include "jsmn.h"

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

IoT_Error_t url_parser(const char* url, url_info *info);
static IoT_Error_t firmware_info_on_body(zlg_iot_http_client_t* c) {
	int r = 0;
  size_t size = 0;
	char* str;
	jsmn_parser p;
	jsmntok_t t[64];
  char* JSON_STRING = (char*)(c->body);

  char *url = NULL;
  url_info* uinfo = (url_info*)c->ctx;

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, c->size, t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		IOT_ERROR("Failed to parse JSON: %d\n", r);
		return FAILURE;
	}

  if (jsoneq(JSON_STRING, &t[1], "url") == 0) {
    str = JSON_STRING + t[2].start;
    size = t[2].end-t[2].start;
    str[size] = '\0';
    url = strdup(str);
    url_parser(url, uinfo);
    free(url);
  }

  return SUCCESS;
}

IoT_Error_t zlg_iot_download_firmware(const char* url, const char* token, zlg_iot_http_client_on_body on_data, void *ctx) {
  IoT_Error_t err = 0;
  zlg_iot_http_client_t c;
  url_info uinfo;
  char cookies[1024] = {0x00};

  if (NULL == url || NULL == token || NULL == on_data) {
    return NULL_VALUE_ERROR;
  }

  memset(&c, 0x00, sizeof(c));
  if (SUCCESS != url_parser(url, &uinfo)) {
    IOT_ERROR("parse url error");
    return FAILURE;
  }
  snprintf(cookies, sizeof(cookies), "jwt=%s", token);

  printf("%s %s %s %d\n", uinfo.protocol, uinfo.hostname, uinfo.path, uinfo.port);

  c.hostname = uinfo.hostname;
  c.path = uinfo.path;
  c.port = uinfo.port;
  c.cookie = cookies;
  c.ctx = &uinfo;
  c.on_body = firmware_info_on_body;
  c.protocol = uinfo.protocol;

  err = zlg_iot_https_get(&c);
  if (err != SUCCESS) {
    return FAILURE;
  }

  if (c.status != 200) {
    IOT_ERROR("%s", c.body);
    return FAILURE;
  }

  c.hostname = uinfo.hostname;
  c.path = uinfo.path;
  c.port = uinfo.port;
  c.ctx = ctx;
  c.on_body = on_data;
  c.protocol = uinfo.protocol;

  printf("%s %s %s %d\n", uinfo.protocol, uinfo.hostname, uinfo.path, uinfo.port);

  err = zlg_iot_https_get(&c);
  if (err == SUCCESS && c.status != 200) {
    IOT_ERROR("download error = %s", c.body);
  }

  return err == SUCCESS && c.status == 200 ? SUCCESS : FAILURE;
}
