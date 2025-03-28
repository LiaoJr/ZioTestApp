#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "main.h"
#include "cmd_process.h"
#include "modbus/modbus.h"
#include "eeprom.h"
#include "ecat_master.h"
#include "slave.h"
#include "ticks.h"
#include "log.h"


#define LOG_TAG "EEPROM"


extern ECAT_SIMPLE_START_CONTEXT ctx;
extern modbus_mapping_t *mb_mapping;    /** modbus寄存器映射信息，请务必做好互斥访问 */

static uint16_t zio_slv_cnt[CMD_DATA_MAX_NUM] = {
    0,
    SLAVE_CNT_ZCPC,
    SLAVE_CNT_ZIO_AI,
    SLAVE_CNT_ZIO_AI,
    SLAVE_CNT_ZIO_AI,
    SLAVE_CNT_ZIO_AO,
    SLAVE_CNT_ZIO_DO,
    SLAVE_CNT_ZIO_DO,
    SLAVE_CNT_ZIO_DI,
    SLAVE_CNT_ZIO_DI,
    SLAVE_CNT_ZIO_DD,
    SLAVE_CNT_ZIO_DD,
    SLAVE_CNT_ZCPC,
    SLAVE_CNT_ZIO_DO,
    SLAVE_CNT_ZIO_DI,
    SLAVE_CNT_ZIO_AO,
    SLAVE_CNT_ZIO_AI,
    SLAVE_CNT_ZIO_AO
};

eeprom_ctx_t eepromctx = {
    .file_name = NULL, 
    .file_size = 0,
    .eeprom_addr = 0,
    .slave_count = 0,
    .slave_start = 0,
    .product_id = 0,
    .serial_no = 0,
    .UID = 0
};


uint8_t set_esi_file_by_id(uint8_t esi_id_in)
{
    memset(eepromctx.file_name, 0, ESI_FILENAME_LEN);
    switch(esi_id_in){
    case CMD_DATA_ZCPC_E80801:
        strcpy(eepromctx.file_name, ESI_ZCPC_E80801);
        break;
    case CMD_DATA_ZIOC_E0800AI:
        strcat(eepromctx.file_name, ESI_ZIOC_E0800AI);
        break;
    case CMD_DATA_ZIOC_E0800AU:
        strcat(eepromctx.file_name, ESI_ZIOC_E0800AU);
        break;
    case CMD_DATA_ZIOC_E0800AU1:
        strcat(eepromctx.file_name, ESI_ZIOC_E0800AU1);
        break;
    case CMD_DATA_ZIOC_E0008AU:
        strcat(eepromctx.file_name, ESI_ZIOC_E0008AU);
        break;
    case CMD_DATA_ZIOC_E0016DN:
        strcat(eepromctx.file_name, ESI_ZIOC_E0016DN);
        break;
    case CMD_DATA_ZIOC_E0016DP:
        strcat(eepromctx.file_name, ESI_ZIOC_E0016DP);
        break;
    case CMD_DATA_ZIOC_E1600DN:
        strcat(eepromctx.file_name, ESI_ZIOC_E1600DN);
        break;
    case CMD_DATA_ZIOC_E1600DP:
        strcat(eepromctx.file_name, ESI_ZIOC_E1600DP);
        break;
    case CMD_DATA_ZIOD_E0808DN:
        strcat(eepromctx.file_name, ESI_ZIOD_E0808DN);
        break;
    case CMD_DATA_ZIOD_E0808DP:
        strcat(eepromctx.file_name, ESI_ZIOD_E0808DP);
        break;
    case CMD_DATA_ZCPC_E80801_PIO:
        strcat(eepromctx.file_name, ESI_ZCPC_E80801_PIO);
        break;
    case CMD_DATA_ZIOC_E0016DP_PIO:
        strcat(eepromctx.file_name, ESI_ZIOC_E0016DP_PIO);
        break;
    case CMD_DATA_ZIOC_E1600DP_PIO:
        strcat(eepromctx.file_name, ESI_ZIOC_E1600DP_PIO);
        break;
    case CMD_DATA_ZIOC_E0008AU_PIO:
        strcat(eepromctx.file_name, ESI_ZIOC_E0008AU_PIO);
        break;
    case CMD_DATA_ZIOC_E0800AU1_PIO:
        strcat(eepromctx.file_name, ESI_ZIOC_E0800AU1_PIO);
        break;
    case CMD_DATA_ZIOC_E0008AU1:
        strcat(eepromctx.file_name, ESI_ZIOC_E0008AU1);
        break;
    default:
        /** ESI文件不存在 */
        printf("ESI file didn't exit.\n");
        return 1;
    }

    return 0;
}

