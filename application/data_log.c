/**
 * @brief 按日期将各类模块的测试数据存入本地中
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "modbus/modbus.h"
#include "eeprom.h"
#include "cmd_process.h"
#include "log.h"


#define LOG_TAG "DATA_LOG"


#define DATA_LOG_DIR          "data_log"


const char csv_header_cp[] = 
    "Product ID,"
    "Serial No,"
    "UID,"
    "Result_CPs,"
    "Result_Eeprom_CP,"
    "Result_Ecat_CP,"
    "Result_CP_Vol,"
    "Result_LEDs_CP\r\n";

const char csv_header_do[] = 
    "Product ID,"
    "Serial No,"
    "UID,"
    "Result_DOs,"
    "Result_Eeprom_DO,"
    "Result_Ecat_DO,"
    "Result_DOs_Shortcut,"
    "Result_DOs_Carry,"
    "Result_LEDs_DO\r\n";

const char csv_header_di[] = 
    "Product ID,"
    "Serial No,"
    "UID,"
    "Result_DIs,"
    "Result_Eeprom_DI,"
    "Result_Ecat_DI,"
    "Result_DIs_2ms,"
    "Result_DIs_4ms,"
    "Result_LEDs_DI\r\n";

const char csv_header_ai[] = 
    "Product ID,"
    "Serial No,"
    "UID,"
    "Result_AIs,"
    "Result_ADC_AI,"
    "Result_Eeprom_AI,"
    "Result_Ecat_AI,"
    "Result_AIs_Cal,"
    "Result_AIs_Test,"
    "Result_LEDs_AI\r\n";

const char csv_header_ao[] = 
    "Product ID,"
    "Serial No,"
    "UID,"
    "Result_AOs,"
    "Result_ADC_AO,"
    "Result_Eeprom_AO,"
    "Result_Ecat_AO,"
    "Result_AOs_Cal,"
    "Result_AOs_Test,"
    "Result_LEDs_AO\r\n";


extern modbus_mapping_t *mb_mapping;    /** modbus寄存器映射信息，请务必做好互斥访问 */
extern eeprom_ctx_t eepromctx;


void data_log_init(void)
{
    int ret;
    struct stat statbuf;
    DIR *dir;
    struct dirent *entry;
    char file_name[512];
    time_t mtime, time_now;
    double time_diff;

    ret = stat(DATA_LOG_DIR, &statbuf);
    if(ret != 0){
        /* data_log did not exist */
        printf("stat file %s failed\n", DATA_LOG_DIR);
        mkdir(DATA_LOG_DIR, 0755);
        return;
    }
    if(!S_ISDIR(statbuf.st_mode)){
        /* directory data_log dose not exist */
        printf("file %s dose not exist.\n", DATA_LOG_DIR);
        return;
    }

    dir = opendir(DATA_LOG_DIR);
    if(NULL == dir){
        perror("open directory failed\n");
        return;
    }

    time_now = time(NULL);
    while(NULL != (entry = readdir(dir))){
        if(entry->d_name[0] == '.'){
            continue;
        }

        memset(file_name, 0, sizeof(file_name));
        sprintf(file_name, "./%s/%s", DATA_LOG_DIR, entry->d_name);
        ret = stat(file_name, &statbuf);
        mtime = statbuf.st_mtime;        
        time_diff = difftime(time_now, mtime);
        if((uint64_t)time_diff > 31104000){  /* 3600*24*30*12 = 31104000 */
            /* 若存在data log文件超过1年 */
            unlink(file_name);  /* 删除该文件 */
            printf("\t>>>>>data log file [%s] deleted.\n", file_name);
        }
    }

    LOG_D("data log init finish\n");
    closedir(dir);
}

