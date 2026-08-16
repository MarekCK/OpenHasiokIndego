#include "motor.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

static const char *TAG = "OHI_MOTOR";
static int8_t right_speed_procent = 0;
static int8_t left_speed_procent = 0;

static mcpwm_cmpr_handle_t left_r_cmp;
static mcpwm_cmpr_handle_t left_l_cmp;
static mcpwm_cmpr_handle_t right_r_cmp;
static mcpwm_cmpr_handle_t right_l_cmp;

void motors_init(void)
{
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_oper_handle_t oper = NULL;

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks = 1000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));
    mcpwm_operator_config_t operator_config = {.group_id = 0,};
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    /* tutaj później:
       generator LEFT_RPWM
       generator LEFT_LPWM
       generator RIGHT_RPWM
       generator RIGHT_LPWM
    */

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer,MCPWM_TIMER_START_NO_STOP));
}


void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed) {

    printf("Motor:%d, dir:%d, speed:%d\n", motor, dir, speed);
}

void motor_stop(motor_side_t motor) {
    motor_set(motor, MOTOR_FORWARD, 0);
}

void motors_stop_all(void) {
    motor_stop(LEFT_MOTOR);
    motor_stop(RIGHT_MOTOR);    
}

void motor_task(void *) {

    while (1) {
        switch (ohi_cmd)
        {
            case CMD_FORWARD:
            // ESP_LOGI(TAG, "forward");
            left_speed_procent = 50;
            right_speed_procent = 50;
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, right_speed_procent);
            break;

            case CMD_BACKWARD:
                left_speed_procent = 50;
                right_speed_procent = 50;
                motor_set(LEFT_MOTOR, MOTOR_REVERSE, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_REVERSE, right_speed_procent);
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
                motors_stop_all();
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}
