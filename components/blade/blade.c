
#include "blade.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "led.h"

#include "driver/gpio.h"


void blade_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BLADE_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(BLADE_GPIO, 1); // kosa OFF
}

void blade_on(void) {
    gpio_set_level(BLADE_GPIO, 0);
}

void blade_off(void) {
    gpio_set_level(BLADE_GPIO, 1);
}     

void blade_task(void *) {
    while (1)
    {
        switch (ohi_cmd) {
            case CMD_BLADE_ON:
                blade_on();
                led_set(0, 0, 10);
                break;

            case CMD_BLADE_OFF:
                blade_off();
                led_set(0, 10, 0);
                break;

            default:
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}