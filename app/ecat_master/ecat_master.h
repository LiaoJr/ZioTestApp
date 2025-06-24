#ifndef _ECAT_MASTER_H
#define _ECAT_MASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "ecat/zecm_zh.h"


#define ECAT_ENI "/opt/MasterOfZio/MasterOfZio.xml"
#define ECAT_ETH0 "zecm0"
#define ECAT_ETH1 "zecm1"
#define MAIN_CYCLE_TIME 1000                 // 主站过程数据通讯周期
#define SUB_CYCLE_TIME (MAIN_CYCLE_TIME * 5) // 邮箱数据通讯周期
#define RPC_SERVER_PORT 5000                 // 开放与上位机通讯的端口号
#define RECOVERY_TIMEOUTMs (100)


ECAT_RESULT ecat_master_change_state(void *mst_handle, uint8_t state, uint8_t retry_cnt);
ECAT_RESULT ecat_master_get_state(void *mst_handle, ECAT_BYTE *query);
ECAT_RESULT ecat_master_init(void);
void ecat_master_wait_set_handle_success(void);
void ecat_master_set_handle_success(void);
void ecat_master_wait_test_done(void);
void ecat_master_deinit();

#endif
