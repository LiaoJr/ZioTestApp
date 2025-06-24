/** 
 * 功能说明：
 *     1.模拟设备。只用于示例，切不可直接拷贝用于实际开发中。
 *
 * 修改历史：
 *     1.2020-02-25 余晓蓝 <yuxiaolan@zlg.cn> 创建。
 */

#include <stdio.h>

#include "zlg_iot_log.h"
#include "zlg_iot_version.h"
#include "zlg_iot_http_package_client.h"

/*
static bool parsepkginfos(jsmntok_t *pkginfojson) {
    jsmntok_t* pobject;
    jsmntok_t* ptoken;

    if(NULL == pkginfojson) return false;

	  int len = pkginfojson->size;

    if(len > 0) {
        zlg_package_info *pkginfos = (zlg_package_info*)malloc(sizeof(zlg_package_info)*len);
        for(int i = 0; i < len; i++) {
            pobject = jsmn_get_item_from_arr(pkginfojson, i);

            ptoken = jsmn_get_prop_from_obj(pobject, "pkgtype");
            jsmn_val2string(ptoken, pkginfos[i].pkgtype, 20);

            ptoken = jsmn_get_prop_from_obj(pobject, "version");
            jsmn_val2string(ptoken, pkginfos[i].version, 20);

            ptoken = jsmn_get_prop_from_obj(pobject, "url");
            jsmn_val2string(ptoken, pkginfos[i].url, 512);

            ptoken = jsmn_get_prop_from_obj(pobject, "size");
            jsmn_val2int(ptoken, &pkginfos[i].pkgsize);
        }
    }

    return true;
}
*/

IoT_Error_t on_data(zlg_iot_http_client_t* c) {
  IOT_INFO("package content:");
  IOT_INFO("%s", c->body);

  return SUCCESS;
}

IoT_Error_t allpackages_info_on_body(zlg_iot_http_client_t* c) {

  if(c != NULL && c->ctx != NULL && c->size > 0){
    char *buff_pool = (char*)c->ctx;
    uint32_t len = strlen(buff_pool);
    memcpy(buff_pool + len, c->body, c->size);
  }

  if(c->finish  && c->ctx != NULL){  // 已获取到全部http数据
    printf("get allpackageinfo finish \n\n");

    jsmn_parser p;
    jsmntok_t* ptoken;
    jsmntok_t tokens[128];
    jsmntok_t* pobject;

    char* JSON_STRING = (char*)(c->ctx);
    //void* arg = c->ctx;

    jsmn_init_ex(&p, JSON_STRING);
    int r = jsmn_parse_ex(&p, tokens, sizeof(tokens)/sizeof(tokens[0]));
    if (r < 0) {
      printf("Failed to parse JSON: %d\n", r);
      return 1;
    }

    /*
      这里兼容下旧版ZWS和新版ZWS，旧版ZWS返回数据格式如下
      [
        {
          "result": true,
          "message": "string",
          "data": [
            {
              "pkgtype": "string",
              "version": "string",
              "suffix": "string",
              "size": 0,
              "ctime": "string",
              "desc": {}
            }
          ]
        }
      ]
      新版ZWS返回数据格式如下
      [
        {
          "pkgtype": "string",
          "version": "string",
          "suffix": "string",
          "size": 0,
          "ctime": "string",
          "desc": {}
        }
      ]
    */
    jsmntok_t *pkginfojson = jsmn_get_prop_from_obj(tokens, "data");  // 尝试按照旧版ZWS返回的格式解析
    if (pkginfojson == NULL) {
      if(tokens->type == JSMN_ARRAY){ // 尝试按照新版ZWS返回的格式解析
        pkginfojson = tokens;
      } else {
        printf("error json:%s\n", JSON_STRING);
        return 1;
      }
    }

    for(int i = 0; i < pkginfojson->size; i++) {
        char pkgtype[20];
        char version[20];
        char url[512];
        int size = 0;
        pobject = jsmn_get_item_from_arr(pkginfojson, i);

        ptoken = jsmn_get_prop_from_obj(pobject, "pkgtype");
        jsmn_val2string(ptoken, pkgtype, 20);
        printf("pkgtype: %s\n", pkgtype);

        ptoken = jsmn_get_prop_from_obj(pobject, "version");
        jsmn_val2string(ptoken, version, 20);
        printf("version: %s\n", version);

        ptoken = jsmn_get_prop_from_obj(pobject, "url");
        jsmn_val2string(ptoken, url, 512);
        printf("url: %s\n", url);

        ptoken = jsmn_get_prop_from_obj(pobject, "size");
        jsmn_val2int(ptoken, &size);
        printf("size: %d\n", size);
    }
  }
  
  return SUCCESS;
}

