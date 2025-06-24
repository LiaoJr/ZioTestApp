#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_order.h"

static upload_info_t __g_upload_info = {0};

test_order_t* test_order_create(
                        uint32_t product_id,
                        uint32_t serial_no,
                        uint64_t uid,
                        uint32_t test_reslut_size,
                        test_reslut_t test_reslut[])
{
    test_order_t* test_order = NULL;

    if (test_reslut_size <= 0) {
        goto __err;
    }
    if (!test_reslut) {
        goto __err;
    }
    test_order = malloc(sizeof(test_order_t));
    if (!test_order) {
        goto __err;
    }
    memset(test_order, 0, sizeof(test_order_t));
    test_order->product_id = product_id;
    test_order->serial_no = serial_no;
    test_order->uid = uid;
    test_order->test_reslut_size = test_reslut_size;
    test_order->test_reslut = malloc(sizeof(test_reslut_t) * test_order->test_reslut_size);
    for (uint32_t i = 0; i < test_reslut_size; i++) {
        memcpy(test_order->test_reslut + i, &(test_reslut[i]), sizeof(test_reslut_t));
    }
    return test_order;
__err:
    if(test_order) {
        free(test_order);
        test_order = NULL;
    }
    return test_order;
}

upload_info_t* get_upload_info_obj()
{
    return &__g_upload_info;
}