if(ARM_LINUX)
include(../../cross-linux.mk)
endif()

set(LIB_SRC 
    jsmn.c jsmn_wrapper.c
)
add_library(jsmn ${LIB_SRC})
set_target_properties(jsmn PROPERTIES FOLDER "3rd/jsmn")
add_library(jsmn_wrapper ${LIB_SRC})
set_target_properties(jsmn_wrapper PROPERTIES FOLDER "3rd/jsmn")


