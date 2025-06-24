set(LIB_SRC 
    ecat_master.c
    eeprom.c
    slave.c
)
include_directories(${PROJECT_SOURCE_DIR}/app/include)
include_directories(${PROJECT_SOURCE_DIR}/app/application)
include_directories(${PROJECT_SOURCE_DIR}/app/utils)
add_library(ecat_master ${LIB_SRC})

