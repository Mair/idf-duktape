
idf_component_register(
    SRCS 
        main.c 
        ductape/duktape.c 
        mqtt_task.c
        execute_js.c
    INCLUDE_DIRS 
        .
        ductape/.
      REQUIRES
        nvs_flash
        fatfs
        json
        partition_table
        esp_http_server
        esp_http_client
        app_update
        esp_https_ota
        mqtt
        spiffs
        protocol_examples_common
     )


set_source_files_properties(ductape/duktape.c PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized)
spiffs_create_partition_image(storage ../embedded_js FLASH_IN_PROJECT)