static IoT_Error_t package_info_on_body(zlg_iot_http_client_t* c) {

  if(c != NULL && c->ctx != NULL && c->size > 0){
    char *buff_pool = (char*)c->ctx;
    uint32_t len = strlen(buff_pool);
    memcpy(buff_pool + len, c->body, c->size);
  }

  if(c->finish  && c->ctx != NULL){  // 已获取到全部http数据
    printf("get lastpackage finish \n\n");

    size_t size = 0;
    jsmn_parser p;
    jsmntok_t t[64];
    zlg_package_info pkginfo;
    char* str;
    //void* arg = c->ctx;
    char* JSON_STRING = (char*)(c->ctx);

    //jsmn_init(&p);
    //r = jsmn_parse(&p, JSON_STRING, c->size, t, sizeof(t)/sizeof(t[0]));

    jsmn_init_ex(&p, JSON_STRING);
    int r = jsmn_parse_ex(&p, t, sizeof(t)/sizeof(t[0]));

    if (r < 0) {
      IOT_ERROR("Failed to parse JSON: %d\n", r);
      return FAILURE;
    }

	  for (int i = 1; i < r; i++) {
      if (jsoneq(JSON_STRING, &t[i], "pkgtype") == 0) {
          str = JSON_STRING + t[i+1].start;
          size = t[i+1].end - t[i+1].start;
          str[size] = '\0';
          strncpy(pkginfo.pkgtype, str, size + 1);
          i++;

          printf("pkgtype: %s\n", pkginfo.pkgtype);
      }

      if (jsoneq(JSON_STRING, &t[i], "version") == 0) {
          str = JSON_STRING + t[i+1].start;
          size = t[i+1].end - t[i+1].start;
          str[size] = '\0';
          strncpy(pkginfo.version, str, size + 1);
          i++;

          printf("version: %s\n", pkginfo.version);
      }

      if (jsoneq(JSON_STRING, &t[i], "url") == 0) {
          str = JSON_STRING + t[i+1].start;
          size = t[i+1].end - t[i+1].start;
          str[size] = '\0';
          strncpy(pkginfo.url, str, size + 1);
          i++;

          printf("url: %s\n", pkginfo.url);
      }

      if (jsoneq(JSON_STRING, &t[i], "size") == 0) {
          str = JSON_STRING + t[i+1].start;
          size = t[i+1].end - t[i+1].start;
          str[size] = '\0';
          pkginfo.pkgsize = atoi(str);
          i++;

          printf("pkgsize: %d\n", pkginfo.pkgsize);
      }
    }

    // 下载package，只是演示测试，实际场景自行决定是否下载
    zlg_iot_download_file(pkginfo.url, on_data);
  }

    return SUCCESS;
}

static zlg_cloud_info cloud_info;

// 运行示例：./bin/download_package_example web_user web_password your_pkgtype
int main(int argc, const char* argv[]) {
    IOT_INFO("\nZLG IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
    IOT_INFO("Usage: %s subusername password pkgtype", argv[0]);

    if (argc < 4) {
        IOT_ERROR("Invalid arguments");
        exit(1);
    }

    zlg_cloud_info_init(&cloud_info);

    cloud_info.username = argv[1];
    cloud_info.password = argv[2];
    const char* pkgtype = argv[3];

    memset(cloud_info.token, 0, sizeof(cloud_info.token)); 

    IoT_Error_t ret = zlg_iot_login_cloud(&cloud_info);
    if(ret != SUCCESS) {
        printf("Login zlgcloud fail for ret = %d\n", ret);
        goto finish;
    }

    // 由于http存在分块传输，把每次接收到的http body数据累计缓存到buff_pool中
    char buff_pool[ZLG_IOT_HTTP_BUFF_SIZE*10] = { 0x0 };  // 大小可以根据需求自定义，需要能缓存全部http body数据
    // 获取安装包列表
    ret = zlg_iot_get_allpackageinfo(&cloud_info, allpackages_info_on_body, buff_pool);
    if(ret != SUCCESS) {
        printf("get allpackageinfo fail for ret = %d\n", ret);
        goto finish;
    }

    // 获取最新的安装包，获取成功后package_info_on_body内部会下载安装包，只是演示测试，实际场景自行决定是否下载
    memset(buff_pool, 0, sizeof(buff_pool));
    ret = zlg_iot_get_lastpackage(pkgtype, &cloud_info, package_info_on_body, buff_pool);
    if(ret != SUCCESS) {
        printf("get lastpackage fail for ret = %d\n", ret);
        goto finish;
    }

    ret = zlg_iot_logout_cloud(&cloud_info);
    if(ret != SUCCESS) {
        printf("Logout zlgcloud fail for ret = %d\n", ret);
        goto finish;
    }

    finish:  
        zlg_cloud_info_deinit(&cloud_info);
        return ret;
}
