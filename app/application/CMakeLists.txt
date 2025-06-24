set(LIB_SRC 
     cmd_process.c
     app_zmcio.c
     data_log.c
     modbus_tcp_server.c
)
include_directories(${PROJECT_SOURCE_DIR}/app/include)
include_directories(${PROJECT_SOURCE_DIR}/app/application)
include_directories(${PROJECT_SOURCE_DIR}/app/utils)
include_directories(${PROJECT_SOURCE_DIR}/app/ecat_master)
include_directories(${PROJECT_SOURCE_DIR}/app/upgrade)
include_directories(${PROJECT_SOURCE_DIR}/app/database)
include_directories(${PROJECT_SOURCE_DIR}/app/lib/sqlite3/include)

set(PRJ app)
add_executable(${PRJ} main.c ${LIB_SRC})
# #链接库
target_link_libraries(${PRJ} client network modbus zecm app_pugrade utils ecat_master sqlite3 database ${SYS_LIBS})
# 设置外部库路径
target_link_directories(${PRJ} PRIVATE ${PROJECT_SOURCE_DIR}/app/lib)
target_link_directories(${PRJ} PRIVATE ${PROJECT_SOURCE_DIR}/app/lib/sqlite3/lib)

set(PRJ app_test)
add_executable(${PRJ} test.c)
target_link_directories(${PRJ} PRIVATE ${PROJECT_SOURCE_DIR}/app/lib/sqlite3/lib)
# #链接库
target_link_libraries(${PRJ} database sqlite3 ${SYS_LIBS})


