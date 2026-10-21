idf_component_register(
    SRCS "battery.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        freertos
        esp_driver_gpio
        esp_adc
)