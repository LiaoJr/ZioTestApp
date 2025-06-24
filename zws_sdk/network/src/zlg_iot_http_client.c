/** 
 * 功能说明：
 *     1.HTTP 协议简单包装。
 *
 * 修改历史：
 *     1.2017-11-24 李先静<lixianjing@zlg.cn> 创建。
 *     2.2021-6-22  薛米样<xuemiyang@zlg.cn> 新增http分块传输
 */

#include <assert.h>
#include <stdlib.h>
#include "zlg_iot_log.h"
#include "zlg_iot_http_client.h"

#define STR_KEY_CONTENT_TYPE   "Content-Type"
#define STR_KEY_CONTENT_LENGTH "Content-Length"
#define STR_KEY_SET_COOKIE     "Set-Cookie"
#define STR_KEY_TRANSFER_ENCODING "Transfer-Encoding"

IoT_Error_t url_parser(const char* url, url_info *info) {
  const char *start = NULL;
  const char *end = NULL;
  const char *semicolon = NULL;
  size_t write_size = 0;

  if (NULL == url || info == NULL) {
    return NULL_VALUE_ERROR;
  }

  memset(info, 0x00, sizeof(url_info));

  start = url;
  end = strstr(start, "://");
  if(NULL == end) {
    IOT_ERROR("parse protocol type error");
    return FAILURE;
  }
  write_size =  end - start + 1;
  snprintf(info->protocol, write_size > PROTOCOL_SIZE ? PROTOCOL_SIZE : write_size, "%s", start);

  start = end + strlen("://");
  end = strstr(start, "/");
  if(NULL == end) {
    // 支持没有最后的/,如http://192.168.24.16:30880
    //IOT_ERROR("parse host error");
    //return FAILURE;
    end = url + strlen(url);
  }
  semicolon = strstr(start, ":");
  if(NULL == semicolon) {
    write_size = end - start + 1;
    snprintf(info->hostname, write_size > HOSTNAME_SIZE ? HOSTNAME_SIZE : write_size, "%s", start);
    if(0 == strcmp(info->protocol, "https")) {
      info->port = 443;
    } else {
      info->port = 80;
    }
  } else {
    int port_len = end - semicolon;
    if(port_len){
        char *port = (char*)malloc(port_len + 1);
        strncpy(port, semicolon + strlen(":"), port_len);
        port[port_len] = '\0';
        info->port = atoi(port);
        free(port);
    }
    else{
        IOT_ERROR("parse port error");
        return FAILURE;
    }
    write_size = semicolon - start + 1;
    snprintf(info->hostname, write_size > HOSTNAME_SIZE ? HOSTNAME_SIZE : write_size, "%s", start);
  }

  start = end;
  end = url + strlen(url);
  write_size = end - start + 1;
  snprintf(info->path, write_size > ZLG_IOT_HTTP_URL_PATH_LEN ? ZLG_IOT_HTTP_URL_PATH_LEN : write_size, "%s", start);

  return SUCCESS;
}

static char* get_header(char* buff, char* body, const char* key) {
    char* p = buff;

    while(p < body) {
        if(strcmp(p, key) == 0) {
            return p + strlen(p) + 2;
        }
        p += strlen(p);

        while(p < body && *p == '\0') {
            p++;
        }
    }

    return NULL;
}

