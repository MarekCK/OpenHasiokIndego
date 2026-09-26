idf_component_register(
    SRCS "blade.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_driver_gpio 
        freertos
        command
        led
)


