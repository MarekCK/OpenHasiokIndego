#include "motor.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static const char *TAG = "OHI_MOTOR";

void motor_task(void *) {

    while (1) {
        switch (ohi_cmd)
        {
            case CMD_FORWARD:
            // ESP_LOGI(TAG, "forward");
                // motors_forward();
                break;

            case CMD_BACKWARD:
            // ESP_LOGI(TAG, "backward");
                // motors_backward();
                break;

            case CMD_LEFT:
            // ESP_LOGI(TAG, "left");
                // motors_left();
                break;

            case CMD_RIGHT:
            // ESP_LOGI(TAG, "right");
                // motors_right();
                break;

            default:
            // ESP_LOGI(TAG, "stop");
                // motors_stop();
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
