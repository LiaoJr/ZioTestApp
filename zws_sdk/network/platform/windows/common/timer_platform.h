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

#ifndef SRC_PROTOCOL_MQTT_AWS_IOT_EMBEDDED_CLIENT_WRAPPER_PLATFORM_LINUX_COMMON_TIMER_PLATFORM_H_
#define SRC_PROTOCOL_MQTT_AWS_IOT_EMBEDDED_CLIENT_WRAPPER_PLATFORM_LINUX_COMMON_TIMER_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file timer_platform.h
 */
#include "timer_interface.h"

#include <time.h>
#include <Windows.h>

/**
 * definition of the Timer struct. Platform specific
 */
struct Timer {
	struct timeval end_time;
};

#ifdef __cplusplus
}
#endif

#endif /* SRC_PROTOCOL_MQTT_AWS_IOT_EMBEDDED_CLIENT_WRAPPER_PLATFORM_LINUX_COMMON_TIMER_PLATFORM_H_ */
