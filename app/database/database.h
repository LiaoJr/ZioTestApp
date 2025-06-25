#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "sqlite3.h"
#include "test_order.h"

#ifdef __cplusplus
extern "C" {
#endif

sqlite3* database_init(const char* path);
int database_insert_order(sqlite3* db, test_order_t* test_order);
int database_insert_result(sqlite3* db, test_reslut_t* test_reslut);
const char* database_get_config(sqlite3* db, const char* key);
int database_set_config(sqlite3* db, const char* key, const char* value);
int database_get_test_order(sqlite3* db, test_order_t** test_order);
int database_update_order_upload_status(sqlite3* db, uint32_t id, UPLOAD_STATU_T upload_status);
int database_get_test_reslut(sqlite3* db, uint32_t test_order_id, test_reslut_t** test_reslut , uint32_t* test_reslut_size);
#ifdef __cplusplus
}
#endif

#endif