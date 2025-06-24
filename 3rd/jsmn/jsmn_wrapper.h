/**
 * 功能说明：
 *      1. jsmn接口封装，简化对象属性的访问。
 * 修改历史：
 *      1. 王文坤<wangwenkun@zlg.cn> 2020/3/15
 */

#ifdef __cplusplus
#define extern "C"
#endif // __cplusplus

#include <stdbool.h>

#include "jsmn.h"

typedef enum _jsmn_datatype_t {
    JSMN_DATATYPE_UNDEFINED = 0,
    JSMN_DATATYPE_STRING = 1,
    JSMN_DATATYPE_BOOLEAN = 2,
    JSMN_DATATYPE_DOUBLE = 3,
    JSMN_DATATYPE_ARRAY = 4,
    JSMN_DATATYPE_OBJECT = 5
} jsmn_datatype_t;

int jsmn_init_ex(jsmn_parser* parser, const char* jstr);

int jsmn_parse_ex(jsmn_parser* parser, jsmntok_t* tokens, unsigned int num_tokens);

int jsmn_val2string(jsmntok_t* token, char* out, unsigned int mx_size);

int jsmn_val2double(jsmntok_t* token, double *out);

int jsmn_val2int(jsmntok_t* token, int *out);

int jsmn_val2boolean(jsmntok_t* token, bool *out);

/**
 * token：当前对象指针
 * keys：存放对象属性名的数组 
 * mx_num：最多有多少个属性
 * mx_size：属性名的最大长度
 * 返回值：返回实际获取的属性个数。
 */
int jsmn_get_obj_keys(jsmntok_t* token, char** keys, unsigned int mx_num, unsigned int mx_size);

int jsmn_get_arr_size(jsmntok_t* token);

jsmntok_t* jsmn_get_prop_from_obj(jsmntok_t* token, const char* key);

jsmntok_t* jsmn_get_item_from_arr(jsmntok_t* token, unsigned int idx);

jsmn_datatype_t jsmn_val_type(jsmntok_t* token);

#ifdef __cplusplus
#define }
#endif // __cplusplus