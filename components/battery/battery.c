#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "battery.h"
#include "driver/gpio.h"


static adc_oneshot_unit_handle_t adc_handle;
volatile float battery_voltage = 0;

void batt_init(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BAT_ADC_CH, &chan_config));
}

// float battery_read_voltage(void) {
//     int raw = 0;
//     static float battery = 0.0f; // Initial battery voltage
//     ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, BAT_ADC_CH, &raw));
//     float adc_voltage = (raw * 3.3f) / 4095.0f;
//     float b = adc_voltage * 6.55; //((146.2f + 20.3f) / 30.0f);
//     battery = battery * 0.9f + b * 0.1f; 
//     return battery;
// }

float battery_read_voltage(void) {

    int sum = 0;
    static float battery = -1.0f;

    for (int i = 0; i < 32; i++) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, BAT_ADC_CH, &raw));
        sum += raw;
    }
    int raw = sum / 32;
    float adc_voltage = (raw * 3.3f) / 4095.0f;
    float b = adc_voltage * 6.14f; //   ((146.2f + 20.3f + 30.0f) / 30.0f);
    if (battery < 0)
        battery = b;
    else
        battery = battery * 0.9f + b * 0.1f;

    return battery;
}

void battery_task(void *arg) {
    while (1) {
        battery_voltage = battery_read_voltage();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}