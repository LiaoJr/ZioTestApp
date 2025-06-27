#ifndef __UPLOAD_DATA_H__
#define __UPLOAD_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "sqlite3.h"
#include <semaphore.h>
#include <pthread.h>

#include "client.h"

typedef struct upload_info upload_info_t;

typedef enum {
    NO_UPLOAD = 0,
    UPLOADED = 1
} UPLOAD_STATU_T;

struct upload_info
{
    sqlite3* db;
    uint32_t upload_id;
    uint32_t upload_date;
    sem_t sem;
    pthread_t thread_id;
    dev_meta_info_t meta;
};

upload_info_t* get_upload_info_obj();
int uplaod_info_notify_new_data(upload_info_t* upload_info);
int upload_info_wait_for_data(upload_info_t* upload_info, uint64_t ms);
int upload_info_init(upload_info_t* upload_info, const char* meta_id, const char* meta_type, const char* meta_secret);
int upload_task_entry();



#ifdef __cplusplus
}
#endif

#endif