INCLUDEPATH  += $$PWD \
    $$PWD/3rd/jsmn/ \
    $$PWD/zws_sdk/protocol/common \
    $$PWD/zws_sdk/network/include

contains(QMAKE_HOST.os, Windows) {
INCLUDEPATH += $$PWD/zws_sdk/network/platform/windows/common \
    $$PWD/zws_sdk/network/platform/windows/socket \
    $$PWD/zws_sdk/network/platform/windows/pthread
} else {
INCLUDEPATH += $$PWD/zws_sdk/network/platform/linux/common \
    $$PWD/zws_sdk/network/platform/linux/socket \
    $$PWD/zws_sdk/network/platform/linux/pthread
}

# 3rd/jsmn
HEADERS += \
    $$PWD/3rd/jsmn/jsmn.h \
    $$PWD/3rd/jsmn/jsmn_wrapper.h

SOURCES += \
    $$PWD/3rd/jsmn/jsmn.c \
    $$PWD/3rd/jsmn/jsmn_wrapper.c

# zws_sdk/network
HEADERS += \
    $$PWD/zws_sdk/network/include/network_interface.h \
    $$PWD/zws_sdk/network/include/threads_interface.h \
    $$PWD/zws_sdk/network/include/timer_interface.h \
    $$PWD/zws_sdk/network/include/zlg_iot_config.h \
    $$PWD/zws_sdk/network/include/zlg_iot_error.h \
    $$PWD/zws_sdk/network/include/zlg_iot_http_auth_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_http_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_http_firmware_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_http_package_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_http_upload_file_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_log.h \
    $$PWD/zws_sdk/network/include/zlg_iot_mqtt_client_common_internal.h \
    $$PWD/zws_sdk/network/include/zlg_iot_mqtt_client.h \
    $$PWD/zws_sdk/network/include/zlg_iot_mqtt_client_interface.h \
    $$PWD/zws_sdk/network/include/zlg_iot_version.h

SOURCES += \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_subscribe.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_common_internal.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_unsubscribe.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_connect.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_yield.c \
    $$PWD/zws_sdk/network/src/zlg_iot_mqtt_client_publish.c \
    $$PWD/zws_sdk/network/src/zlg_iot_http_client.c \
    $$PWD/zws_sdk/network/src/zlg_iot_http_auth_client.c \
    $$PWD/zws_sdk/network/src/zlg_iot_http_package_client.c \
    $$PWD/zws_sdk/network/src/zlg_iot_http_firmware_client.c \
    $$PWD/zws_sdk/network/src/zlg_iot_http_upload_file_client.c

contains(QMAKE_HOST.os, Windows) {
HEADERS += \
    $$PWD/zws_sdk/network/platform/windows/common/timer_platform.h \
    $$PWD/zws_sdk/network/platform/windows/common/win_timer_util.h \
    $$PWD/zws_sdk/network/platform/windows/mbedtls/network_platform.h \
    $$PWD/zws_sdk/network/platform/windows/socket/network_platform.h

SOURCES += \
    $$PWD/zws_sdk/network/platform/windows/common/timer.c \
    $$PWD/zws_sdk/network/platform/windows/mbedtls/network_mbedtls_wrapper.c \
    $$PWD/zws_sdk/network/platform/windows/socket/network_socket_wrapper.c
} else {
HEADERS += \
    $$PWD/zws_sdk/network/platform/linux/common/timer_platform.h \
    $$PWD/zws_sdk/network/platform/linux/mbedtls/network_platform.h \
    $$PWD/zws_sdk/network/platform/linux/socket/network_platform.h

SOURCES += \
    $$PWD/zws_sdk/network/platform/linux/common/timer.c \
    $$PWD/zws_sdk/network/platform/linux/mbedtls/network_mbedtls_wrapper.c \
    $$PWD/zws_sdk/network/platform/linux/socket/network_socket_wrapper.c
}

# zws_sdk/protocol/common
HEADERS += \
    $$PWD/zws_sdk/protocol/common/command.h \
    $$PWD/zws_sdk/protocol/common/settings.h \
    $$PWD/zws_sdk/protocol/common/types_def.h \
    $$PWD/zws_sdk/protocol/common/client.h \
    $$PWD/zws_sdk/protocol/common/params.c \
    $$PWD/zws_sdk/protocol/common/mqtt_client.c

SOURCES += \
    $$PWD/zws_sdk/protocol/common/client.c \
    $$PWD/zws_sdk/protocol/common/params.c \
    $$PWD/zws_sdk/protocol/common/mqtt_client.c

# Lib
contains(QMAKE_HOST.os, Windows) {
INCLUDEPATH += $$PWD/zws_sdk/network/platform/windows/common \
    LIBS += -lWs2_32
    LIBS += -lwsock32
}

