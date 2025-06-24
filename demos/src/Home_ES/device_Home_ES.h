
#include "client.h"
#include "../errors.h"
#include "../warnings.h"
#include "../device_consts.h"

#ifndef DEVICE_DEVICE_H
#define DEVICE_DEVICE_H

BEGIN_C_DECLS

extern unsigned char Industry_ES_query_strategy;
extern char Industry_ES_peak_shaving_buff[10240];
extern const char* Industry_ES_demand_control;

BOOL Home_ES_dev_init(client_t* c, const char* owner, const char* type, const char* id, const char *secret);
// 获取户用储能的电池数据bms
BOOL Home_ES_dev_get_bms(params_t* params);
// 获取户用储能的逆变器-额定条件数据rated
BOOL Home_ES_dev_get_rated(params_t* params);



END_C_DECLS

#endif/*DEVICE_DEVICE_H*/