static unsigned char* parse_header(zlg_iot_http_client_t* c, char* buff) {
    char* p = NULL;
    char* body = NULL;

    c->status = atoi(buff+strlen("HTTP/1.1 ")); 
    body = strstr(buff, "\r\n\r\n");
    if(body != NULL) {
        *body = '\0';
        body += 4;
    }else{
        return NULL;
    }

    for(p = buff; p != body; p++) {
        if(*p == '\r' || *p == '\n' || *p == ':') {
            *p = '\0';
        }
    }

    memset(c->content_type, 0, sizeof(c->content_type));
    p = get_header(buff, body, STR_KEY_CONTENT_TYPE);
    if(p != NULL) {
        strncpy(c->content_type, p, sizeof(c->content_type)-1);
    }
    
    c->content_length = 0;
    p = get_header(buff, body, STR_KEY_CONTENT_LENGTH);
    if(p != NULL) {
        c->content_length = atoi(p);
    }

    memset(c->set_cookie, 0, sizeof(c->set_cookie));
    p = get_header(buff, body, STR_KEY_SET_COOKIE);
    if(p != NULL) {
        strncpy(c->set_cookie, p, sizeof(c->set_cookie)-1);
    }

    memset(c->transfer_encoding, 0, sizeof(c->transfer_encoding));
    p = get_header(buff, body, STR_KEY_TRANSFER_ENCODING);
    if(p != NULL) {
      strncpy(c->transfer_encoding, p, sizeof(c->transfer_encoding)-1);
    }
    
    if(c->on_header) {
        c->on_header(c);
    }

    return (unsigned char*)body;
}

#define goto_exit_if_fail(fret) err = fret; if(err != SUCCESS) goto fexit;

static int ch16toi(char ch) {
  if (ch <= '9' && ch >= '0')
    return ch - '0';
  if (ch <= 'f' && ch >= 'a')
    return ch - 'a' + 10;
  if (ch <= 'F' && ch >= 'A')
    return ch - 'A' + 10;
  return -1;
}

static int str16toi(char *str) {
  int n = 0;
  int i = 1;
  int result = ch16toi(*str);
  while (result != -1) {
    n = n * 16 + result;
    result = ch16toi(*(str + i));
    i++;
  }
  return n;
}

