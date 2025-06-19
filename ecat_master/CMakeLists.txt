# add_subdirectory(../application)
# add_subdirectory(../upgrade)
# add_subdirectory(../utils)

add_library(ecat_master OBJECT ecat_master.c)
add_library(eeprom OBJECT eeprom.c)
add_library(slave OBJECT slave.c)

include_directories(../application ../include ../upgrade ../utils)
