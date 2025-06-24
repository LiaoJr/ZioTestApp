/** 
 * 功能说明：
 *     1.HTTP 获取云端安装包。
 *
 * 修改历史：
 *     1.2020-03-04 余晓蓝<yuxiaolan@zlg.cn> 创建。
 */

#include <assert.h>
#include <stdlib.h>
#include "zlg_iot_http_package_client.h"

#define INVALIDPARAMES 1

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

void zlg_cloud_info_init(zlg_cloud_info* info) {
    memset(info, 0x00, sizeof(*info));
}

void zlg_cloud_info_deinit(zlg_cloud_info* info) {
    memset(info, 0x00, sizeof(*info));
}

static IoT_Error_t cloud_info_on_body(zlg_iot_http_client_t* c) {
  zlg_cloud_info* info = c->ctx;

  if(NULL == c->set_cookie) {
    return FAILURE;
  }

  char* temp = c->set_cookie;
  int n = 0;

  while(temp++) {
    n++;
    if(*temp == ';') {
      break;
    } 
  }

  strncpy(info->token, c->set_cookie, n);

  return SUCCESS;
}

IoT_Error_t url_parser(const char* url, url_info *info);
IoT_Error_t zlg_iot_login_cloud(zlg_cloud_info* info) {
  IoT_Error_t err = 0;
  size_t buff_size = 0;
  zlg_iot_http_client_t c;

  if(NULL == info) {
    return INVALIDPARAMES;
  }

  unsigned char buff[ZLG_IOT_HTTP_BUFF_SIZE+1];
  const char* username = info->username;
  const char* password = info->password;

  memset(&c, 0x00, sizeof(c));

  #ifdef WITH_MBED_TLS
    c.protocol = "https";
  #else
    c.protocol = "http";
  #endif
  c.hostname = ZLG_IOT_ZWS_PACKAGE_HOST;
  c.port = ZLG_IOT_ZWS_PACKAGE_PORT;
  c.path = ZLG_IOT_LOGIN_CLOUD_PATH;
  c.on_body = cloud_info_on_body;
  c.ctx = info;

  char cookies[ZLG_IOT_HTTP_COOKIE_SIZE] = {0x00};
  snprintf(cookies, sizeof(cookies), "jwt=%s", ZLG_IOT_ZWS_JWT);
  c.cookie = cookies;

  buff_size = snprintf(buff, ZLG_IOT_HTTP_BUFF_SIZE, "{\"username\": \"%s\", \"password\": \"%s\"}", username, password);
  buff[ZLG_IOT_HTTP_BUFF_SIZE] = '\0';

  err = zlg_iot_https_post(&c, buff, buff_size, "application/json");

  return err == SUCCESS && c.status == 200 ? SUCCESS : FAILURE;
}

IoT_Error_t zlg_iot_get_allpackageinfo(zlg_cloud_info* info, zlg_iot_http_client_on_body on_data, void* arg) {
  zlg_iot_http_client_t c;

  char path[ZLG_IOT_HTTP_BUFF_SIZE+1] = {0x00};
  char cookies[1024] = {0x00};

  if(NULL == info->token) {
    return INVALIDPARAMES;
  }

  snprintf(cookies, sizeof(cookies), "%s", info->token);
  memset(&c, 0x00, sizeof(c));

  #ifdef WITH_MBED_TLS
    c.protocol = "https";
  #else
    c.protocol = "http";
  #endif
  c.hostname = ZLG_IOT_ZWS_PACKAGE_HOST;
  c.port = ZLG_IOT_ZWS_PACKAGE_PORT;
  c.cookie = cookies;
  c.ctx = arg;
  c.on_body = on_data;

  snprintf(path, ZLG_IOT_HTTP_BUFF_SIZE, ZLG_IOT_GET_ALLPACKAGE_PATH);
  c.path = path;

  zlg_iot_https_get(&c);
  if (c.status != 200) return FAILURE;

  return SUCCESS;
}

IoT_Error_t zlg_iot_get_lastpackage(const char* pkgtype, zlg_cloud_info* info, zlg_iot_http_client_on_body on_data, void*  arg) {
  zlg_iot_http_client_t c;

  char path[ZLG_IOT_HTTP_BUFF_SIZE+1] = {0x00};
  char cookies[1024] = {0x00};

  if(NULL == info->token || NULL == pkgtype) {
    return INVALIDPARAMES;
  }

  snprintf(cookies, sizeof(cookies), "%s", info->token);
  memset(&c, 0x00, sizeof(c));

  #ifdef WITH_MBED_TLS
    c.protocol = "https";
  #else
    c.protocol = "http";
  #endif
  c.hostname = ZLG_IOT_ZWS_PACKAGE_HOST;
  c.port = ZLG_IOT_ZWS_PACKAGE_PORT;
  c.cookie = cookies;
  c.ctx = arg;
  c.on_body = on_data;

  snprintf(path, ZLG_IOT_HTTP_BUFF_SIZE, ZLG_IOT_GET_LASTPACKAGE_PATH, pkgtype);
  c.path = path;

  zlg_iot_https_get(&c);
  if (c.status != 200) return FAILURE;

  return SUCCESS;
}

IoT_Error_t zlg_iot_download_file(const char* url, zlg_iot_http_client_on_body on_data) {
    zlg_iot_http_client_t c;
    //char path[ZLG_IOT_HTTP_BUFF_SIZE + 1] = {0x00};
    //char cookies[1024] = {0x00};
    url_info urlinfo;

    if(NULL == url) {
        return INVALIDPARAMES;
    }

    url_parser(url, &urlinfo);

    c.hostname = urlinfo.hostname;
    c.path = urlinfo.path;
    c.port = urlinfo.port;
    c.protocol = urlinfo.protocol;
    c.on_body = on_data;
    c.on_header = NULL;

    zlg_iot_https_get(&c);

    if (c.status == 200) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

IoT_Error_t zlg_iot_logout_cloud(zlg_cloud_info* info) {
  IoT_Error_t err = 0;
  zlg_iot_http_client_t c;
  char cookies[1024] = {0x00};

  if(NULL == info->token) {
      return INVALIDPARAMES;
  }

  snprintf(cookies, sizeof(cookies), "%s", info->token);
  memset(&c, 0x00, sizeof(c));

  #ifdef WITH_MBED_TLS
    c.protocol = "https";
  #else
    c.protocol = "http";
  #endif
  c.hostname = ZLG_IOT_ZWS_PACKAGE_HOST;
  c.port = ZLG_IOT_ZWS_PACKAGE_PORT;
  c.path = ZLG_IOT_LOGOUT_CLOUD_PATH;
  c.cookie = cookies;

  err = zlg_iot_https_get(&c);
  if (err == SUCCESS && c.status == 200) {
    return SUCCESS;
  } else {
    return FAILURE;
  }
}