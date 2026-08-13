idf_component_register(
    SRCS
        "main.c"
        "wifi/wifi_ap.c"
        "web/webserver.c"

    INCLUDE_DIRS
        "."
        "wifi"
        "web"