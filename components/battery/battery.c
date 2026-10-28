#include "battery.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_now.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#define TAG "OHI_BATTERY"

uint8_t joystick_mac[] = {0x7C, 0x4F, 0xAD, 0xBB, 0xD6, 0x81};  //TODO: Change to your joystick MAC address

typedef struct {
    int32_t battery;
    int32_t left_delta;
    int32_t right_delta;
} ohi_status_t;

ohi_status_t status;
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
    
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, joystick_mac, 6);
    peer.channel = 1;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));    
    ESP_LOGI(TAG, "peer added");
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
        status.battery = battery_voltage * 100;
        status.left_delta = 0;
        status.right_delta = 0;  
        esp_now_send(joystick_mac, (uint8_t *)&status, sizeof(status));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}