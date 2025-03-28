#ifndef _EEPROM_H
#define _EEPROM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "ecat/zecm_zh.h"


#define TX_PDO_MAX_BYTE     (8)
#define RX_PDO_MAX_BYTE     (8)

#define EEPROM_SIZE         (2048)
#define ESI_FILENAME_LEN    (128)

#define ESI_BIN_PATH            "./bin/"

#define ESI_ZCPC_E80801         "ZCPC-E80801_ESI.bin"

#define ESI_ZIOC_E0800AI        "ZIOC-E0800AI_ESI.bin"
#define ESI_ZIOC_E0800AU        "ZIOC-E0800AU_ESI.bin"
#define ESI_ZIOC_E0800AU1       "ZIOC-E0800AU1_ESI.bin"
#define ESI_ZIOC_E0008AU        "ZIOC-E0008AU_ESI.bin"
#define ESI_ZIOC_E0008AU1       "ZIOC-E0008AU1_ESI.bin"

#define ESI_ZIOC_E0016DN        "ZIOC-E0016DN_ESI.bin"
#define ESI_ZIOC_E0016DP        "ZIOC-E0016DP_ESI.bin"
#define ESI_ZIOC_E1600DN        "ZIOC-E1600DN_ESI.bin"
#define ESI_ZIOC_E1600DP        "ZIOC-E1600DP_ESI.bin"

#define ESI_ZIOD_E0808DN        "ZIOD-E0808DN_ESI.bin"
#define ESI_ZIOD_E0808DP        "ZIOD-E0808DP_ESI.bin"

#define ESI_ZCPC_E80801_PIO     "ZCPC-E80801-Pio_ESI.bin"
#define ESI_ZIOC_E0016DP_PIO    "ZIOC-E0016DP-Pio_ESI.bin"
#define ESI_ZIOC_E1600DP_PIO    "ZIOC-E1600DP-Pio_ESI.bin"
#define ESI_ZIOC_E0008AU_PIO    "ZIOC-E0008AU-Pio_ESI.bin"
#define ESI_ZIOC_E0800AU1_PIO   "ZIOC-E0800AU1-Pio_ESI.bin"



typedef struct _eeprom_ctx_t{    
    char *file_name;  //EEPROM二进制数据文件名
    uint16_t file_size;  //文件大小
    uint16_t eeprom_addr;  //EEPROM起始地址
    ECAT_WORD slave_count;  //从站数量
    ECAT_WORD slave_start;  //总站在组态中的起始位置
    uint32_t  product_id;
    uint32_t  serial_no;
    uint64_t  UID;
}eeprom_ctx_t;


uint8_t get_esi_file_by_pid(uint32_t pid_in);

uint8_t _read_eeprom(void *p_arg);
uint8_t _write_eeprom(void *p_arg);
uint8_t _erase_eeprom(void *p_arg);
uint8_t _update_esi(void *p_arg);

void eeprom_test_init(void);
void eeprom_wait_test_done(void);
void eeprom_test_deinit(void);


#endif // _EEPROM_H
