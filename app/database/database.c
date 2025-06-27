#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_order.h"
#include "database.h"
#include <time.h>

static int __database_create_table(sqlite3* db);
static int __database_create_result_desc_table(sqlite3* db, const char* path);
static int __database_create_product_table(sqlite3* db, const char* path);



sqlite3* database_init(const char* path)
{
    int rc = 0;
    sqlite3* db = NULL;

    if (!path) {
        return db;
    }
    rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        printf("Can not open database: %s\r\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
    }
    // rc = __database_create_table(db);
    return db;
}
int database_insert_result(sqlite3* db, test_reslut_t* test_reslut)
{
    int rc = 0;
    char* err_msg = NULL;
    char buf[512] = {0};
    sqlite3_stmt *stmt;

    if (!test_reslut) {
        return -1;
    }
    if (!db) {
        return -1;
    }
    sprintf(buf, "INSERT INTO test_result (type, num, order_id) VALUES ('%s', %d, %d);",
    test_reslut->type,
    test_reslut->num,
    test_reslut->order_id);
    const char* sql = buf;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not insert data: %s\r\n", err_msg);
        goto __end;
    }
    const char *sqlLastID = "SELECT last_insert_rowid();";
    rc = sqlite3_prepare_v2(db, sqlLastID, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int last_id = sqlite3_column_int(stmt, 0);
            test_reslut->id = last_id;
            // printf("Last inserted ID: %d\r\n", last_id);
        }
    sqlite3_finalize(stmt);
    } else {
        printf("Failed to retrieve last insert row ID\r\n");
    }
__end:
    sqlite3_free(err_msg);
    return rc;
}
int database_insert_order(sqlite3* db, test_order_t* test_order)
{
    int rc = 0;
    char* err_msg = NULL;
    char buf[512] = {0};
    sqlite3_stmt *stmt;

    time_t timestamp;
    timestamp = time(NULL);
    if (!test_order) {
        return -1;
    }
    if (!db) {
        return -1;
    }
    sprintf(buf, "INSERT INTO test_order (product_id, serial_no, uid, date, upload_status, model) VALUES (%d, %d, %d, %d, 0, '%s');",
    test_order->product_id,
    test_order->serial_no,
    test_order->uid,
    timestamp,
    test_order->model);
    const char* sql = buf;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not insert data: %s\r\n", err_msg);
        goto __end;
    }
    const char *sql_last_id = "SELECT last_insert_rowid();";
    rc = sqlite3_prepare_v2(db, sql_last_id, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int last_id = sqlite3_column_int(stmt, 0);
            test_order->id = last_id;
            // printf("Last inserted ID: %d\r\n", last_id);
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Failed to retrieve last insert row ID\r\n");
    }
    test_order->date = timestamp;
    for(uint32_t i = 0; i < test_order->test_reslut_size; i++) {
        (test_order->test_reslut + i)->order_id = test_order->id;
        database_insert_result(db, test_order->test_reslut + i);
    }
__end:
    sqlite3_free(err_msg);
    return rc;
}

static int __database_create_table(sqlite3* db)
{
    int rc = 0;

    if (!db) {
        return -1;
    }
    rc = __database_create_result_desc_table(db, "/home/result_desc.sql");
    if (rc != SQLITE_OK) {
        return rc;
    }
    rc = __database_create_product_table(db, "/home/product.sql");
    if (rc != SQLITE_OK) {
        return rc;
    }


}
static int __database_create_result_desc_table(sqlite3* db, const char* path)
{
    int rc = 0;
    char* err_msg = NULL;
    FILE *f;
    size_t file_size = 0;
    char* buff = NULL;

    if (!db) {
        return -1;
    }
    if (!path) {
        return -1;
    }
    
    f = fopen(path, "r");
    if (f == NULL) {
        goto __end;
    }
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buff = malloc(file_size);
    memset(buff, 0, file_size);
    fread(buff, sizeof(char), file_size, f);

    const char* sql = buff;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not create talbe: %s\r\n", err_msg);
        goto __end;
    }

__end:
    sqlite3_free(err_msg);
    fclose(f);
    f = NULL;
    if (buff) {
        free(buff);
        buff = NULL;
    }
    return rc;
}

static int __database_create_product_table(sqlite3* db, const char* path) 
{
    int rc = 0;
    char* err_msg = NULL;
    FILE *f;
    size_t file_size = 0;
    char* buff = NULL;

    if (!db) {
        return -1;
    }
    if (!path) {
        return -1;
    }
    
    f = fopen(path, "r");
    if (f == NULL) {
        goto __end;
    }
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buff = malloc(file_size);
    memset(buff, 0, file_size);
    fread(buff, sizeof(char), file_size, f);

    const char* sql = buff;

    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not create talbe: %s\r\n", err_msg);
        goto __end;
    }