uint8_t data_log_cp(void *p_arg)
{
    uint8_t ret = 0;

    printf("\n>>>>>>>>>>CP module data log processing...\n");
    printf("\tPID:%08x\n", eepromctx.product_id);
    printf("\tSerial No: %08X\n", eepromctx.serial_no);
    printf("\tUID: %ld\n", eepromctx.UID);
    printf("\tResult_CP: %d\n", mb_mapping->tab_registers[4]);
    printf("\tResult_Eeprom_CP: %d\n", mb_mapping->tab_registers[5]);
    printf("\tResult_Ecat_CP: %d\n", mb_mapping->tab_registers[6]);
    printf("\tResult_CP_Vol: %d\n", mb_mapping->tab_registers[7]);
    printf("\tResult_CP_Vol: %d\n", mb_mapping->tab_registers[8]);

    return ret;
}

uint8_t data_log_do(void *p_arg)
{
    uint8_t ret = 0;
    int fd;
    char log_file_name[512] = {0};
    char result_log[512] = {0};

    printf("\n>>>>>>>>>>DO module data log processing...\n");
    printf("\tPID:%08x\n", eepromctx.product_id);
    printf("\tSerial No: %08X\n", eepromctx.serial_no);
    printf("\tUID: %ld\n", eepromctx.UID);
    printf("\tResult_DOs: %d\n", (int16_t)mb_mapping->tab_registers[4]);
    printf("\tResult_Eeprom_DO: %d\n", (int16_t)mb_mapping->tab_registers[5]);
    printf("\tResult_Ecat_DO: %d\n", (int16_t)mb_mapping->tab_registers[6]);
    printf("\tResult_DOs_Shoutcut: %d\n", (int16_t)mb_mapping->tab_registers[7]);
    printf("\tResult_DOs_Carry: %d\n", (int16_t)mb_mapping->tab_registers[8]);
    printf("\tResult_LEDs_DO: %d\n", (int16_t)mb_mapping->tab_registers[9]);

    switch(eepromctx.product_id){
    case PID_ZIOC_E0016DN:
        sprintf(log_file_name, "./%s/ZIOC-E0016DN_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0016DP:
        sprintf(log_file_name, "./%s/ZIOC-E0016DP_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0016DP_PIO:
        sprintf(log_file_name, "./%s/ZIOC-E0016DP-Pio_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    }

    /* 创建打开data log文件 */
    fd = open(log_file_name, O_CREAT|O_RDWR|O_TRUNC, 0664);
    if(fd < 0){
        perror("creat data log file failed");
        return 1;
    }

    write(fd, csv_header_do, strlen(csv_header_do));
    sprintf(result_log, "%08X,%08X,%ld,%d,%d,%d,%d,%d,%d\r\n",
                        eepromctx.product_id,
                        eepromctx.serial_no,
                        eepromctx.UID,
                        (int16_t)mb_mapping->tab_registers[4],
                        (int16_t)mb_mapping->tab_registers[5],
                        (int16_t)mb_mapping->tab_registers[6],
                        (int16_t)mb_mapping->tab_registers[7],
                        (int16_t)mb_mapping->tab_registers[8],
                        (int16_t)mb_mapping->tab_registers[9]);
    write(fd, result_log, strlen(result_log));  /* 写入测试结果 */

    ret = close(fd);
    if(ret < 0){
        printf("\tData log file [%s] create failed.\n", log_file_name);
        ret = 1;
    }else{
        printf("\tData log file [%s] create success.\n", log_file_name);
    }
    return ret;
}

uint8_t data_log_di(void *p_arg)
{
    uint8_t ret = 0;
    int fd;
    char log_file_name[512] = {0};
    char result_log[512] = {0};

    printf("\n>>>>>>>>>>DI module data log processing...\n");
    printf("\tPID:%08x\n", eepromctx.product_id);
    printf("\tSerial No: %08X\n", eepromctx.serial_no);
    printf("\tUID: %ld\n", eepromctx.UID);
    printf("\tResult_DI: %d\n", (int16_t)mb_mapping->tab_registers[4]);
    printf("\tResult_Eeprom_DI: %d\n", (int16_t)mb_mapping->tab_registers[5]);
    printf("\tResult_Ecat_DI: %d\n", (int16_t)mb_mapping->tab_registers[6]);
    printf("\tResult_DIs_2ms: %d\n", (int16_t)mb_mapping->tab_registers[7]);
    printf("\tResult_DIs_4ms: %d\n", (int16_t)mb_mapping->tab_registers[8]);
    printf("\tResult_LEDs_DI: %d\n", (int16_t)mb_mapping->tab_registers[9]);

    switch(eepromctx.product_id){
    case PID_ZIOC_E1600DN:
        sprintf(log_file_name, "./%s/ZIOC-E1600DN_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E1600DP:
        sprintf(log_file_name, "./%s/ZIOC-E1600DP_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E1600DP_PIO:
        sprintf(log_file_name, "./%s/ZIOC-E1600DP-Pio_%ld.csv",
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    }

    /* 创建打开data log文件 */
    fd = open(log_file_name, O_CREAT|O_RDWR|O_TRUNC, 0664);
    if(fd < 0){
        perror("creat data log file failed");
        return 1;
    }

    write(fd, csv_header_di, strlen(csv_header_di));
    sprintf(result_log, "%08X,%08X,%ld,%d,%d,%d,%d,%d,%d\r\n",
                        eepromctx.product_id,
                        eepromctx.serial_no,
                        eepromctx.UID,
                        (int16_t)mb_mapping->tab_registers[4],
                        (int16_t)mb_mapping->tab_registers[5],
                        (int16_t)mb_mapping->tab_registers[6],
                        (int16_t)mb_mapping->tab_registers[7],
                        (int16_t)mb_mapping->tab_registers[8],
                        (int16_t)mb_mapping->tab_registers[9]);
    write(fd, result_log, strlen(result_log));  /* 写入测试结果 */

    ret = close(fd);
    if(ret < 0){
        printf("\tData log file [%s] create failed.\n", log_file_name);
        ret = 1;
    }else{
        printf("\tData log file [%s] create success.\n", log_file_name);
    }
    return ret;
}

uint8_t data_log_ao(void *p_arg)
{
    uint8_t ret = 0;
    int fd;
    char log_file_name[512] = {0};
    char result_log[512] = {0};
    char data_log[512] = {0};
    char str_temp[256] = {0};
    float adc_smp_val[40] = {0};
    float ao_set_val[40] = {0};
    float measure_error[40] = {0};
    uint32_t *p_uint32;

    printf("\n>>>>>>>>>>AO module data log processing...\n");
    printf("\tPID:%08x\n", eepromctx.product_id);
    printf("\tSerial No: %08X\n", eepromctx.serial_no);
    printf("\tUID: %ld\n", eepromctx.UID);
    printf("\tResult_AOs: %d\n", (int16_t)mb_mapping->tab_registers[4]);
    printf("\tResult_ADC_AO: %d\n", (int16_t)mb_mapping->tab_registers[5]);
    printf("\tResult_Eeprom_AO: %d\n", (int16_t)mb_mapping->tab_registers[6]);
    printf("\tResult_Ecat_AO: %d\n", (int16_t)mb_mapping->tab_registers[7]);
    printf("\tResult_AOs_Cal: %d\n", (int16_t)mb_mapping->tab_registers[8]);
    printf("\tResult_AOs_Test: %d\n", (int16_t)mb_mapping->tab_registers[9]);
    printf("\tResult_LEDs_AO: %d\n", (int16_t)mb_mapping->tab_registers[10]);

    switch(eepromctx.product_id){
    case PID_ZIOC_E0008AU:
        sprintf(log_file_name, "./%s/ZIOC-E0008AU_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0008AU1:
        sprintf(log_file_name, "./%s/ZIOC-E0008AU1_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0008AU_PIO:
        sprintf(log_file_name, "./%s/ZIOC-E0008AU-Pio_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    }

    /* 创建打开data log文件 */
    fd = open(log_file_name, O_CREAT|O_RDWR|O_TRUNC, 0664);
    if(fd < 0){
        perror("creat data log file failed");
        return 1;
    }

    write(fd, csv_header_ao, strlen(csv_header_ao));
    sprintf(result_log, "%08X,%08X,%ld,%d,%d,%d,%d,%d,%d,%d\r\n",
                        eepromctx.product_id,
                        eepromctx.serial_no,
                        eepromctx.UID,
                        (int16_t)mb_mapping->tab_registers[4],
                        (int16_t)mb_mapping->tab_registers[5],
                        (int16_t)mb_mapping->tab_registers[6],
                        (int16_t)mb_mapping->tab_registers[7],
                        (int16_t)mb_mapping->tab_registers[8],
                        (int16_t)mb_mapping->tab_registers[9],
                        (int16_t)mb_mapping->tab_registers[10]);
    write(fd, result_log, strlen(result_log));  /* 写入测试结果 */

    /** 写入测试数据 */
    memset(result_log, 0, sizeof(result_log));
    write(fd, "\r\n", 2);
    for(int i = 0, j = 12, k = 92, n = 172; 
    i < sizeof(ao_set_val)/sizeof(ao_set_val[0]); i++){
        p_uint32 = (uint32_t *)(ao_set_val + i);
        *p_uint32 = mb_mapping->tab_registers[j];
        *p_uint32 |= ((uint32_t)mb_mapping->tab_registers[j+1]) << 16;
        j += 2;

        p_uint32 = (uint32_t *)(adc_smp_val + i);
        *p_uint32 = mb_mapping->tab_registers[k];
        *p_uint32 = ((uint32_t)mb_mapping->tab_registers[k+1]) << 16;
        k += 2;

        p_uint32 = (uint32_t *)(measure_error + i);
        *p_uint32 = mb_mapping->tab_registers[n];
        *p_uint32 = ((uint32_t)mb_mapping->tab_registers[n+1]) << 16;
        n += 2;

        if(i % 8 == 0){
            memset(str_temp, 0, sizeof(str_temp));
            sprintf(str_temp,  "Data of test point %d\r\n", i / 8);
            write(fd, str_temp, strlen(str_temp));
        }
        memset(data_log, 0, sizeof(data_log));
        sprintf(data_log, "Channel[%d],"
                            "product set value: %.7f,"
                            "ADC sample value: %.7f,"
                            "measure error: %.7f%%\r\n",
                            i % 8, 
                            ao_set_val[i], 
                            adc_smp_val[i], 
                            measure_error[i]);
        write(fd, data_log, strlen(data_log));
    }

    ret = close(fd);
    if(ret < 0){
        printf("\tData log file [%s] create failed.\n", log_file_name);
        ret = 1;
    }else{
        printf("\tData log file [%s] create success.\n", log_file_name);
    }
    return ret;
}

uint8_t data_log_ai(void *p_arg)
{
    uint8_t ret = 0;
    int fd;
    char log_file_name[512] = {0};
    char result_log[512] = {0};
    char data_log[512] = {0};
    char str_temp[256] = {0};
    float adc_smp_val[40] = {0};
    float ai_smp_val[40] = {0};
    float measure_error[40] = {0};
    uint32_t *p_uint32;

    printf("\n>>>>>>>>>>AI module data log processing...\n");
    printf("\tPID:%08x\n", eepromctx.product_id);
    printf("\tSerial No: %08X\n", eepromctx.serial_no);
    printf("\tUID: %ld\n", eepromctx.UID);
    printf("\tResult_AIs: %d\n", (int16_t)mb_mapping->tab_registers[4]);
    printf("\tResult_ADC_AI: %d\n", (int16_t)mb_mapping->tab_registers[5]);
    printf("\tResult_Eeprom_AI: %d\n", (int16_t)mb_mapping->tab_registers[6]);
    printf("\tResult_Ecat_AI: %d\n", (int16_t)mb_mapping->tab_registers[7]);
    printf("\tResult_AIs_Cal: %d\n", (int16_t)mb_mapping->tab_registers[8]);
    printf("\tResult_AIs_Test: %d\n", (int16_t)mb_mapping->tab_registers[9]);
    printf("\tResult_LEDs_AI: %d\n", (int16_t)mb_mapping->tab_registers[10]);
    //for(int i = 0; i < 240; i++){
    //    printf("%d\n", mb_mapping->tab_registers[i+12]);
    //}

    switch(eepromctx.product_id){
    case PID_ZIOC_E0800AI:
        sprintf(log_file_name, "./%s/ZIOC-E0800AI_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0800AU:
        sprintf(log_file_name, "./%s/ZIOC-E0800AU_%ld.csv", 
                                DATA_LOG_DIR,
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0800AU1:
        sprintf(log_file_name, "./%s/ZIOC-E0800AU1_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    case PID_ZIOC_E0800AU1_PIO:
        sprintf(log_file_name, "./%s/ZIOC-E0800AU1-Pio_%ld.csv", 
                                DATA_LOG_DIR, 
                                eepromctx.UID);
        break;
    }

    /* 创建打开data log文件 */
    fd = open(log_file_name, O_CREAT|O_RDWR|O_TRUNC, 0664);
    if(fd < 0){
        perror("creat data log file failed");
        return 1;
    }

    write(fd, csv_header_ai, strlen(csv_header_ai));
    sprintf(result_log, "%08X,%08X,%ld,%d,%d,%d,%d,%d,%d,%d\r\n",
                        eepromctx.product_id,
                        eepromctx.serial_no,
                        eepromctx.UID,
                        (int16_t)mb_mapping->tab_registers[4],
                        (int16_t)mb_mapping->tab_registers[5],
                        (int16_t)mb_mapping->tab_registers[6],
                        (int16_t)mb_mapping->tab_registers[7],
                        (int16_t)mb_mapping->tab_registers[8],
                        (int16_t)mb_mapping->tab_registers[9],
                        (int16_t)mb_mapping->tab_registers[10]);
    write(fd, result_log, strlen(result_log));  /* 写入测试结果 */

    /** 写入测试数据 */
    memset(result_log, 0, sizeof(result_log));
    write(fd, "\r\n", 2);
    for(int i = 0, j = 12, k = 92, n = 172; 
    i < sizeof(adc_smp_val)/sizeof(adc_smp_val[0]); i++){
        p_uint32 = (uint32_t *)(adc_smp_val + i);
        *p_uint32 = mb_mapping->tab_registers[j];
        *p_uint32 |= ((uint32_t)mb_mapping->tab_registers[j+1]) << 16;
        j += 2;

        p_uint32 = (uint32_t *)(ai_smp_val + i);
        *p_uint32 = mb_mapping->tab_registers[k];
        *p_uint32 = ((uint32_t)mb_mapping->tab_registers[k+1]) << 16;
        k += 2;

        p_uint32 = (uint32_t *)(measure_error + i);
        *p_uint32 = mb_mapping->tab_registers[n];
        *p_uint32 = ((uint32_t)mb_mapping->tab_registers[n+1]) << 16;
        n += 2;

        if(i % 8 == 0){
            memset(str_temp, 0, sizeof(str_temp));
            sprintf(str_temp,  "Data of test point %d\r\n", i / 8);
            write(fd, str_temp, strlen(str_temp));
        }
        memset(data_log, 0, sizeof(data_log));
        sprintf(data_log, "Channel[%d],"
                            "adc sample value: %.7f,"
                            "product sample value: %.7f,"
                            "measure error: %.7f%%\r\n",
                            i % 8, 
                            adc_smp_val[i], 
                            ai_smp_val[i], 
                            measure_error[i]);
        write(fd, data_log, strlen(data_log));
    }

    ret = close(fd);
    if(ret < 0){
        printf("\tData log file [%s] create failed.\n", log_file_name);
        ret = 1;
    }else{
        printf("\tData log file [%s] create success.\n", log_file_name);
    }
    return ret;
}

uint8_t data_log_dd(void *p_arg)
{
    uint8_t ret = 0;

    return ret;
}


uint8_t data_log_pw(void *p_arg)
{
    uint8_t ret = 0;

    printf("\n>>>>>>>>>>PW module data log processing...\n");
    printf("\tResult_PW: %d\n", mb_mapping->tab_registers[4]);
    printf("\tResult_Ecat_PW: %d\n", mb_mapping->tab_registers[5]);
    printf("\tResult_PW_Vol: %d\n", mb_mapping->tab_registers[6]);

    return ret;
}