uint8_t get_esi_file_by_pid(uint32_t pid_in)
{
    memset(eepromctx.file_name, 0, ESI_FILENAME_LEN);
    switch(pid_in){
    case PID_ZCPC_80801:
        strcpy(eepromctx.file_name, ESI_ZCPC_E80801);
        break;
    case PID_ZIOC_E0016DN:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0016DN);
        break;
    case PID_ZIOC_E0016DP:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0016DP);
        break;
    case PID_ZIOC_E1600DN:
        strcpy(eepromctx.file_name, ESI_ZIOC_E1600DN);
        break;
    case PID_ZIOC_E1600DP:
        strcpy(eepromctx.file_name, ESI_ZIOC_E1600DP);
        break;
    case PID_ZIOC_E0008AU:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0008AU);
        break;
    case PID_ZIOC_E0008AU1:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0008AU1);
        break;
    case PID_ZIOC_E0800AI:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0800AI);
        break;
    case PID_ZIOC_E0800AU:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0800AU);
        break;
    case PID_ZIOC_E0800AU1:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0800AU1);
        break;
    case PID_ZCPC_80801_PIO:
        strcpy(eepromctx.file_name, ESI_ZCPC_E80801_PIO);
        break;
    case PID_ZIOC_E0016DP_PIO:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0016DP_PIO);
        break;
    case PID_ZIOC_E1600DP_PIO:
        strcpy(eepromctx.file_name, ESI_ZIOC_E1600DP_PIO);
        break;
    case PID_ZIOC_E0008AU_PIO:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0008AU_PIO);
        break;
    case PID_ZIOC_E0800AU1_PIO:
        strcpy(eepromctx.file_name, ESI_ZIOC_E0800AU1_PIO);
        break;
    case PID_ZIOD_E0808DN:
        strcpy(eepromctx.file_name, ESI_ZIOD_E0808DN);
        break;
    case PID_ZIOD_E0808DP:
        strcpy(eepromctx.file_name, ESI_ZIOD_E0808DP);
        break;
    default:
        printf("wrong product ID\n");
        return 1;
    }
    return 0;
}

/**
 * @brief 将EtherCAT从站中的EEPROM数据读取到文件中
 * 
 * @details 根据PLC发送的命令中的从站节点位置读取从站的EEPROM数据并存到本地
 *          cmd_data: [ eeprom_size(8bits) | slave_start(8bits) ]
 *          for eeprom_size: set bit0 for 256 bytes, 
 *                           set bit1 for 512 bytes, 
 *                           set bit2 for 1024 bytes, 
 *                           set bit3 for 2048 bytes, 
 *                           set bit4 for 4096 bytes, 
 *                           set bit5 for 8192 bytes, 
 *                           set bit6 for 16384 bytes, 
 *                           set bit7 for 65536 bytes, 
*/
uint8_t _read_eeprom(void *p_arg)
{
    uint8_t errno = 0;
    uint8_t cmd_data_eeprom_size;
    uint8_t i;
    uint64_t temp;
    ECAT_RESULT ret_ecat;
    uint16_t eeprom_cmd_data = *((uint16_t *)p_arg);

    cmd_data_eeprom_size = (eeprom_cmd_data & 0xFF00) >> 8;
    i = 0;
    do{
        if(cmd_data_eeprom_size & 0x1){
            eepromctx.file_size = pow(2, i+8);
        }
        cmd_data_eeprom_size >>= 1;
        i++;
    }while(cmd_data_eeprom_size);

    eepromctx.slave_count = 1;  /* 一次只能读一个从站 */
    eepromctx.slave_start = eeprom_cmd_data & 0xFF;
    eepromctx.eeprom_addr = 0;
    sprintf(eepromctx.file_name, "ESI_Slave%d.bin", eepromctx.slave_start);

    do{
        printf("\n>>>>>>>>>>EEPROM read processing...\n");
        ret_ecat = read_eeprom_to_file(ctx.Master, &eepromctx);
        if (ret_ecat != ECAT_S_OK){
            printf("\tRead EEPROM failed, ret_ecat = %d\n", ret_ecat);
            errno = 2;
            return errno;
        }

        temp = eepromctx.UID;
        mb_mapping->tab_registers[4] = temp / 1000000000000;  //get year
        temp %= 1000000000000;
        mb_mapping->tab_registers[5] = temp / 10000000000 << 8;  // get month
        temp %= 10000000000;
        mb_mapping->tab_registers[5] |= temp / 100000000;  //get day
        temp %= 100000000;
        mb_mapping->tab_registers[6] = temp / 1000000 << 8;  //get hour
        temp %= 1000000;
        mb_mapping->tab_registers[6] |= temp / 10000;  //get mimute
        temp %= 10000;
        mb_mapping->tab_registers[7] = temp / 100 << 8;  //get second
        temp %= 100;
        mb_mapping->tab_registers[7] |= temp;  //get number

        mb_mapping->tab_registers[8] = eepromctx.serial_no >> 16;
        mb_mapping->tab_registers[9] = eepromctx.serial_no & 0xFFFF;
        
        errno = 0;
        printf("\tSlave position:%d, data size:%d, data file: %s\n",
                eepromctx.slave_start, 
                eepromctx.file_size,
                eepromctx.file_name);
        printf("\tPID: %08X, SerialNo: %08X, Slave UID: %ld\n"
                "\tsave eeprom data to file [%s] success.\n", 
                eepromctx.product_id,
                eepromctx.serial_no, 
                eepromctx.UID, 
                eepromctx.file_name);
    } while (0);

    return errno;
}

