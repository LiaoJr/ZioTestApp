#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include <sys/time.h>
#include <sys/types.h>
#include "app_zmcio.h"
#include "log.h"


#define LOG_TAG "ZMCIO"

#define TEST_DEVICE "/dev/zmcio"
#define MAX_DATA_LEN 128
#define MAX_CMD_LEN  128

#define ZMCIO_MODE_DI   (1 << 0)
#define ZMCIO_MODE_DO   (1 << 1)
#define ZMCIO_MODE_PWM  (1 << 2)
#define ZMCIO_MODE_EQEP (1 << 3)

#define ZMCIO_CMD_SET_MODE_DI   (1 << 0)
#define ZMCIO_CMD_SET_MODE_DO   (1 << 1)
#define ZMCIO_CMD_SET_MODE_PWM  (1 << 2)
#define ZMCIO_CMD_SET_MODE_EQEP (1 << 3)
#define ZMCIO_CMD_GET_MODE_DI   (1 << 4)
#define ZMCIO_CMD_GET_MODE_DO   (1 << 5)

#define ZMCIO_CMD_GET_DI_NUM    (1 << 6)
#define ZMCIO_CMD_GET_DO_NUM    (1 << 7)

#define ZMCIO_CMD_GET_DI        (1 << 8)
#define ZMCIO_CMD_GET_DO        (1 << 9)
#define ZMCIO_CMD_SET_DO        (1 << 10)

//zmcio set mode c pwm

static int fd_zmcio;


/**
 * @brief ZMC600E IO应用初始化
*/
void app_zmcio_init(void)
{
    fd_zmcio = open(TEST_DEVICE, O_RDWR, 0);
    if(fd_zmcio < 0){
        fprintf(stderr, "open <%s> error: %s\n", TEST_DEVICE, strerror(errno));
        return;
    }
    LOG_D("zmcio init finish\n");
}

/**
 * @brief IO应用，DO电平输出执行，对应cmd_id:4.
*/
uint8_t app_zmcio_do_level(void *p_arg)
{
    uint8_t ret = 0;
    zmcio_ctl_t msg;
    uint16_t zmcio_cmd_data = *((uint16_t *)p_arg);

    if(fd_zmcio < 0){
        return (2);
    }

    memset(&msg, 0, sizeof(zmcio_ctl_t));

    printf("\n>>>>>>>>>>zmcio do set processing...\n");
    /* set channel as do mode */
    msg.cmd = ZMCIO_CMD_SET_MODE_DO;
    msg.io = zmcio_cmd_data >> 8;
    if(ioctl(fd_zmcio, 0, &msg) < 0){
        printf("\tzmcio do[%d] set mode do failed\n", msg.io);
        ret = 1;
        return ret;
    }
    printf("\tzmcio do[%d] set mode do success\n", msg.io);

    /* set output level */
    msg.cmd = ZMCIO_CMD_SET_DO;
    msg.io = zmcio_cmd_data >> 8;
    msg.val = zmcio_cmd_data & 0xFF;
    if(ioctl(fd_zmcio, 0, &msg) < 0){
        printf("\tzmcio do[%d] output failed\n", msg.io);
        ret = 1;
        return ret;
    }
    printf("\tzmcio do[%d] output val: %d\n", msg.io, msg.val);

    ret = 0;
    return ret;
}

/**
 * @brief IO应用，PWM输出执行, 对应cmd_id:5.
*/
uint8_t app_zmcio_do_pwm(void *p_arg)
{
    uint8_t ret = 0;
    zmcio_ctl_t msg;
    uint8_t channel, period_ms, duty_cycle_ms;
    char cmd[MAX_CMD_LEN] = {0};
    uint16_t zmcio_cmd_data = *((uint16_t *)p_arg);

    if(fd_zmcio < 0){
        return (2);
    }

    memset(&msg, 0, sizeof(zmcio_ctl_t));
    printf("\n>>>>>>>>>>zmcio do set processing...\n");
    /* set channel as pwm mode */
    msg.cmd = ZMCIO_CMD_SET_MODE_PWM;
    msg.io = zmcio_cmd_data >> 12;
    if(ioctl(fd_zmcio, 0, &msg) < 0){
        printf("\tzmcio do[%d] set mode pwm failed\n", msg.io);
        ret = 1;
        return ret;
    }
    printf("\tzmcio do[%d] set mode pwm success\n", msg.io);

    channel = zmcio_cmd_data >> 12;
    period_ms = zmcio_cmd_data >> 6 & 0x3F;
    duty_cycle_ms = zmcio_cmd_data & 0x3F;
    
    if(channel < 12){
        printf("\tInvalid chanel, pwm output failed\n");
        ret = 1;
        return ret;
    }

    if(period_ms == 0){
        /* disable pwm output */
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "echo 0  >  /dev/pwm/do%d-pwm/enable", channel);
        printf("\tcmd: [%s]\n", cmd);
        system(cmd);
        ret = 0;
        return ret;
    }

    /* set period */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo %d  >  /dev/pwm/do%d-pwm/period", period_ms*1000000, channel);
    printf("\tcmd: [%s]\n", cmd);
    system(cmd);

    /* set duty cycle */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo %d  >  /dev/pwm/do%d-pwm/duty_cycle", duty_cycle_ms*1000000, channel);
    printf("\tcmd: [%s]\n", cmd);
    system(cmd);

    /* enable pwm */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo 1  >  /dev/pwm/do%d-pwm/enable", channel);
    printf("\tcmd: [%s]\n", cmd);
    system(cmd);

    ret = 0;
    return ret;
}

void app_zmcio_deinit(void)
{
    close(fd_zmcio);
}
