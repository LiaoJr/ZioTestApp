set(LIB_SRC 
    log.c
    ticks.c
)
include_directories(${PROJECT_SOURCE_DIR}/app/include)
add_library(utils ${LIB_SRC})

