idf_component_register(
    SRCS "blade.c"
    INCLUDE_DIRS "."
    REQUIRES
        freertos
        command
        led
        battery
        esp_driver_ledc
)


