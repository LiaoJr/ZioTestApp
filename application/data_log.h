#ifndef __DATA_LOG_H
#define __DATA_LOG_H




void data_log_init(void);

uint8_t data_log_cp(void *p_arg);
uint8_t data_log_do(void *p_arg);
uint8_t data_log_di(void *p_arg);
uint8_t data_log_ao(void *p_arg);
uint8_t data_log_ai(void *p_arg);
uint8_t data_log_dd(void *p_arg);


#endif
