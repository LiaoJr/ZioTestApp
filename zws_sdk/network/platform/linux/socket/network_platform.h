﻿/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef IOTSDKC_NETWORK_SOCKET_PLATFORM_H_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>

#define IOTSDKC_NETWORK_SOCKET_PLATFORM_H_H

typedef struct _SocketDataParams {
    int sock;
}SocketDataParams;

#ifdef __cplusplus
}
#endif

#endif //IOTSDKC_NETWORK_SOCKET_PLATFORM_H_H

