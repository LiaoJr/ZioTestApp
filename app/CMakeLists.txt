# set(PRJ app)
# add_executable(${PRJ} "")
# # 添加源码
# target_sources(${PRJ} PRIVATE application/main.c application/cmd_process.c application/app_zmcio.c application/data_log.c 
# application/modbus_tcp_server.c
# )

# set_target_properties(${PRJ} PROPERTIES FOLDER "app")
# include_directories(${PROJECT_SOURCE_DIR}/app/include)
# include_directories(${PROJECT_SOURCE_DIR}/app/application)
# include_directories(${PROJECT_SOURCE_DIR}/app/ecat_master)
# include_directories(${PROJECT_SOURCE_DIR}/app/upgrade)
# include_directories(${PROJECT_SOURCE_DIR}/app/utils)



# # 设置库路径
# target_link_directories(${PRJ} PRIVATE ${PROJECT_SOURCE_DIR}/app/lib)
# #链接库
# target_link_libraries(${PRJ} client network modbus zecm ${SYS_LIBS})

add_subdirectory(upgrade)
add_subdirectory(utils)
add_subdirectory(ecat_master)
add_subdirectory(application)
add_subdirectory(database)