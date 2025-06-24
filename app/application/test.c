#include <stdio.h>
#include "database.h"
#include "test_order.h"

int main() {
    printf("hello\r\n");
    upload_info_t* upload_info = get_upload_info_obj();
    upload_info->db = database_init("/home/test.db");
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
            .type = "Result_Ecat_CP",
            .num = -3
        },
    };
    test_order_t* test_order = NULL;
    test_order = test_order_create(0x01020005,
        0x00250522,
        2025052209344901,
        sizeof(test_reslut)/sizeof(test_reslut[0]),
        test_reslut);
    database_insert_order(upload_info->db, test_order);
    database_set_config(upload_info->db, "upload_id", "2");
    const char* upload_id = database_get_config(upload_info->db, "upload_id");
    if (upload_id) {
        upload_info->upload_id = atoi(upload_id);
    }
}