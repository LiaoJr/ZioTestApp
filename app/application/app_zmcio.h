#ifndef _APP_ZMCIO_H_
#define _APP_ZMCIO_H_



typedef struct zmcio_ctl {
    int cmd;        //命令
    int io;         //需要操作的DI/DO
    int val;        //設置或獲取的值
}zmcio_ctl_t;


void app_zmcio_init(void);
uint8_t app_zmcio_do_level(void *p_arg);
uint8_t app_zmcio_do_pwm(void *p_arg);
void app_zmcio_deinit(void);


#endif  //_APP_ZMCIO_H_
