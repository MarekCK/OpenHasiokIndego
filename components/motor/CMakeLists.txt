idf_component_register(
    SRCS "motor.c"
    INCLUDE_DIRS "."
    REQUIRES
        driver
        esp_driver_mcpwm
        esp_driver_gpio        
        freertos
        command
)


