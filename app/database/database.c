#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_order.h"
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
            printf("Last inserted ID: %d\r\n", last_id);
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
    sprintf(buf, "INSERT INTO test_order (product_id, serial_no, uid, date) VALUES (%d, %d, %ld, %ld);",
    test_order->product_id,
    test_order->serial_no,
    test_order->uid,
    timestamp);
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
            printf("Last inserted ID: %d\r\n", last_id);
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
const char* database_get_config(sqlite3* db, const char * key)
{
    int rc = 0;
    char* err_msg = NULL;
    char buf[512] = {0};   
    sqlite3_stmt* stmt; 
    static char value[128] = {0};

    sprintf(buf, "select value from configuration where key = '%s'", key);
    const char* sql = buf;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Can not find %s: %s\r\n",key , err_msg);
        return value;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* _value =sqlite3_column_text(stmt, 0);
        // memcpy(value, , strlen);
    }
    sqlite3_finalize(stmt);
    return value;
}