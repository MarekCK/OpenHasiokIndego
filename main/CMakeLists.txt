file(GLOB_RECURSE app_sources
    "*.c"
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS
        "."
        "wifi"
        "web"
        "led"
        "ota"
        "command"
        "motor"
)