#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include "ecat/zecm_zh.h"
#include "cmd_process.h"
#include "modbus_tcp_server.h"
#include "ecat_master.h"


////////////////////////////////////////////////////////////////////////////////
#define _MSG_(flt,fmt,args...)  printf(flt ": %s<%d>: " fmt "\n", __PRETTY_FUNCTION__, __LINE__, ##args)
#define _INF_(fmt,args...)  _MSG_("INF", fmt, ##args)
#define _WRN_(fmt,args...)  _MSG_("WRN", fmt, ##args)
#define _ERR_(fmt,args...)  _MSG_("ERR", fmt, ##args)
#define _DBG_(fmt,args...)  _MSG_("DBG", fmt, ##args)
#define BREAK_IF_FAILED(ret, expr)  { ret = (expr); if (ECAT_FAILED(ret)) { _ERR_(#expr " failed: err=%d", ret); break; } else { _INF_(#expr " succeeded"); } }
#define LOG_RET(ret, expr)          { ret = (expr); if (ECAT_FAILED(ret)) { _ERR_(#expr " failed: err=%d", ret);        } else { _INF_(#expr " succeeded"); } }
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))
#define BIT_STR(v,i)  ((v).b.i ? (" " #i) : "")

////////////////////////////////////////////////////////////////////////////////
// 全局配置参数
#define ECAT_ETH0       "zecm0"
#define MAIN_CYCLE_TIME  1000                 // 主站过程数据通讯周期
#define SUB_CYCLE_TIME  (MAIN_CYCLE_TIME * 5) // 邮箱数据通讯周期
#define RPC_SERVER_PORT  5000                 // 开放与上位机通讯的端口号
#define MAX_SERVOS  4                         // 最大电机数
#define NUM_SERVOS  1                         // 总电机数
#define DETAIL_LOGS  0                        // 打印电机运行LOG


extern ECAT_SIMPLE_START_CONTEXT ctx;

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief main function
*/
int main(int argc, char **argv)
{
    ECAT_RESULT ret;

    // 将当前线程转为实时线程，优先级为50
    struct sched_param sched;
    sched.sched_priority = 50;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched);

    // 初始化主站环境
    LOG_RET(ret, EcatMasterLibInit(ECAT_NULL));
    if (ECAT_FAILED(ret)) return -1;

    /** 启动EtherCAT主站 */
    ecat_master_init();

    /** Modbus TCP服务端初始化 */
    ret = modbus_tcp_server_init();
    if(0 != ret){
        printf("Modbus TCP server init failed(%d).\n", ret);
        return ret;
    }

    /** 创建线程执行PLC命令处理 */
    cmd_process_init();

    /** 等待PLC命令处理完成 */
    cmd_process_wait();
    
    /** PLC命令处理完成，释放资源 */
    cmd_process_deinit();

    /** Modbus TCP服务端反初始化 */
    modbus_tcp_server_deinit();

    return 0;
}

