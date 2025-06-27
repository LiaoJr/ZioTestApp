#ifndef __TEST_ORDER_H__
#define __TEST_ORDER_H__

#include <stdio.h>
#include <stdint.h>
#include "sqlite3.h"
#include "upload_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct test_order test_order_t;
typedef struct test_reslut test_reslut_t;




struct test_order
{
    uint32_t id;
    char model[64];
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
    int32_t num;
    uint32_t order_id;
    char desc[512];
};


void test_order_destroy(test_order_t* test_order);

test_order_t* test_order_create(
                        const char* model,
                        uint32_t product_id,
                        uint32_t serial_no,
                        uint64_t uid,
                        uint32_t test_reslut_size,
                        test_reslut_t test_reslut[]);



#ifdef __cplusplus
}
#endif

#endif