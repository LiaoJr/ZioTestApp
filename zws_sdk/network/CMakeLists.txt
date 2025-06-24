set(LIB_SRC 
    src/zlg_iot_mqtt_client.c
    src/zlg_iot_mqtt_client_subscribe.c
    src/zlg_iot_mqtt_client_common_internal.c
    src/zlg_iot_mqtt_client_unsubscribe.c
    src/zlg_iot_mqtt_client_connect.c
    src/zlg_iot_mqtt_client_yield.c
    src/zlg_iot_mqtt_client_publish.c
    src/zlg_iot_http_client.c
    src/zlg_iot_http_auth_client.c
    src/zlg_iot_http_package_client.c
    src/zlg_iot_http_firmware_client.c
    src/zlg_iot_http_upload_file_client.c
)

if(WIN32)
list(APPEND LIB_SRC 
    platform/windows/common/timer.c
    platform/windows/mbedtls/network_mbedtls_wrapper.c
    platform/windows/socket/network_socket_wrapper.c
)
else()
list(APPEND LIB_SRC 
    platform/linux/common/timer.c
    platform/linux/mbedtls/network_mbedtls_wrapper.c
    platform/linux/socket/network_socket_wrapper.c
)
endif()

ADD_DEFINITIONS(
    -DENABLE_IOT_INFO=true
    -DENABLE_IOT_ERROR=true
)

add_library(network ${LIB_SRC})
set_target_properties(network PROPERTIES FOLDER "zws_sdk")

if(NOT WIN32)
    set(ECHO_CLIENT_SRC tools/echo_client.c)
    add_executable(echoc ${ECHO_CLIENT_SRC})
    target_link_libraries(echoc network ${SYS_LIBS}) 

    set(MQTT_DEMO_SRC tools/mqtt_demo.c)
    add_executable(mqtt_demo ${MQTT_DEMO_SRC})
    target_link_libraries(mqtt_demo network ${SYS_LIBS}) 

    set(HTTP_DOWNLOAD_SRC tools/http_download.c)
    add_executable(http_download ${HTTP_DOWNLOAD_SRC})
    target_link_libraries(http_download network ${SYS_LIBS}) 

    set(HTTP_AUTH_SRC tools/http_auth.c)
    add_executable(http_auth ${HTTP_AUTH_SRC})
    target_link_libraries(http_auth network ${SYS_LIBS})
endif()
