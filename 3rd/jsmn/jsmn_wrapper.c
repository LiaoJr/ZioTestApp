#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "jsmn_wrapper.h"

static const char* JSON_STRING = NULL;
static int obj_val_size(jsmntok_t* token);

int jsmn_init_ex(jsmn_parser* parser, const char* jstr) {
    if (NULL == parser || NULL == jstr) {
        return -1;
    }

    JSON_STRING = jstr;
    jsmn_init(parser);

    return 0;
}

int jsmn_parse_ex(jsmn_parser* parser, jsmntok_t* tokens, unsigned int num_tokens) {
    return jsmn_parse(parser, JSON_STRING, strlen(JSON_STRING), tokens, num_tokens);
}

int jsmn_val2string(jsmntok_t* token, char* out, unsigned int mx_size) {
    int ncopys;

    if (NULL == token || NULL == out || 0 >= mx_size) {
        return -1;
    }

    ncopys = token->end - token->start;
    ncopys = ncopys > (int)mx_size - 1 ? (int)mx_size - 1 : ncopys;
    strncpy(out, JSON_STRING + token->start, ncopys);
    out[ncopys] = '\0';

    return 0;
}

int jsmn_val2double(jsmntok_t* token, double *out) {
    char str[64];

    if (NULL == token) {
        return -1;
    }

    if (JSMN_PRIMITIVE != token->type) {
        return -1;
    }

    snprintf(str, token->end - token->start + 1, "%s", JSON_STRING + token->start);
    *out = atof(str);

    return 0;
}

int jsmn_val2int(jsmntok_t* token, int *out) {
    char str[64];

    if (NULL == token) {
        return -1;
    }

    if (JSMN_PRIMITIVE != token->type) {
        return -1;
    }

    snprintf(str, token->end - token->start + 1, "%s", JSON_STRING + token->start);
    *out = atoi(str);

    return 0;
}

int jsmn_val2boolean(jsmntok_t* token, bool *out) {
    if (NULL == token) {
        return -1;
    }

    if (JSMN_PRIMITIVE != token->type) {
        return -1;
    }

    if (0 == strncmp("true", JSON_STRING + token->start, token->end - token->start)) {
        *out = true;
    } else {
        *out = false;
    }

    return 0;
}

int jsmn_get_obj_keys(jsmntok_t* token, char** keys, unsigned int mx_num, unsigned int mx_size) {
    int i, count = 0, num_keys = token->size;

    if (NULL == token || NULL == keys || 0 >= mx_num || 0 >= mx_size) {
        return -1;
    }

    if (JSMN_OBJECT != token->type) {
        return -1;
    }

    for (i = 0; i < num_keys; ++i) {
        int ncopys;
        jsmntok_t* k = ++token;

        if (i > mx_num || NULL == *keys) {
            return count;
        }

        ncopys = k->end - k->start;
        ncopys = ncopys > (int)mx_size - 1 ? (int)mx_size : ncopys;

        strncpy(*keys, JSON_STRING + k->start, ncopys);
        *((*keys) + ncopys) = '\0';

        ++keys;
        ++count;
        token += obj_val_size(token + 1);
    }

    return count;
}

int jsmn_get_arr_size(jsmntok_t* token) {
    if (NULL == token) {
        return -1;
    }

    if (JSMN_ARRAY != token->type) {
        return -1;
    }

    return token->size;
}

jsmntok_t* jsmn_get_prop_from_obj(jsmntok_t* token, const char* key) {
    int i, num_keys = token->size;

    if (NULL == token || NULL == key) {
        return NULL;
    }

    if (JSMN_OBJECT != token->type) {
        return NULL;
    }

    for(i = 0; i < num_keys; ++i) {
        jsmntok_t* k = ++token;
        jsmntok_t* v = token + 1;

        if (!strncmp(key, JSON_STRING + k->start, k->end - k->start)) {
            return v;
        }

        token += obj_val_size(v);
    }

    return NULL;
}

jsmntok_t* jsmn_get_item_from_arr(jsmntok_t* token, unsigned int idx) {
    int i, size = (++token)->size;

    if (NULL == token || 0 > idx || idx > size) {
        return NULL;
    }

    if (0 == idx) {
        return token;
    }

    for (i = 0; i < size; ++i) {
        if (idx > i) {
            token += obj_val_size(token);
        }
    }

    return token;
}

jsmn_datatype_t jsmn_val_type(jsmntok_t* token) {
    int i;
    jsmn_datatype_t dtype;

    if (NULL == token){
        return JSMN_DATATYPE_UNDEFINED;
    }

    switch(token->type) {
        case JSMN_STRING:
            dtype = JSMN_DATATYPE_STRING;
            break;
        case JSMN_ARRAY:
            dtype = JSMN_DATATYPE_ARRAY;
            break;
        case JSMN_OBJECT:
            dtype = JSMN_DATATYPE_OBJECT;
            break;
        case JSMN_PRIMITIVE:
            for(i = token->start; i < token->end; ++i) {
                if(!isdigit(JSON_STRING[i])) {
                    return JSMN_DATATYPE_BOOLEAN;
                }
            }

            dtype = JSMN_DATATYPE_DOUBLE;
            break;
        default:
            dtype = JSMN_DATATYPE_UNDEFINED;
            break;
    }

    return dtype;
}

int obj_val_size(jsmntok_t* token) {
    int i, offset = 0;

    switch(token->type) {
    case JSMN_OBJECT:
        offset += 1;

        for(i = 0; i < token->size; ++i) {
            ++offset;
            offset += obj_val_size(token + offset);
        }
        break;
    case JSMN_ARRAY:
        offset += 1;

        for(i = 0; i < token->size; ++i) {
            offset += obj_val_size(token + offset);
        }
        break;
    default:
        offset += 1;
    }

    return offset;
}