#include <stdio.h>
#include "database.h"

int main(void)
{
    printf("hello\r\n");
    sqlite3* db = database_init("/opt/MasterOfZio/test.db");
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
                                    (uint64_t)2025052209344901,
                                    sizeof(test_reslut)/sizeof(test_reslut[0]),
                                    test_reslut);

    database_insert_order(db, test_order);
}
