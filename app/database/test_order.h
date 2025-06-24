#ifndef __TEST_ORDER_H__
#define __TEST_ORDER_H__

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct test_order test_order_t;
typedef struct test_reslut test_reslut_t;

struct test_order
{
    uint32_t id;
    uint32_t product_id;
    uint32_t serial_no;
    uint64_t uid;
    uint32_t test_reslut_size;
    uint32_t date;
    test_reslut_t* test_reslut;
};

struct test_reslut
{
    uint32_t id;
    char type[64];
    uint32_t num;
    uint32_t order_id;
    const char* desc;
};

test_order_t* test_order_create(
                        uint32_t product_id,
                        uint32_t serial_no,
                        uint64_t uid,
                        uint32_t test_reslut_size,
                        test_reslut_t test_reslut[]);

#ifdef __cplusplus
}
#endif

#endif