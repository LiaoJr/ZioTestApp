#ifndef __TEST_ORDER_H__
#define __TEST_ORDER_H__

#include <stdio.h>
#include <stdint.h>
#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct test_order test_order_t;
typedef struct test_reslut test_reslut_t;
typedef struct upload_info upload_info_t;

typedef enum {
    NO_UPLOAD = 0,
    UPLOADED = 1
} UPLOAD_STATU_T;

struct test_order
{
    uint32_t id;
    uint32_t product_id;
    uint32_t serial_no;
    uint64_t uid;
    uint32_t date;
    UPLOAD_STATU_T upload_status;
    uint32_t test_reslut_size;
    test_reslut_t* test_reslut;
};

struct test_reslut
{
    uint32_t id;
    char type[64];
    uint32_t num;
    uint32_t order_id;
    char desc[512];
};

struct upload_info
{
    sqlite3* db;
    uint32_t upload_id;
    uint32_t upload_date;
};

void test_order_destroy(test_order_t* test_order);

test_order_t* test_order_create(
                        uint32_t product_id,
                        uint32_t serial_no,
                        uint64_t uid,
                        uint32_t test_reslut_size,
                        test_reslut_t test_reslut[]);

upload_info_t* get_upload_info_obj();

#ifdef __cplusplus
}
#endif

#endif