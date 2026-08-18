#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "wifi_ap.h"
#include "webserver.h"
#include "led.h"
#include "ota.h"
#include "command.h"
#include "motor.h"


static const char *TAG = "OHI_MAIN_APP";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Starting OpenHasiokIndego");
    led_init();
    led_set(10, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ohi_wifi_init();
    ohi_webserver_start();
    motors_init();
    xTaskCreate(motor_task, "motor_task", 1024 * 2, NULL, 5, NULL);
    led_set(0, 10, 0); // Set LED  to indicate successful initialization
    vTaskDelete(NULL);
    // while (1)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}