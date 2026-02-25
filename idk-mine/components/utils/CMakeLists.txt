idf_component_register(
    SRCS "config_loader.c" "json_helpers.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_timer
)
