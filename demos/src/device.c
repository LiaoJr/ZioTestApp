
#include "device.h"
#include "client.h"
#include "commands.h"
#include "settings.h"


BOOL mydev_init(client_t* c, const char* owner, const char* type,  const char* id, const char *secret) { 
    client_init(c, owner, type, id, secret);
    c->subdev = FALSE;

    client_set_cmds(c, mydev_get_commands());
    client_set_settings(c, settings_get_instance());

    return TRUE;
}


BOOL mygw_init(client_t* c, const char* owner, const char* type, const char* id, const char *secret) { 

    client_init(c, owner, type, id, secret);
    c->subdev = FALSE;
    
    client_set_cmds(c, myzggw_get_commands());
    client_set_settings(c, settings_get_instance());

    return TRUE;
}

BOOL mydev_get_all_data(params_t* params) {
    params->size = 1; /*保留版本号*/

    // 每条记录的产生时间，云端要求时间精度为ms；
    // 可省略该字段，此时以云端收到设备数据的时间为准
    uint64_t now = time(0)*1000L; // 转化为ms，长度类型必须是64bit
    params_append_uint(params, STR_KEY_TIME, now);	
    /*TODO: 请在这里设置真实数据*/
    params_append_float(params, "total_energy", 100);
    params_append_float(params, "today_energy", 120);
    params_append_float(params, "current", 11.11111);
    /*params_append_float(params, "temperature", 0);
    params_append_float(params, "gfci", 0);
    params_append_float(params, "bus_volt", 0);
    params_append_float(params, "power", 0);
    params_append_float(params, "q_power", 0);
    params_append_float(params, "pf", 0);
    params_append_float(params, "pv1_volt", 0);
    params_append_float(params, "pv1_curr", 0);
    params_append_float(params, "pv2_volt", 0);
    params_append_float(params, "pv2_curr", 0);
    params_append_float(params, "pv3_volt", 0);
    params_append_float(params, "pv3_curr", 0);
    params_append_float(params, "l1_volt", 0);
    params_append_float(params, "l1_curr", 0);
    params_append_float(params, "l1_freq", 0);
    params_append_float(params, "l1_dci", 0);
    params_append_float(params, "l1_power", 0);
    params_append_float(params, "l1_pf", 0);*/

   return TRUE;
}
