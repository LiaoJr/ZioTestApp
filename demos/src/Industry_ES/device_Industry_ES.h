
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

BOOL Industry_ES_dev_init(client_t* c, const char* owner, const char* type, const char* id, const char *secret);
// 获取工商储能的统计数据statistics_data
BOOL Industry_ES_dev_get_statistics_data(params_t* params);
// 获取工商储能的电池总控数据statistics_bms
BOOL Industry_ES_dev_get_statistics_bms(params_t* params);
// 获取工商储能的逆变器数据pcs1
BOOL Industry_ES_dev_get_pcs1(params_t* params);


END_C_DECLS

#endif/*DEVICE_DEVICE_H*/

