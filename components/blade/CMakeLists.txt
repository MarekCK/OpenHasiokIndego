idf_component_register(
    SRCS "blade.c"
    INCLUDE_DIRS "."
    REQUIRES
        freertos
        command
        led
        esp_driver_ledc
)


