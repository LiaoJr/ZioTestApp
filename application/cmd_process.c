#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "cmd_process.h"
#include "modbus/modbus.h"
#include "slave.h"
#include "eeprom.h"
#include "app_zmcio.h"
#include "data_log.h"


extern pthread_mutex_t mutex_of_mbreg;
extern modbus_mapping_t *mb_mapping;    /** modbus寄存器映射信息，请务必做好互斥访问 */

static pthread_t pid_cmd_process;
static cmd_manager_t zio_cmd_mgr[CMD_ID_MAX_NUM];
static cmd_receive_t zio_cmd_recv;    /** PLC发来的命令信息 */


/**
 * @brief PLC命令注册函数
*/
void cmd_process_register(cmd_manager_t *cmd_mgr, 
                            cmd_type_t cmd_type_in,
                            cmd_id_t cmd_id_in, 
                            cmd_process_callback pfn_cmd_process_callback)
{
    if(NULL == cmd_mgr || NULL == pfn_cmd_process_callback ||
        cmd_id_in > CMD_ID_MAX_NUM){
        return;
    }

    cmd_mgr[cmd_id_in].cmd_type = cmd_type_in;
    cmd_mgr[cmd_id_in].cmd_id = cmd_id_in;
    cmd_mgr[cmd_id_in].pfn_cmd_process = pfn_cmd_process_callback;
}


/**
 * @brief ZMC600E接受PLC命令处理线程
*/
static void *cmd_process_handler(void *args)
{
    uint8_t errno;

    eeprom_test_init();
    app_zmcio_init();
    data_log_init();

    printf(">>>Cmd process handler start:\n");

    while(1){
        /** 读取PLC发送的命令 */
        pthread_mutex_lock(&mutex_of_mbreg);
        zio_cmd_recv.cmd_type = mb_mapping->tab_registers[CMD_TYPE_REG];
        zio_cmd_recv.cmd_id = mb_mapping->tab_registers[CMD_ID_REG];
        zio_cmd_recv.cmd_data = mb_mapping->tab_registers[CMD_DATA_REG];
        zio_cmd_recv.cmd_req_ack = mb_mapping->tab_registers[CMD_REQ_ACK_REG] & 0xFF;
        pthread_mutex_unlock(&mutex_of_mbreg);

        switch(zio_cmd_recv.cmd_type){
        case CMD_TYPE_EEPROM_RW:    /** EEPROM读写命令 */
        case CMD_TYPE_ZMCIO_DO:  /* DO命令 */
        case CMD_TYPE_ZMCIO_DI:  /* DI命令 */
        case CMD_TYPE_DATA_LOG:  /* 数据记录命令 */
            if(zio_cmd_recv.cmd_id > CMD_ID_NONE && zio_cmd_recv.cmd_id < CMD_ID_MAX_NUM 
            && zio_cmd_recv.cmd_req_ack == CMD_REQ
            && zio_cmd_mgr[zio_cmd_recv.cmd_id].pfn_cmd_process != NULL){
                /* 执行命令对应的回调函数 */
                errno = zio_cmd_mgr[zio_cmd_recv.cmd_id].pfn_cmd_process((void *)&zio_cmd_recv.cmd_data);
                /* 执行完毕，req_ack寄存器置0，并返回执行结果 */
                pthread_mutex_lock(&mutex_of_mbreg);
                mb_mapping->tab_registers[CMD_REQ_ACK_REG] = CMD_ACK;
                mb_mapping->tab_registers[CMD_REQ_ACK_REG] |= errno << 8;
                pthread_mutex_unlock(&mutex_of_mbreg);
            }
            break;
        default:
            break;
        }

        sleep(1);    /* 每间隔1S读取一次命令 */
    }

    pthread_exit((void *)0);
}

/**
 * @brief ZMC600E接受PLC命令初始化，创建线程处理接收到的PLC命令测试
*/
int cmd_process_init(void)
{
    memset(&zio_cmd_recv, 0, sizeof(cmd_receive_t));
    memset(&zio_cmd_mgr, 0, sizeof(cmd_manager_t)*CMD_ID_MAX_NUM);

    /** 注册PLC命令回调函数 */
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_EEPROM_RW, CMD_ID_EEPROM_READ, _read_eeprom);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_EEPROM_RW, CMD_ID_EEPROM_WRITE, _write_eeprom);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_EEPROM_RW, CMD_ID_EEPROM_ERASE, _erase_eeprom);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_EEPROM_RW, CMD_ID_ESI_UPDATE, _update_esi);

    cmd_process_register(zio_cmd_mgr, CMD_TYPE_ZMCIO_DO, CMD_ID_ZMCIO_DO_LEVEL, app_zmcio_do_level);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_ZMCIO_DO, CMD_ID_ZMCIO_DO_PWM, app_zmcio_do_pwm);

    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_CP, data_log_cp);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_DO, data_log_do);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_DI, data_log_di);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_AO, data_log_ao);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_AI, data_log_ai);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_DD, data_log_dd);
    cmd_process_register(zio_cmd_mgr, CMD_TYPE_DATA_LOG, CMD_ID_DATA_LOG_PW, data_log_pw);
    
    int err = pthread_create(&pid_cmd_process, NULL, cmd_process_handler, NULL);
    if (err != 0){
        printf("cmd_process_init pthread_create() failed:%d", err);
        return err;
    }

    return 0;
}

/**
 * @brief PLC命令处理线程资源回收
*/
void cmd_process_wait(void)
{
    pthread_join(pid_cmd_process, NULL);
    printf(">>>Cmd process thread is joined\n");
}

/**
 * @brief PLC命令处理线程反初始化
*/
void cmd_process_deinit(void)
{
    eeprom_test_deinit();
    app_zmcio_deinit();
    printf(">>>Cmd process deinit\n");
}
