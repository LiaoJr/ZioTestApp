#ifndef _CMD_PROCESS_H_
#define _CMD_PROCESS_H_


#include <stdint.h>


#define SLAVE_CNT_ZCPC   5
#define SLAVE_CNT_ZIO_DO 3
#define SLAVE_CNT_ZIO_DI 3
#define SLAVE_CNT_ZIO_DD 3
#define SLAVE_CNT_ZIO_AO 4
#define SLAVE_CNT_ZIO_AI 4

#define PID_ZCPC_80801        (0x01020004)
#define PID_ZIOC_E0016DN      (0x0101001C)
#define PID_ZIOC_E0016DP      (0x0101001D)
#define PID_ZIOC_E1600DN      (0x0101001A)
#define PID_ZIOC_E1600DP      (0x0101001B)
#define PID_ZIOC_E0008AU      (0x01010023)
#define PID_ZIOC_E0008AU1     (0x01010025)
#define PID_ZIOC_E0800AI      (0x01010020)
#define PID_ZIOC_E0800AU      (0x01010021)
#define PID_ZIOC_E0800AU1     (0x01010022)
#define PID_ZCPC_80801_PIO    (0x01020005)
#define PID_ZIOC_E0016DP_PIO  (0x01010029)
#define PID_ZIOC_E1600DP_PIO  (0x01010028)
#define PID_ZIOC_E0008AU_PIO  (0x01010026)
#define PID_ZIOC_E0800AU1_PIO (0x01010024)
#define PID_ZIOD_E0808DN      (0x0101001E)
#define PID_ZIOD_E0808DP      (0x0101001F)


typedef uint8_t (*cmd_process_callback)(void *);

typedef enum{
    CMD_TYPE_REG = 0,
    CMD_ID_REG,
    CMD_DATA_REG,
    CMD_REQ_ACK_REG,
}cmd_reg_t;

typedef enum{
    CMD_TYPE_NONE = 0,
    CMD_TYPE_EEPROM_RW,
    CMD_TYPE_ZMCIO_DO,
    CMD_TYPE_ZMCIO_DI,
    CMD_TYPE_DATA_LOG,
    CMD_TYPE_END,
}cmd_type_t;

typedef enum{
    CMD_ID_NONE = 0,
    CMD_ID_EEPROM_READ,
    CMD_ID_EEPROM_WRITE,
    CMD_ID_EEPROM_ERASE,
    CMD_ID_ZMCIO_DO_LEVEL,
    CMD_ID_ZMCIO_DO_PWM,
    CMD_ID_DATA_LOG_CP,
    CMD_ID_DATA_LOG_DO,
    CMD_ID_DATA_LOG_DI,
    CMD_ID_DATA_LOG_AO,
    CMD_ID_DATA_LOG_AI,
    CMD_ID_DATA_LOG_DD,
    CMD_ID_DATA_LOG_PW,
    CMD_ID_ESI_UPDATE,
    CMD_ID_MAX_NUM,
}cmd_id_t;

typedef enum{
    CMD_DATA_NONE = 0,
    CMD_DATA_ZCPC_E80801 = 1,
    CMD_DATA_ZIOC_E0800AI = 2,
    CMD_DATA_ZIOC_E0800AU = 3,
    CMD_DATA_ZIOC_E0800AU1 = 4,
    CMD_DATA_ZIOC_E0008AU = 5,
    CMD_DATA_ZIOC_E0016DN = 6,
    CMD_DATA_ZIOC_E0016DP = 7,
    CMD_DATA_ZIOC_E1600DN = 8,
    CMD_DATA_ZIOC_E1600DP = 9,
    CMD_DATA_ZIOD_E0808DN = 10,
    CMD_DATA_ZIOD_E0808DP = 11,
    CMD_DATA_ZCPC_E80801_PIO = 12,
    CMD_DATA_ZIOC_E0016DP_PIO = 13,
    CMD_DATA_ZIOC_E1600DP_PIO = 14,
    CMD_DATA_ZIOC_E0008AU_PIO = 15,
    CMD_DATA_ZIOC_E0800AU1_PIO = 16,
    CMD_DATA_ZIOC_E0008AU1 = 17,
    CMD_DATA_MAX_NUM = 18,
}cmd_data_t;

typedef enum{
    CMD_ACK = 0,
    CMD_REQ,
}cmd_req_ack_t;

typedef struct cmd_manager{
    cmd_type_t            cmd_type;
    cmd_id_t              cmd_id;
    cmd_process_callback  pfn_cmd_process;
}cmd_manager_t;

/**
 * @brief 命令帧结构
 * 
 * @details | cmd type(16bits) | cmd id(16bits) | cmd data(16bits) | error(8bits) req/ack(8bits) |
 */
typedef struct cmd_receive{
    cmd_type_t     cmd_type;    /* 命令类型:16bits */
    cmd_id_t       cmd_id;    /* 命令id:16bits */
    uint16_t       cmd_data;    /* 命令数据:16bits */
    cmd_req_ack_t  cmd_req_ack;    /* 请求/响应:8bits */
    uint8_t        cmd_error;    /* 执行返回错误码:8bits */
}cmd_receive_t;

void cmd_process_register(cmd_manager_t *cmd_mgr, 
                            cmd_type_t cmd_type_in,
                            cmd_id_t cmd_id_in, 
                            cmd_process_callback pfn_cmd_process_callback);
int cmd_process_init(void);
void cmd_process_wait(void);
void cmd_process_deinit(void);


#endif  //_CMD_PROCESS_H_
