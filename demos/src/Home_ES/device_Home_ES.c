
#include "device_Home_ES.h"
#include "client.h"
#include "../commands.h"
#include "settings.h"
#include <time.h>

unsigned char Industry_ES_query_strategy = 0;
char Industry_ES_peak_shaving_buff[10240] = {0};
const char* Industry_ES_demand_control = "{\"state\":1,\"limit\":\"100\"}";

BOOL Home_ES_dev_init(client_t* c, const char* owner, const char* type,  const char* id, const char *secret) { 
    client_init(c, owner, type, id, secret);
    c->subdev = FALSE;

    client_set_cmds(c, mydev_get_commands());
    client_set_settings(c, settings_get_instance());

    // 初始化随机数的种子
    srand((unsigned)time(NULL));

    // 初始化默认的削峰填谷策略
    const char * default_peak_shaving = "{\"state\":1,\"peak_thing\":{\"2023-11\":{\"21\":\"1\",\"30\":\"1\"}},\"strategy\":{\"1\":[{\"start_time\":\"00:00\",\"end_time\":\"09:00\",\"status\":\"1\",\"power\":\"2\"},{\"start_time\":\"09:00\",\"end_time\":\"11:00\",\"status\":\"2\",\"power\":\"0\"},{\"start_time\":\"11:00\",\"end_time\":\"13:00\",\"status\":\"1\",\"power\":\"3\"},{\"start_time\":\"13:00\",\"end_time\":\"18:00\",\"status\":\"3\",\"power\":\"1\"},{\"start_time\":\"18:00\",\"end_time\":\"21:00\",\"status\":\"1\",\"power\":\"2\"},{\"start_time\":\"21:00\",\"end_time\":\"23:59\",\"status\":\"1\",\"power\":\"3\"}]}}";
    memcpy(Industry_ES_peak_shaving_buff, default_peak_shaving, strlen(default_peak_shaving));
    return TRUE;
}

BOOL Home_ES_dev_get_bms(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    params_append_uint(params, STR_KEY_TIME, time(0)*1000);	
    /*TODO: 请在这里设置真实数据*/
    // 以下数据字段对应云端Home_ES类型中bms分组下面定义的字段
    params_append_float(params, "voltage", 100);
    params_append_float(params, "current", 110);
    params_append_float(params, "SOC", 120);
    params_append_float(params, "SOH", 110);
    params_append_float(params, "SOE", rand()%1000);
    params_append_float(params, "dischage", rand()%1000);
    params_append_float(params, "total_dischage", rand()%1000);
    params_append_float(params, "H_temperature", rand()%1000);
    params_append_float(params, "L_temperature", rand()%1000);
    params_append_float(params, "H_voltage", rand()%1000);
    params_append_float(params, "L_voltage", rand()%1000);
    params_append_float(params, "nominal_voltage", rand()%1000);
    params_append_float(params, "rated_current", rand()%1000);
    params_append_float(params, "rated_capacity", rand()%1000);
    params_append_float(params, "end_off_voltage", rand()%1000);
    params_append_float(params, "end_off_disvoltage", rand()%1000);
    params_append_float(params, "max_charge_c", rand()%1000);
    params_append_float(params, "max_discharge_c", rand()%1000);
    params_append_float(params, "max_charge_p", rand()%1000);
    params_append_float(params, "max_discharge_p", rand()%1000);

   return TRUE;
}

BOOL Home_ES_dev_get_rated(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    params_append_uint(params, STR_KEY_TIME, time(0)*1000);	
    /*TODO: 请在这里设置真实数据*/
    // 以下数据字段对应云端Home_ES类型中rated分组下面定义的字段
    params_append_float(params, "AC1_voltage", 100);
    params_append_float(params, "AC1_frequency", 110);
    params_append_float(params, "AC1_power", 120);
    params_append_float(params, "AC2_load_voltage", 110);
    params_append_float(params, "AC2_load_frequency", rand()%1000);
    params_append_float(params, "AC2_load_power", rand()%1000);
    params_append_float(params, "AC3_voltage", rand()%1000);
    params_append_float(params, "AC3_frequency", rand()%1000);
    params_append_float(params, "AC3_power", rand()%1000);
    params_append_float(params, "voltage_H", rand()%1000);
    params_append_float(params, "total_power", rand()%1000);
    params_append_float(params, "total_current", rand()%1000);
    params_append_float(params, "access_power", rand()%1000);
    params_append_float(params, "access_current", rand()%1000);
    params_append_float(params, "switch_fre", rand()%1000);
    params_append_float(params, "sample_fre", rand()%1000);

   return TRUE;
}
