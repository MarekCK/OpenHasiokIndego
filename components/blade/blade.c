
#include "blade.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"
#include "led.h"



static void blade_pwm_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
        ledc_channel_config_t channel = {
        .gpio_num = BLADE_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}


void blade_init(void)
{
    blade_pwm_init();
    blade_set(0);
}

void blade_set(uint8_t percent) {

    if (percent > 100)
        percent = 100;
    /*
    * Moduł z opto miał odwróconą logikę:
    *
    * 0% -> pełne wysterowanie
    * 100% -> wyłączony
    */
    // percent = 100 - percent;
    uint32_t duty = (1023 * percent) / 100;
    ESP_ERROR_CHECK(
        ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        duty));
    ESP_ERROR_CHECK(
        ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0));
}



void blade_task(void *) {

    uint8_t p = 0;

    while (1) {
        switch (ohi_cmd) {
            case CMD_BLADE_ON:
                if (p == 0)
                    for (p = 30; p <= 95; p += 5) {
                        blade_set(p);
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                led_set(0, 0, 10);
                break;

            case CMD_BLADE_OFF:
                blade_set(0);
                led_set(0, 10, 0);
                p = 0;
                break;

            default:
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}