/**
 * @brief 将ESI数据写入到EtherCAT从站的EEPROM中
*/
uint8_t _write_eeprom(void *p_arg)
{
    uint8_t errno = 0;
    uint16_t date_rx[4];
    uint16_t slave_count;
    uint16_t year, month, day, hour, minute, sec, number;
    uint8_t esi_id;
    ECAT_RESULT ret_ecat;
    uint16_t eeprom_cmd_data = *((uint16_t *)p_arg);

    if(mb_mapping->tab_registers[4] == 0 && mb_mapping->tab_registers[5] == 0 &&
    mb_mapping->tab_registers[6] == 0 && mb_mapping->tab_registers[7] == 0){
        for(int i = 0; i < 4; i++){
            date_rx[i] = 0;
        }
        eepromctx.UID = 0xFFFFFFFFFFFFFFFF;
    }else{
        date_rx[0] = mb_mapping->tab_registers[4];
        date_rx[1] = mb_mapping->tab_registers[5];
        date_rx[2] = mb_mapping->tab_registers[6];
        date_rx[3] = mb_mapping->tab_registers[7];

        year = date_rx[0];
        month = date_rx[1] >> 8;
        day = date_rx[1] & 0xFF;
        hour = date_rx[2] >> 8;
        minute = date_rx[2] & 0xFF;
        sec = date_rx[3] >> 8;
        number = date_rx[3] & 0xFF;
        eepromctx.UID  = (uint64_t)year*1000000000000;
        eepromctx.UID += (uint64_t)month*10000000000;
        eepromctx.UID += (uint64_t)day*100000000;
        eepromctx.UID += (uint64_t)hour*1000000;
        eepromctx.UID += (uint64_t)minute*10000;
        eepromctx.UID += (uint64_t)sec*100;
        eepromctx.UID += number;
    }

    esi_id = (eeprom_cmd_data & 0xFF00) >> 8;
    errno = set_esi_file_by_id(esi_id);
    if(errno != 0){
        return errno;
    }

    eepromctx.file_size = 1024;
    eepromctx.serial_no = (mb_mapping->tab_registers[8] << 16) + mb_mapping->tab_registers[9];

    eepromctx.slave_count = 1;  /* 一次只能写一个从站 */
    eepromctx.slave_start = eeprom_cmd_data & 0xFF;
    eepromctx.eeprom_addr = 0;
    do{
        ret_ecat = get_slave_count(ctx.Master, &slave_count);  /* 获取组态中的从站数量 */
        if(slave_count < zio_slv_cnt[esi_id]){
            printf("\twrong EtherCAT slave count\n");
            return 2;
        }
        ret_ecat = write_eeprom_from_file(ctx.Master, &eepromctx);
        if (ret_ecat != ECAT_S_OK){
            printf("\tWrite EEPROM failed, ret_ecat = %d\n", ret_ecat);
            errno = 1;
            return errno;
        }

        printf("\n>>>>>>>>>>EEPROM write processing...\n"
                "\tSlave count:%d, Slave position:%d, data file name: %s\n"
                "\tPID: %08X, Serial No: %08X, UID: %ld\n", 
                slave_count,
                eepromctx.slave_start, 
                eepromctx.file_name,
                eepromctx.product_id,
                eepromctx.serial_no,
                eepromctx.UID);
        printf("\tWrite eeprom data file [%s] to EtherCAT slave success.\n", eepromctx.file_name);
    } while (0);

    return errno;
}