typedef enum {
  HTTP_POST,
  HTTP_GET,
  HTTP_DELETE
} HTTP_METHOD;
IoT_Error_t zlg_iot_https_do(zlg_iot_http_client_t* c, unsigned char* data, size_t data_size, 
        const char* type,const HTTP_METHOD method) {
    Timer  t = {0};
    size_t ret = 0;
    IoT_Error_t err = 0;
    unsigned char* body = NULL;
    Network network;
    Network* net = &network;
    unsigned char buff[ZLG_IOT_HTTP_BUFF_SIZE+1];

    memset(buff, 0x00, sizeof(buff));
    memset(net, 0x00, sizeof(Network));

    if(method == HTTP_POST && data != NULL) {
			snprintf((char*)buff, sizeof(buff), 
        "POST %s HTTP/1.1\r\nHost: %s:%d\r\n%s: %u\r\n%s: %s\r\nCookie: %s\r\n\r\n",
        c->path, c->hostname, c->port, 
        STR_KEY_CONTENT_LENGTH, (uint32_t)data_size, 
        STR_KEY_CONTENT_TYPE, type, 
        NULL != c->cookie ? c->cookie : "");
    }else if(method == HTTP_GET){
        bool is_oss = false;
        if(strstr(c->hostname, "aliyun")){
          is_oss = true;
        }
        if(strstr(c->hostname, "myqcloud")){
          is_oss = true;
        }
        if(is_oss) {
          snprintf((char*)buff, sizeof(buff), 
            "GET %s HTTP/1.1\r\nHost: %s\r\nCookie: %s\r\n\r\n",
            c->path, c->hostname,
            NULL != c->cookie ? c->cookie : "");
          IOT_INFO("is oss");
        } else {
          snprintf((char*)buff, sizeof(buff),
            "GET %s HTTP/1.1\r\nHost: %s:%d\r\nCookie: %s\r\n\r\n",
            c->path, c->hostname, c->port,
            NULL != c->cookie ? c->cookie : "");
          IOT_INFO("not oss");
        }
    } else if(method == HTTP_DELETE){
      snprintf((char*)buff, sizeof(buff), 
        "DELETE %s HTTP/1.1\r\nHost: %s\r\nCookie: %s\r\n\r\n",
        c->path, c->hostname,
        NULL != c->cookie ? c->cookie : "");
    }

    if(0 == strncmp(c->protocol, "https", strlen("https"))) {
#ifdef WITH_MBED_TLS
      // isSSLHostnameVerify为false，不对https服务器的证书进行合法性验证；
      // 注意，即使isSSLHostnameVerify为false，只要开启WITH_MQTT_SSL了，https通信依旧是加密通信
      bool isSSLHostnameVerify = false;
      char * pRootCALocation = NULL;

      // isSSLHostnameVerify为true，对https服务器的证书进行合法性验证；
      // isSSLHostnameVerify = true;
      // 为了验证https服务器的证书，需要使用到公钥证书，pRootCALocation设置为公钥证书的路径即可
      // 下面./bin/server.pem为测试环境中公钥证书的相对路径，执行demo时当前目录必须和./bin文件夹在同一目录，如./bin/device_example devtype devid devpassword
      // 实测时可以把./bin/server.pem替换为自己的相对路径或者绝对路径
      // pRootCALocation最终通过fopen、fread读取(见mbedtls_pk_load_file函数)，实测时请确认自己的操作系统对fopen、fread函数的支持
      // pRootCALocation = "./bin/server.pem";

      goto_exit_if_fail(iot_tls_init_ex(
            net, 
            pRootCALocation,NULL,NULL,
            c->hostname, c->port, 10000,
            isSSLHostnameVerify));
#else
    IOT_ERROR("not support https, please define WITH_MBED_TLS.")
    goto fexit;
#endif
    } else {
      goto_exit_if_fail(iot_socket_init(net, c->hostname, c->port, NULL, 10000));
    }

    goto_exit_if_fail(net->connect(net, NULL));

    init_timer_delta(&t, 5, 0);
    goto_exit_if_fail(net->write(net, buff, strlen((char*)buff), &t, &ret));
    if(data != NULL) {
        init_timer_delta(&t, 5, 0);
        goto_exit_if_fail(net->write(net, data, data_size, &t, &ret));
    }

    ret = 0;
    size_t cache_size = 0;
    unsigned char cache[ZLG_IOT_HTTP_BUFF_SIZE] = {0};
    init_timer_delta(&t, 5, 0);
    //bool is_end = false;
    //size_t chunk_size = 0;
    //bool is_parser_data = false;
    do {
      memset(buff, 0x00, sizeof(buff));
      goto_exit_if_fail(net->readAny(net, buff, ZLG_IOT_HTTP_BUFF_SIZE, &ret));
      //printf("%s\n",buff);
      if (cache_size + ret <= ZLG_IOT_HTTP_BUFF_SIZE) {
        memcpy(cache + cache_size, buff, ret);
        cache_size += ret;
      }
      body = parse_header(c, (char*)cache);

    } while(!has_timer_expired(&t) && !body);

    if(body && c->on_body) {
        size_t offset = 0;
        size_t size = cache_size - (body - cache);
        long int content_length = c->content_length;
        IOT_INFO("content_length = %ld",content_length);
        /**transfer_encoding chunked**/
        size_t chunk_left = 0; // 防止没有传输完一个chunk，记录下次读取的不完整chunk大小
        const size_t chunk_size_max_len = 16;
        unsigned char chunk_size_buff[16] = {0}; // 处理chunk size不完整的情况 1 a\r\n, 需要拼接成1a\r\n，这里认为chunk size的长度最大为16
        size_t chunk_size_buff_len = 0;
        do {
            c->body = body;
            c->size = size;
            c->offset = offset;
            c->body[size] = '\0';
            c->finish = false;
            IOT_INFO("recv body = %s #####",body);
            if (strcmp(c->transfer_encoding, "chunked") == 0) {
              // 分块传输
              IOT_INFO("transfer_encoding chunked true");
              /**
               * 1a\r\n
               * chunk内容
               * \r\n
               * 1c\r\n
               * chunk内容
               * \r\n
               * 0\r\n
               * \r\n
               * */
              unsigned char *head = body;

              while (body - head < size) {
                size_t chunk_size = 0;
                if (chunk_left > 0) {
                  chunk_size = chunk_left;
                  chunk_left = 0;
                } else {
                  if (chunk_size_max_len - chunk_size_buff_len < size - (body - head)) {
                    memcpy(chunk_size_buff + chunk_size_buff_len, body, chunk_size_max_len - chunk_size_buff_len);
                  } else {
                    memcpy(chunk_size_buff + chunk_size_buff_len, body, size - (body - head)); 
                  }
                  unsigned char *chunk_size_buff_p = chunk_size_buff;
                  if (chunk_size_buff[0] == '\r' && chunk_size_buff[1] == '\n') { // 跳过chunk结尾
                    chunk_size_buff_p += 2;
                  }
                  unsigned char *p = strstr(chunk_size_buff_p, "\r\n");
                  if (p == NULL) { // 没有获取完整的chunk size
                    chunk_size_buff_len = chunk_size_buff_len + size - (body - head);
                    break;
                  }
                  size_t chunk_size_offset = p + 2 - chunk_size_buff - chunk_size_buff_len;
                  chunk_size = str16toi(chunk_size_buff_p);
                  memset(chunk_size_buff, 0x00, chunk_size_max_len);
                  chunk_size_buff_len = 0;
                  body += chunk_size_offset;
                }
                c->size = chunk_size;
                c->offset = offset;
                c->body = body;
                offset += chunk_size;
                if (body - head + chunk_size > size) {//读取的数据不够一个chunk的情况
                  chunk_left = body - head + chunk_size - size;
                  if (body - head < size) {
                    c->size = size - (body - head);
                    offset = offset - chunk_size + c->size;
                    goto_exit_if_fail(c->on_body(c));
                  }
                  break;
                }
                body = body + chunk_size; // 先移动到下一个chunk，再截取上一个chunk的字符串。
                unsigned char *p = strstr(body, "\r\n");
                if (p == NULL) { // 移动不到chunk的结尾
                  memcpy(chunk_size_buff + chunk_size_buff_len, body, size - (body - head));
                  chunk_size_buff_len = chunk_size_buff_len + size - (body - head);
                  body = head + size;
                } else {
                  body = p + 2;
                }
                c->body[chunk_size] = '\0'; // 截取上一个chunk的字符串。
                c->finish = chunk_size == 0;
                goto_exit_if_fail(c->on_body(c));
              }
              if (c->finish) { // 接收到0chunk，结束读取body
                break;
              }
            } else {
              // 非分块传输
              IOT_INFO("transfer_encoding chunked false");
              offset += size;
              content_length -= size;
              c->finish = content_length <= 0;
              goto_exit_if_fail(c->on_body(c));
              if(c->finish) {
                break;
              }
            }
            
            
            memset(buff, 0x00, sizeof(buff));
            ret = 0;
            goto_exit_if_fail(net->readAny(net, buff, ZLG_IOT_HTTP_BUFF_SIZE, &ret));
            //IOT_INFO("ret = %d",ret);

            size = ret;
            body = buff;
        }while(true); // 这里为true必须在里面跳出循环
    }
    err = SUCCESS;

fexit:
    net->disconnect(net);
    net->destroy(net);

    return err;
}

IoT_Error_t zlg_iot_https_post(zlg_iot_http_client_t* c, unsigned char* body, size_t body_size,
        const char* type) {
    return zlg_iot_https_do(c, body, body_size, type, HTTP_POST);
}

IoT_Error_t zlg_iot_https_get(zlg_iot_http_client_t* c) {
    return zlg_iot_https_do(c, NULL, 0, NULL, HTTP_GET);
}

IoT_Error_t zlg_iot_https_delete(zlg_iot_http_client_t* c) {
    return zlg_iot_https_do(c, NULL, 0, NULL, HTTP_DELETE);
}


