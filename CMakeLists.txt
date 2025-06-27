cmake_minimum_required(VERSION 3.5)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ADD_DEFINITIONS(-DZMC600E=1)

#内置对epcm3568的支持
if(EPCM3568)
#ADD_DEFINITIONS(-DARM_LINUX=1) #经验证ADD_DEFINITIONS定义的变量，然后同一个文件引用该变量是无效的
set(ARM_LINUX 1)
execute_process(COMMAND ln -svf epcm3568.mk cross-linux.mk)
endif()

#内置对em-500的支持
if(EM500)
set(ARM_LINUX 1)
execute_process(COMMAND ln -svf em-500.mk cross-linux.mk)
endif()

#内置对em-1000的支持
if(EM1000)
set(ARM_LINUX 1)
execute_process(COMMAND ln -svf em-1000.mk cross-linux.mk)
endif()

#内置对zmc-600e支持
if(ZMC600E)
set(ARM_LINUX 1)
execute_process(COMMAND ln -svf zmc-600e.mk cross-linux.mk)
endif()

if(ARM_LINUX)
include(cross-linux.mk)
endif()

project(mqttproto)
#MQTT若需要使用加密端口，需要开启宏 WITH_MQTT_SSL

ADD_DEFINITIONS(
    -DZLG_IOT_MQTT_TX_BUF_LEN=2048
    -DZLG_IOT_MQTT_RX_BUF_LEN=2048
    -DZLG_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS=20
    #http buf 的大小，用下下载安装包等。
    -DZLG_IOT_HTTP_BUFF_SIZE=1023 
    -DZLG_IOT_HTTP_COOKIE_SIZE=1024
    -DZLG_IOT_HTTP_URL_PATH_LEN=512
    -DZLG_IOT_HTTP_TOKEN_LEN=1024
    #META 相关
    -DZLG_IOT_DEV_ID_LEN=64
    -DZLG_IOT_DEV_TYPE_LEN=64
    -DZLG_IOT_DEV_SECRET_LEN=64
    #LOG 相关
    -DENABLE_IOT_INFO=true
    -DENABLE_IOT_WARN=true
    -DENABLE_IOT_ERROR=true
)

#定义USE_TLS启用TLS功能，开启后会编译3rd/mbedtls库
#set(USE_TLS  1) 

set(SYS_LIBS jsmn)
if(USE_TLS)
  list(APPEND SYS_LIBS mbedtls mbedx509 mbedcrypto)
  ADD_DEFINITIONS(
    -DWITH_MBED_TLS=true
#    -DTRANSFER_PROTOCOL="https"
    -DZLG_IOT_AUTH_PORT=443   #https port
    -DZLG_IOT_ZWS_PACKAGE_PORT=443
    -g  #add debug info 
)
else()
 ADD_DEFINITIONS(
#    -DTRANSFER_PROTOCOL="http"
    -DZLG_IOT_AUTH_PORT=80   #http port
    -DZLG_IOT_ZWS_PACKAGE_PORT=80
    -g  #add debug info 
)
endif()

if(APPLE)
	list(APPEND SYS_LIBS -lm -lpthread)
    ADD_DEFINITIONS(
        -DMACOS -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
        -Wno-deprecated -Wwrite-strings
    )
elseif(UNIX)
	list(APPEND SYS_LIBS -lm -lpthread)
    ADD_DEFINITIONS(
        -DUNIX  -fPIC 
        -Wno-unused-variable 
        -Wno-unused-parameter 
        -Wno-unused-function
        -Wno-unused-but-set-variable
        -Wno-deprecated -Wwrite-strings
    )
else()
	list(APPEND SYS_LIBS wsock32 ws2_32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -DHAVE_STRUCT_TIMESPEC)
endif()

set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(GTEST_DIR ${PROJECT_SOURCE_DIR}/3rd/googletest/googletest)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR} )

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR} )
    message(STATUS CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG})
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

link_directories(${BIN_DIR})
link_directories(${PROJECT_SOURCE_DIR}/3rd/jsmn)
link_directories(${PROJECT_SOURCE_DIR}/3rd/mbedtls/library)
link_directories(${PROJECT_SOURCE_DIR}/3rd/mbedtls/build/library)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/protocol/common)

if(WIN32)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/windows/common)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/windows/socket)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/windows/pthread)
else()
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/linux/common)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/linux/socket)
include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/platform/linux/pthread)
endif()

include_directories(${PROJECT_SOURCE_DIR}/zws_sdk/network/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/mbedtls/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/jsmn)

add_subdirectory(3rd/jsmn)
add_subdirectory(zws_sdk)
# add_subdirectory(demos)
add_subdirectory(app)
if(USE_TLS)
  add_subdirectory(3rd/mbedtls)
endif()
#add_subdirectory(others)

