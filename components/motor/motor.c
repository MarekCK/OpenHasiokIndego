#include "motor.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

static const char *TAG = "OHI_MOTOR";

volatile int8_t right_speed_procent = 0;
volatile int8_t left_speed_procent = 0;

static mcpwm_cmpr_handle_t left_r_cmp = NULL;
static mcpwm_cmpr_handle_t left_f_cmp = NULL;
static mcpwm_cmpr_handle_t right_r_cmp = NULL;
static mcpwm_cmpr_handle_t right_f_cmp = NULL;
static mcpwm_timer_handle_t timer = NULL;
static mcpwm_oper_handle_t oper_l = NULL;
static mcpwm_oper_handle_t oper_r = NULL;
static mcpwm_gen_handle_t left_r_gen = NULL;
static mcpwm_gen_handle_t left_f_gen = NULL;
static mcpwm_gen_handle_t right_r_gen = NULL;
static mcpwm_gen_handle_t right_f_gen = NULL;

void blade_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num   = BLADE_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

void blade_set(uint8_t percent)
{
    if (percent > 100)
        percent = 100;

    uint32_t duty = (1023 * percent) / 100;

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        )
    );
}

void motors_init(void) {

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks = 1000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };

    blade_init();

    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));
    mcpwm_operator_config_t operator_config = {.group_id = 0,};

    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_l));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_l, timer));
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_r));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_r, timer));

    
    // gpio_config_t io_cfg = {
    // .mode = GPIO_MODE_OUTPUT,
    // .pin_bit_mask = (1ULL << BLADE_SW_GPIO),};
    // ESP_ERROR_CHECK(gpio_config(&io_cfg));
    // ESP_ERROR_CHECK(gpio_set_level(BLADE_SW_GPIO, 0));

    mcpwm_generator_config_t gen_configlr = {.gen_gpio_num = LEFT_RPWM_GPIO,};
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_l, &gen_configlr, &left_r_gen));
    mcpwm_generator_config_t gen_configlf = {.gen_gpio_num = LEFT_FPWM_GPIO,};
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_l, &gen_configlf, &left_f_gen));

    mcpwm_generator_config_t gen_configrr = {.gen_gpio_num = RIGHT_RPWM_GPIO,};
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_r, &gen_configrr, &right_r_gen));
    mcpwm_generator_config_t gen_configrf = {.gen_gpio_num = RIGHT_FPWM_GPIO,};
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_r, &gen_configrf, &right_f_gen));

    mcpwm_comparator_config_t cmp_config = {.flags.update_cmp_on_tez = true,};
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_l, &cmp_config, &left_r_cmp));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_l, &cmp_config, &left_f_cmp));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_r, &cmp_config, &right_r_cmp));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_r, &cmp_config, &right_f_cmp));

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_r_cmp, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_f_cmp, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_r_cmp, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_f_cmp, 0));    

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(left_r_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(left_r_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_r_cmp,MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(left_f_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(left_f_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,left_f_cmp,MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(right_r_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(right_r_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,right_r_cmp,MCPWM_GEN_ACTION_LOW)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(right_f_gen,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,MCPWM_TIMER_EVENT_EMPTY,MCPWM_GEN_ACTION_HIGH)));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(right_f_gen,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,right_f_cmp,MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    ESP_LOGI(TAG, "PWM Setup done");
}

static inline uint32_t speed_to_duty(uint8_t speed) {

    if (speed > 100)
        speed = 100;
    return speed * 10;
}

void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed) {

    uint32_t duty = speed_to_duty(speed);   // 0..100 -> 0..1000

    // ESP_LOGI(TAG, "Motor=%d Dir=%d Speed=%d Duty=%lu", motor, dir, speed, duty);

    switch (motor) {
        case LEFT_MOTOR: 
            switch (dir) {
                case MOTOR_FORWARD:
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_r_cmp, 0));
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_f_cmp, duty));
                break;
                case MOTOR_REVERSE:
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_f_cmp,0));
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(left_r_cmp,duty));
                break;
                default: break;
            }
        break;
        case RIGHT_MOTOR:
            switch(dir) {
                case MOTOR_FORWARD: 
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_r_cmp, 0));
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_f_cmp, duty));
                break;
                case MOTOR_REVERSE:
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_f_cmp, 0));
                    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(right_r_cmp, duty));
                break;
                default: break;
            }            
        break;
        default: break;
    }        
}

void motor_stop(motor_side_t motor) {
    motor_set(motor, MOTOR_FORWARD, 0);
    motor_set(motor, MOTOR_REVERSE, 0);
}

void motors_stop_all(void) {
    motor_stop(LEFT_MOTOR);
    motor_stop(RIGHT_MOTOR);    
}

void motor_task(void *) {

    while (1) {
        switch (ohi_cmd) {
            case CMD_FORWARD:
                left_speed_procent = 73;
                right_speed_procent = 70;
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, right_speed_procent);
            break;

            case CMD_BACKWARD:
                left_speed_procent = 71;
                right_speed_procent = 70;
                motor_set(LEFT_MOTOR, MOTOR_REVERSE, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_REVERSE, right_speed_procent);
            break;
            case CMD_LEFT:
                left_speed_procent = 0;
                right_speed_procent = 50;
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, right_speed_procent);

            break;
            case CMD_RIGHT:
                left_speed_procent = 50;
                right_speed_procent = 0;
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, left_speed_procent);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, right_speed_procent);
            break;
            case CMD_BLADE_ON:
                blade_set(50);
            break;
            case CMD_BLADE_OFF:
                blade_set(0);
            break;
            default:
                motors_stop_all();
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
