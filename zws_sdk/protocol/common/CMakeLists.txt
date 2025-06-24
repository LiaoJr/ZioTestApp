set(LIB_SRC 
    client.c
    params.c
    mqtt_client.c
)
add_library(client ${LIB_SRC})
set_target_properties(client PROPERTIES FOLDER "zws_sdk")