__end:
    sqlite3_free(err_msg);
    fclose(f);
    f = NULL;
    if (buff) {
        free(buff);
        buff = NULL;
    }
    return rc;
}
const char* database_get_config(sqlite3* db, const char* key)
{
    int rc = 0;
    char buf[512] = {0};   
    sqlite3_stmt* stmt; 
    static char value[128] = {0};


    if (!db) {
        return NULL;
    }
    if (!key) {
        return NULL;
    }

    sprintf(buf, "select value from configuration where key = '%s';", key);
    const char* sql = buf;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Can not find %s\r\n", key);
        return NULL;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char* _value = sqlite3_column_text(stmt, 0);
        memset(value, 0, sizeof(value));
        memcpy(value, _value, strlen(_value));
        sqlite3_finalize(stmt);
    } else {
        return NULL;
        sqlite3_finalize(stmt);
    }

    return value;
}
int database_set_config(sqlite3* db, const char* key, const char* value)
{
    int rc = 0;
    char* err_msg = NULL;
    char buf[512] = {0};   
    sqlite3_stmt* stmt; 

    if (!db) {
        return -1;
    }
    if (!key) {
        return -1;
    }
    if (!value) {
        return -1;
    }

    sprintf(buf, "update  configuration set value = '%s' where key = '%s';", value, key);
    const char* sql = buf;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not set config: %s\r\n", err_msg);

    }
    sqlite3_free(err_msg);
    return rc;
}

int database_get_test_reslut(sqlite3* db, uint32_t test_order_id, test_reslut_t** test_reslut , uint32_t* test_reslut_size) 
{
    test_reslut_t* _test_reslut = NULL;
    int rc = 0;
    char buf[512] = {0};   
    sqlite3_stmt* stmt; 
    size_t _test_reslut_size = 0;

    if (!db) {
        return -1;
    }
    const char* sql = buf;
    sprintf(buf, "select count(*) \
    from test_result  inner join result_desc \ 
    where test_result.order_id = %d and test_result.num = result_desc.num and test_result.type = result_desc.type;", test_order_id);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Can not find order\r\n");
        goto __err1;
    }
    if (rc = sqlite3_step(stmt) == SQLITE_ROW) {
        _test_reslut_size = sqlite3_column_int(stmt, 0);
    }
    if (_test_reslut_size == 0) {
        rc = -1;
        goto __err1;
    }
    _test_reslut = malloc(sizeof(test_reslut_t) * _test_reslut_size);
    if (!_test_reslut) {
        rc = -1;
        goto __err1;
    }
    memset(_test_reslut, 0, sizeof(test_reslut_t) * _test_reslut_size);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "select test_result.id, test_result.num, test_result.order_id, test_result.type, result_desc.desc \
    from test_result  inner join result_desc \ 
    where test_result.order_id = %d and test_result.num = result_desc.num and test_result.type = result_desc.type;", test_order_id);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Can not find order\r\n");
        rc = -1;
        goto __err1;
    }
    for (int i = 0; i < _test_reslut_size; i++) {
        if (rc = sqlite3_step(stmt) == SQLITE_ROW) {
            (_test_reslut + i)->id = sqlite3_column_int(stmt, 0);
            (_test_reslut + i)->num = sqlite3_column_int(stmt, 1);
            (_test_reslut + i)->order_id = sqlite3_column_int(stmt, 2);
            const unsigned char* text = sqlite3_column_text(stmt, 3);
            memcpy((_test_reslut + i)->type, text, strlen(text));
            text = sqlite3_column_text(stmt, 4);
            memcpy((_test_reslut + i)->desc, text, strlen(text));
        } else {
            goto __err2;
        }
    }
    rc = 0;
    *test_reslut = _test_reslut;
    *test_reslut_size = _test_reslut_size;
    sqlite3_finalize(stmt);
    return rc;
__err1:
    sqlite3_finalize(stmt);
    return rc;
__err2:
    sqlite3_finalize(stmt);
    free(_test_reslut);
    _test_reslut = NULL;
    return rc;
}

int database_get_test_order(sqlite3* db, test_order_t** test_order)
{
    test_order_t* _test_order = NULL;
    test_reslut_t* _test_reslut = NULL;
    int rc = 0;
    char buf[512] = {0};   
    sqlite3_stmt* stmt; 

    if (!db) {
        return -1;
    }

    sprintf(buf, "select id, date, product_id, serial_no, uid, upload_status, model from test_order where upload_status = 0 order by date ASC;");
    const char* sql = buf;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Can not find order\r\n");
        return rc;
    }
    _test_order = malloc(sizeof(test_order_t));
    if (!_test_order) {
        goto __end;
    }
    memset(_test_order, 0, sizeof(test_order_t));
    
    if (rc = sqlite3_step(stmt) == SQLITE_ROW) {
        _test_order->id = sqlite3_column_int(stmt, 0);
        _test_order->date = sqlite3_column_int(stmt, 1);
        _test_order->product_id = sqlite3_column_int(stmt, 2);
        _test_order->serial_no = sqlite3_column_int(stmt, 3);
        _test_order->uid = sqlite3_column_int64(stmt, 4);
        _test_order->upload_status = sqlite3_column_int(stmt, 5);
        const char* text = sqlite3_column_text(stmt, 6);
        memcpy(_test_order->model, text, strlen(text));
        rc = database_get_test_reslut(db, _test_order->id, &_test_reslut, &_test_order->test_reslut_size);
        _test_order->test_reslut = _test_reslut;
        if (rc != 0) {
            test_order_destroy(_test_order);
            goto  __end;
        }
    } else {
        test_order_destroy(_test_order);
        goto  __end;
    }
    *test_order = _test_order;
__end:
    sqlite3_finalize(stmt);
    return rc;
}

int database_update_order_upload_status(sqlite3* db, uint32_t id, UPLOAD_STATU_T upload_status)
{
    int rc = 0;
    char* err_msg = NULL;
    char buf[512] = {0};   

    sprintf(buf, "update test_order set upload_status = %d where id = %d;", upload_status, id);
    const char* sql = buf;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Can not create talbe: %s\r\n", err_msg);
    }
    sqlite3_free(err_msg);
    return rc;
}