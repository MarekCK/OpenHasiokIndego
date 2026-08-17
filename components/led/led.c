#include "led.h"
#include "driver/gpio.h"
#include "led_strip.h"

static led_strip_handle_t led_strip;

void led_init(void) {
    
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = 48,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void led_set(uint8_t r, uint8_t g, uint8_t b) {

    if (r != 0 || g != 0 || b != 0) {
            led_strip_set_pixel(led_strip, 0, r, g, b); 
            /* Refresh the strip to send data */
            led_strip_refresh(led_strip);
    } 
    else {
            /* Set all LED off to clear all pixels */
            led_strip_clear(led_strip);
    }
}

