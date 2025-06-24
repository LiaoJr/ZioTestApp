#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../jsmn_wrapper.h"

int dumpObject(jsmntok_t *obj);

int main() {
	jsmn_parser p;
	jsmntok_t* ptoken;
	jsmntok_t* parray;
	jsmntok_t* pobject;
	jsmntok_t tokens[128];

	int r;
	double volt, status;
	char devname[64], devid[64];
	const char* const jstr = "{\"devname\": \"***\", \"devid\": \"***\", \"data\": [ \
	{\"volt\": 5, \"temp\": 37.5, \"enable\": false} \
	{\"volt\": 4.5, \"temp\": 36.7, \"enable\": true} \
	{\"volt\": 6, \"temp\": 39, \"enable\": false} \
	], \"status\": 1}";

	jsmn_init_ex(&p, jstr);
	r = jsmn_parse_ex(&p, tokens, sizeof(tokens)/sizeof(tokens[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

    ptoken = jsmn_get_prop_from_obj(tokens, "devname");
    if (ptoken == NULL) {
		printf("Failed to get object property[devname]\n");
		return 1;
	}
	r = jsmn_val2string(ptoken, devname, sizeof(devname) / sizeof(devname[0]));
	if (r < 0) {
		printf("Failed to get property[devname]'s value: %d\n", r);
		return 1;
	}

	ptoken = jsmn_get_prop_from_obj(tokens, "devid");
    if (ptoken == NULL) {
		printf("Failed to get object property[devid]\n");
		return 1;
	}
	r = jsmn_val2string(ptoken, devid, sizeof(devid) / sizeof(devid[0]));
	if (r < 0) {
		printf("Failed to get property[devid]'s value: %d\n", r);
		return 1;
	}

	ptoken = jsmn_get_prop_from_obj(tokens, "status");
    if (ptoken == NULL) {
		printf("Failed to get object property[status]\n");
		return 1;
	}
	r = jsmn_val2double(ptoken, &status);
	if (r < 0) {
		printf("Failed to get property[status]'s value: %d\n", r);
		return 1;
	}

	parray = jsmn_get_prop_from_obj(tokens, "data");
    if (parray == NULL) {
		printf("Failed to get object property[data]\n");
		return 1;
	}
	pobject = jsmn_get_item_from_arr(parray, 1);
	ptoken = jsmn_get_prop_from_obj(pobject, "volt");
	if (ptoken == NULL) {
		printf("Failed to get object property[volt]\n");
		return 1;
	}
	r = jsmn_val2double(ptoken, &volt);
	if (r < 0) {
		printf("Failed to get property[volt]'s value: %d\n", r);
		return 1;
	}

	printf("devname=%s, devid=%s, status=%d, data[1].vole=%0.2f\n", devname, devid, (int)status, volt);

	return EXIT_SUCCESS;

	/*
	int r;
	const char* const jstr = "{\"arr\": [{\"key1\": \"val1\"}], obj: {\"key2\": \"val2\"}}";

	jsmn_init_ex(&p, jstr);
	r = jsmn_parse_ex(&p, tokens, sizeof(tokens)/sizeof(tokens[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

	return dumpObject(tokens);
	*/
}

int dumpObject(jsmntok_t *obj) {
	int r, i, size;
	bool bval;
	double dval;
	char *keys[5], str[64];

	jsmntok_t* ptoken;

	for(int i = 0; i < 5; ++i) {
		keys[i] = (char *)malloc(64);
	}

	size = jsmn_get_obj_keys(obj, keys, 5, 64);
	if (size < 0) {
		printf("Failed to get object keys: %d\n", size);
		return 1;
	}

	for(i = 0; i < size; ++i) {
		printf("{ %s: ", keys[i]);

		ptoken = jsmn_get_prop_from_obj(obj, keys[i]);
		switch(jsmn_val_type(ptoken)) {
			case JSMN_DATATYPE_STRING:
				r = jsmn_val2string(ptoken, str, 64);
				if (r < 0) {
					printf("Failed to get property[%s]'s value: %d\n", keys[i], r);
					return 1;
				}
				printf("%s", str);
				break;
			case JSMN_DATATYPE_DOUBLE:
				r = jsmn_val2double(ptoken, &dval);
				if (r < 0) {
					printf("Failed to get property[%s]'s value: %d\n", keys[i], r);
					return 1;
				}
				printf("%0.2f", dval);
				break;
			case JSMN_DATATYPE_BOOLEAN:
				r = jsmn_val2boolean(ptoken, &bval);
				if (r < 0) {
					printf("Failed to get property[%s]'s value: %d\n", keys[i], r);
					return 1;
				}
				printf("%d", bval);
				break;
			case JSMN_DATATYPE_ARRAY:
				printf("[");
				for (int j = 0; j < ptoken->size; ++j) {
					dumpObject(jsmn_get_item_from_arr(ptoken, j));
				}
				printf("], ");
				break;
			default:
				dumpObject(ptoken);
		}
	}
	printf(" }, ");

	for(i = 0; i < 5; ++i) {
		free(keys[i]);
	}

	return EXIT_SUCCESS;
}
