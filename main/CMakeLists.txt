idf_component_register(
    SRCS
        "main.c"
        "wifi/wifi_ap.c"
        "web/webserver.c"
        "led/led.c"
        "ota/ota.c"

    INCLUDE_DIRS
        "."
        "wifi"
        "web"
        "led"
        "ota"
)