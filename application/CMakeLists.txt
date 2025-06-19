# add_subdirectory(../ecat_master)
# add_subdirectory(../upgrade)
# add_subdirectory(../utils)


add_library(main OBJECT main.c)
add_library(app_zmcio OBJECT app_zmcio.c)
add_library(cmd_process OBJECT cmd_process.c)
add_library(data_log OBJECT data_log.c)
add_library(modbus_tcp_server OBJECT modbus_tcp_server.c)

include_directories(../ecat_master ../include ../upgrade ../utils)