/**
 * @brief 擦除EtherCAT从站的EEPROM
 * 
 * @details 根据PLC发送的命令中的从站节点位置读取从站的EEPROM数据并存到本地
 *          cmd_data: [ eeprom_size(8bits) | slave_start(8bits) ]
 *          for eeprom_size: set bit0 for 256 bytes, 
 *                           set bit1 for 512 bytes, 
 *                           set bit2 for 1024 bytes, 
 *                           set bit3 for 2048 bytes, 
 *                           set bit4 for 4096 bytes, 
 *                           set bit5 for 8192 bytes, 
 *                           set bit6 for 16384 bytes, 
 *                           set bit7 for 65536 bytes, 
*/
uint8_t _erase_eeprom(void *p_arg)
{
    uint8_t errno = 0;
    uint8_t cmd_data_eeprom_size;
    uint8_t i;
    ECAT_RESULT ret_ecat;
    uint16_t eeprom_cmd_data = *((uint16_t *)p_arg);

    cmd_data_eeprom_size = (eeprom_cmd_data & 0xFF00) >> 8;

    i = 0;
    do{
        if(cmd_data_eeprom_size & 0x1){
            eepromctx.file_size = pow(2, i+8);
        }
        cmd_data_eeprom_size >>= 1;
        i++;
    }while(cmd_data_eeprom_size);

    eepromctx.slave_count = 1;    /* 一次只能擦除一个从站EEPROM */
    eepromctx.slave_start = eeprom_cmd_data & 0xFF;

    do{
        printf("\n>>>>>>>>>>EEPROM erase processing...\n"
                "\tSlave position:%d, data size:%d\n", 
                eepromctx.slave_start, 
                eepromctx.file_size);
        ret_ecat = erase_eeprom(ctx.Master, 
                                eepromctx.slave_count, 
                                eepromctx.slave_start,
                                eepromctx.file_size);
        if (ret_ecat != ECAT_S_OK){
            printf("\tWrite EEPROM failed, ret_ecat = %d\n", ret_ecat);
            errno = 1;
            return errno;
        }

        errno = 0;
        printf("\tErasing EEPROM of EtherCAT slave[%d] success.\n", eepromctx.slave_start);
    } while (0);

    return errno;
}

/**
 * @brief 读出从站的ESI文件数据并更新到设备中
 */
uint8_t _update_esi(void *p_arg)
{
    uint8_t errno = 0;
    uint8_t cmd_data_eeprom_size;
    uint8_t i;
    ECAT_RESULT ret_ecat;
    uint16_t eeprom_cmd_data = *((uint16_t *)p_arg);

    cmd_data_eeprom_size = (eeprom_cmd_data & 0xFF00) >> 8;
    i = 0;
    do{
        if(cmd_data_eeprom_size & 0x1){
            eepromctx.file_size = pow(2, i+8);
        }
        cmd_data_eeprom_size >>= 1;
        i++;
    }while(cmd_data_eeprom_size);

    eepromctx.slave_count = 1;  /* 一次只能读一个从站 */
    eepromctx.slave_start = eeprom_cmd_data & 0xFF;  /* 从站地址 */
    eepromctx.eeprom_addr = 0;

    do{
        printf("\n>>>>>>>>>>ESI update processing...\n");
        ret_ecat = read_eeprom_for_esi_update(ctx.Master, &eepromctx);
        if (ret_ecat != ECAT_S_OK){
            printf("\tRead EEPROM failed, ret_ecat = %d\n", ret_ecat);
            errno = 2;
            return errno;
        }

        errno = 0;
        printf("\tSlave position:%d, data size:%d,  PID: %08X\n",
                eepromctx.slave_start, 
                eepromctx.file_size,
                eepromctx.product_id);
        printf("\tESI data file name: %s\n"
                "\tESI file update success.\n", 
                eepromctx.file_name);
    } while (0);

    return errno;
}


void eeprom_test_init(void)
{
    eepromctx.file_name = (ECAT_CHAR *)malloc(ESI_FILENAME_LEN);

    LOG_D("eeprom test init finish\n");
}

void eeprom_wait_test_done(void)
{
    /* Nothing to wait */
}

void eeprom_test_deinit(void)
{
    LOG_D("%s ", __FUNCTION__);
    if (eepromctx.file_name){
        free(eepromctx.file_name);
    }
    EcatResetExtCtrlTaskHandler(ctx.Master);
    EcatSimpleStop(&ctx);
}
