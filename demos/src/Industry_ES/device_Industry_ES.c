
#include "device_Industry_ES.h"
#include "client.h"
#include "commands_Industry_ES.h"
#include "settings.h"
#include <time.h>

unsigned char Industry_ES_query_strategy = 0;
char Industry_ES_peak_shaving_buff[10240] = {0};
const char* Industry_ES_demand_control = "{\"state\":1,\"limit\":\"100\"}";

BOOL Industry_ES_dev_init(client_t* c, const char* owner, const char* type,  const char* id, const char *secret) { 
    client_init(c, owner, type, id, secret);
    c->subdev = FALSE;

    client_set_cmds(c, Industry_ES_dev_get_commands());
    client_set_settings(c, settings_get_instance());

    // 初始化随机数的种子
    srand((unsigned)time(NULL));

    // 初始化默认的削峰填谷策略
    const char * default_peak_shaving = "{\"state\":1,\"peak_thing\":{\"2023-11\":{\"21\":\"1\",\"30\":\"1\"}},\"strategy\":{\"1\":[{\"start_time\":\"00:00\",\"end_time\":\"09:00\",\"status\":\"1\",\"power\":\"2\"},{\"start_time\":\"09:00\",\"end_time\":\"11:00\",\"status\":\"2\",\"power\":\"0\"},{\"start_time\":\"11:00\",\"end_time\":\"13:00\",\"status\":\"1\",\"power\":\"3\"},{\"start_time\":\"13:00\",\"end_time\":\"18:00\",\"status\":\"3\",\"power\":\"1\"},{\"start_time\":\"18:00\",\"end_time\":\"21:00\",\"status\":\"1\",\"power\":\"2\"},{\"start_time\":\"21:00\",\"end_time\":\"23:59\",\"status\":\"1\",\"power\":\"3\"}]}}";
    memcpy(Industry_ES_peak_shaving_buff, default_peak_shaving, strlen(default_peak_shaving));
    return TRUE;
}

BOOL Industry_ES_dev_get_statistics_data(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    params_append_uint(params, STR_KEY_TIME, time(0)*1000);	
    /*TODO: 请在这里设置真实数据*/
    // 以下数据字段对应云端Industry_ES类型中statistics_data分组下面定义的字段
    params_append_float(params, "Edischg_day", 100);
    params_append_float(params, "Edischg_total", 110);
    params_append_float(params, "Echg_day", 120);
    params_append_float(params, "Echg_total", 110);
    params_append_float(params, "active_power", rand()%1000);
    params_append_float(params, "reactive_power", rand()%1000);
    params_append_float(params, "grid_power", rand()%1000);
    params_append_float(params, "grid_Rpower", rand()%1000);
    params_append_float(params, "total_power", rand()%1000);
    params_append_float(params, "grid_energy", rand()%1000);
    params_append_float(params, "load_Rpower", rand()%1000);
    params_append_float(params, "load_power", rand()%1000);
    params_append_float(params, "dump_energy", rand()%1000);
    params_append_float(params, "feed_energy", rand()%1000);
    params_append_float(params, "photovoltaic_energy", rand()%1000);
    params_append_float(params, "photovoltaic_power", rand()%1000);

   return TRUE;
}

BOOL Industry_ES_dev_get_statistics_bms(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    params_append_uint(params, STR_KEY_TIME, time(0)*1000);	
    /*TODO: 请在这里设置真实数据*/
    // 以下数据字段对应云端Industry_ES类型中statistics_bms分组下面定义的字段
    params_append_float(params, "SOC", 100);
    params_append_float(params, "SOH", 110);
    params_append_float(params, "voltage", 120);
    params_append_float(params, "current", 110);
    params_append_float(params, "dischage", rand()%1000);
    params_append_float(params, "total_dischage", rand()%1000);
    params_append_float(params, "chage", rand()%1000);
    params_append_float(params, "total_chage", rand()%1000);
    params_append_float(params, "aver_temperature", rand()%1000);
    params_append_float(params, "dump_energy", rand()%1000);
    params_append_float(params, "SOE", rand()%1000);
    params_append_float(params, "rated_chage", rand()%1000);

   return TRUE;
}

BOOL Industry_ES_dev_get_pcs1(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    params_append_uint(params, STR_KEY_TIME, time(0)*1000);	
    /*TODO: 请在这里设置真实数据*/
    // 以下数据字段对应云端Industry_ES类型中statistics_data分组下面定义的字段

    params_append_float(params, "active_power", rand()%100);
    params_append_float(params, "reactive_power", rand()%100);
    params_append_float(params, "apparent_power", rand()%100);
    params_append_float(params, "power_factor", rand()%100);
    params_append_float(params, "module_temperature", rand()%50);
    params_append_float(params, "pcs_temperature", rand()%50);
    params_append_float(params, "voltage_A", rand()%220);
    params_append_float(params, "voltage_B", rand()%220);
    params_append_float(params, "voltage_C", rand()%1000);
    params_append_float(params, "current_A", rand()%10);
    params_append_float(params, "current_B", rand()%10);
    params_append_float(params, "current_C", rand()%10);

   return TRUE;
}