#include "motor.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"

static const char *TAG = "OHI_MOTOR";

volatile int8_t right_speed_procent = 0;
volatile int8_t left_speed_procent = 0;

static mcpwm_cmpr_handle_t left_r_cmp = NULL;
static mcpwm_cmpr_handle_t left_l_cmp = NULL;
static mcpwm_cmpr_handle_t right_r_cmp = NULL;
static mcpwm_cmpr_handle_t right_l_cmp = NULL;
static mcpwm_timer_handle_t timer = NULL;
static mcpwm_oper_handle_t oper = NULL;
mcpwm_gen_handle_t left_r_gen = NULL;
mcpwm_gen_handle_t left_l_gen = NULL;
// mcpwm_gen_handle_t right_r_gen = NULL;
// mcpwm_gen_handle_t right_l_gen = NULL;



void motors_init(void) {


    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks = 1000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };

    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    // gpio_config_t io_cfg = {
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask =
    //         (1ULL << LEFT_EN_GPIO) |
    //         (1ULL << RIGHT_EN_GPIO)
    // };

    // ESP_ERROR_CHECK(gpio_config(&io_cfg));

    // motors_disable();

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(
        mcpwm_timer_start_stop(
            timer,
            MCPWM_TIMER_START_NO_STOP));

    mcpwm_gen_handle_t left_r_gen = NULL;


mcpwm_generator_config_t gen_configlr = {.gen_gpio_num = LEFT_RPWM_GPIO,};
ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_configlr, &left_r_gen));
mcpwm_generator_config_t gen_configll = {.gen_gpio_num = LEFT_LPWM_GPIO,};
ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_configll, &left_l_gen));

// mcpwm_generator_config_t gen_configrr = {.gen_gpio_num = RIGHT_RPWM_GPIO,};
// ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_configrr, &right_r_gen));
// mcpwm_generator_config_t gen_configrl = {.gen_gpio_num = RIGHT_LPWM_GPIO,};
// ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_configrl, &right_l_gen));

mcpwm_comparator_config_t cmp_config = {.flags.update_cmp_on_tez = true,};
ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_config, &left_r_cmp));
ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_config, &left_l_cmp));

// ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_config, &right_r_cmp));
// ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_config, &right_l_cmp));

ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(left_r_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(left_r_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_r_cmp,MCPWM_GEN_ACTION_LOW)));
ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(left_l_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(left_l_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_l_cmp,MCPWM_GEN_ACTION_LOW)));


// ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(right_r_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
// ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(right_r_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_r_cmp,MCPWM_GEN_ACTION_LOW)));
// ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(right_l_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
// ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(right_l_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_l_cmp,MCPWM_GEN_ACTION_LOW)));

}

void motors_enable(void) {
    // gpio_set_level(LEFT_EN_GPIO, 1);
    // gpio_set_level(RIGHT_EN_GPIO, 1);
}

void motors_disable(void) {
    // gpio_set_level(LEFT_EN_GPIO, 1);
    // gpio_set_level(RIGHT_EN_GPIO, 0);
}

void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed) {

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_r_cmp, 500));
    
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
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
