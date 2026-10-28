idf_component_register(
    SRCS "battery.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        freertos
        esp_driver_gpio
        esp_adc
        esp_wifi
        esp_event
        esp_netif
)