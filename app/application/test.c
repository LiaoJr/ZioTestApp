#include <stdio.h>
#include "database.h"
#include "test_order.h"

int main() {
    printf("hello\r\n");
    upload_info_t* upload_info = get_upload_info_obj();
    upload_info->db = database_init("/home/test.db");

    /* 插入一条测试单 */
    test_reslut_t test_reslut[] = {
        {
            .type = "Result_Eeprom_CP",
            .num = 1
        },
        {
            .type = "Result_Ecat_CP",
            .num = -1
        },
        {
            .type = "Result_LEDs_CP",
            .num = -2
        },
    };
    test_order_t* test_order = NULL;
    test_order = test_order_create(0x01020005,
        0x00250522,
        2025052209344901,
        sizeof(test_reslut)/sizeof(test_reslut[0]),
        test_reslut);
    database_insert_order(upload_info->db, test_order);
    test_order_destroy(test_order);

    /* 获取一条数据库中未上传的测试单 */
    database_get_test_order(upload_info->db, &test_order);
    /* 更新上传状态 */
    database_update_order_upload_status(upload_info->db, test_order->id, UPLOADED);
    test_order_destroy(test_order);
}