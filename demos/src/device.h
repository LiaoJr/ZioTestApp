
#include "client.h"
#include "errors.h"
#include "warnings.h"
#include "device_consts.h"

#ifndef DEVICE_DEVICE_H
#define DEVICE_DEVICE_H

BEGIN_C_DECLS

BOOL mydev_init(client_t* c, const char* owner, const char* type, const char* id, const char *secret);
BOOL mydev_get_all_data(params_t* params);

BOOL mygw_init(client_t* c, const char* owner, const char* type,const char* id, const char *secret);
BOOL mygw_get_all_data(params_t* params);


END_C_DECLS

#endif/*DEVICE_DEVICE_H*/

