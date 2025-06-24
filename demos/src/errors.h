#include "client.h"

#ifndef CLIENT_ERRORS_H
#define CLIENT_ERRORS_H

BOOL myinvert_report_temperature_too_high(client_t* client, float value);

BOOL myinvert_report_log_multi(client_t* client, const char* log_multi);

#endif/*CLIENT_ERRORS_H*/

