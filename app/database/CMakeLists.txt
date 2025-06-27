set(LIB_SRC 
    database.c
    test_order.c
    upload_data.c
)

include_directories(${PROJECT_SOURCE_DIR}/app/database)
include_directories(${PROJECT_SOURCE_DIR}/app/utils)
include_directories(${PROJECT_SOURCE_DIR}/app/lib/sqlite3/include)


add_library(database ${LIB_SRC})



