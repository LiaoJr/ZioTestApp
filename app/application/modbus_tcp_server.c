/**
 * \brief modbus tcp server(从机)部分代码
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "modbus/modbus.h"
#include "modbus/modbus-tcp.h"
#include "log.h"


#define LOG_TAG "MODBUS"
#define MODBUS_TCP_IP_ADDR "192.168.1.136"
#define MODBUS_TCP_SERVER_NB_CONNECTION 5


pthread_mutex_t mutex_of_mbreg;
modbus_mapping_t *mb_mapping;
static pthread_t pid_modbus;
static int server_socket;
static modbus_t *modbus_ctx;
static uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];


/**
 * @brief Modbus TCP Server 处理任务
*/
void *modbus_tcp_server_handler(void *p_arg)
{
    int ret = 0;
    int req_len = 0;
    modbus_t *modbus_ctx = (modbus_t *)p_arg;

    /** 循环接受客户端请求，并且响应客户端 */
    while(1){
        memset(query, 0, sizeof(query));
        req_len = modbus_receive(modbus_ctx, query);
        if(req_len < 0){
            /* Modbus client掉线或无连接 */
            printf("Waiting for TCP connection......\n");
            ret = modbus_tcp_accept(modbus_ctx, &server_socket);  /* 等待连接下一个客户端 */
            if(ret < 0){
                fprintf(stderr, "Modbus TCP server accept failed: %s\n", modbus_strerror(errno));
                return NULL;
            }
            printf("Modbus TCP server accept success, server_socket:%d\n", server_socket);
        } else if(req_len > 0){
            /* 接受到正常报文，做出响应 */
            pthread_mutex_lock(&mutex_of_mbreg);
            modbus_reply(modbus_ctx, query, req_len, mb_mapping);
            pthread_mutex_unlock(&mutex_of_mbreg);
            printf("Reply success, req_len:%d.\n", req_len);
        } else {
            /* 无效报文，忽略 */
            printf("TCP Frame ignored.\n");
        }
    }

    pthread_exit((void *)&ret);
    return NULL;
}

/**
 * @brief 配置modbus寄存器映射
*/
static void modbus_tcp_server_mapping_config(modbus_mapping_t *mb_mapping)
{
    mb_mapping->start_bits = 98;
    mb_mapping->start_input_bits = 99;
    mb_mapping->start_input_registers = 100;
    mb_mapping->start_registers = 200;
    for(int i = 0; i < 16; i++){
        mb_mapping->tab_registers[i] = 0;
    }
    printf("Output corg start: %d, count: %d\n", mb_mapping->start_bits, mb_mapping->nb_bits);
    printf("Input  corg start: %d, count: %d\n", mb_mapping->start_input_bits, mb_mapping->nb_input_bits);
    printf("Input   reg start: %d, count: %d\n", mb_mapping->start_input_registers, mb_mapping->nb_input_registers);
    printf("Holding reg start: %d, count: %d\n", mb_mapping->start_registers, mb_mapping->nb_registers);
}

/**
 * @brief  初始化modbus tcp server
*/
int modbus_tcp_server_init(void)
{
    int ret;

    /** modbus_new_tcp */
    modbus_ctx = modbus_new_tcp(MODBUS_TCP_IP_ADDR, MODBUS_TCP_DEFAULT_PORT);
    if(NULL == modbus_ctx){
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        ret = -1;
        goto FailRet;
    }
    //modbus_set_debug(modbus_ctx, TRUE);

    mb_mapping = modbus_mapping_new(8, 8, 256, 8);
    if(NULL == mb_mapping){
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        ret = -1;
        goto Fail1;
    }

    /** Modbus register init */
    modbus_tcp_server_mapping_config(mb_mapping);

    server_socket = modbus_tcp_listen(modbus_ctx, MODBUS_TCP_SERVER_NB_CONNECTION);
    if(server_socket < 0){
        fprintf(stderr, "Modbus tcp server listen failed: %s\n", modbus_strerror(errno));
        ret = -1;
        goto Fail2;
    }
    printf("Modbus TCP server listen on %s:%d success.\n", MODBUS_TCP_IP_ADDR, MODBUS_TCP_DEFAULT_PORT);

    /** 初始化互斥锁 */
    pthread_mutex_init(&mutex_of_mbreg, NULL);

    /** 创建线程响应Modbus TCP Client端请求 */
    ret = pthread_create(&pid_modbus, NULL, modbus_tcp_server_handler, (void *)modbus_ctx);
    if(ret != 0){
        LOG_E("Modbus pthread_create() failed:%d");
        ret = -1;
        goto FailRet;
    }
    ret = 0;
    return ret;

Fail2:
    modbus_mapping_free(mb_mapping);
Fail1:
    modbus_free(modbus_ctx);
FailRet:
    return ret;
}

void modbus_tcp_server_deinit(void)
{
    pthread_join(pid_modbus, NULL);
    LOG_D("Modbus TCP server thread is Joined\n");
    modbus_mapping_free(mb_mapping);
    modbus_close(modbus_ctx);
    modbus_free(modbus_ctx);
}
