set(shared_src 
    commands.c  
    device.c    
    errors.c    
    settings.c  
    warnings.c
)

set(PRJ device_example)
add_executable(${PRJ} "")
target_sources(${PRJ} PRIVATE device_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ gateway_example)
add_executable(${PRJ} gateway_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ appuser_example)
add_executable(${PRJ} appuser_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ device_example_Industry_ES)
add_executable(${PRJ} "")
target_sources(${PRJ} PRIVATE Industry_ES/device_expl_Industry_ES.c Industry_ES/commands_Industry_ES.c Industry_ES/device_Industry_ES.c errors.c settings.c warnings.c)
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ device_example_Home_ES)
add_executable(${PRJ} "")
target_sources(${PRJ} PRIVATE Home_ES/device_expl_Home_ES.c commands.c Home_ES/device_Home_ES.c errors.c settings.c warnings.c)
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ download_firmware_example)
add_executable(${PRJ} download_firmware_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ download_package_example)
add_executable(${PRJ} download_package_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ upload_file_example)
add_executable(${PRJ} upload_file_expl.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ http_transfer_chunk)
add_executable(${PRJ} http_transfer_chunk.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})

set(PRJ test)
add_executable(${PRJ} test.c ${shared_src})
set_target_properties(${PRJ} PROPERTIES FOLDER "demos")
target_link_libraries(${PRJ} client network ${SYS_LIBS})