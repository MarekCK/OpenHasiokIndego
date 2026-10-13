#include "motor.h"
#include "command.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"

static const char *TAG = "OHI_MOTOR";

volatile int8_t right_forward_procent = 100;
volatile int8_t left_forward_procent = 100;
volatile int8_t right_reverse_procent = 80;
volatile int8_t left_reverse_procent = 80;
volatile bool encoder_updated = false;

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

volatile int32_t left_ticks  = 0;
volatile int32_t right_ticks = 0;
static int32_t left_prev = 0;
static int32_t right_prev = 0;
volatile int32_t left_delta = 0;
volatile int32_t right_delta = 0;

static void IRAM_ATTR left_encoder_isr(void *arg) {
    left_ticks++;
}

static void IRAM_ATTR right_encoder_isr(void *arg) {
    right_ticks++;
}

void motors_init(void) {

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks = 1000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };

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

    gpio_config_t enc_cfg = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << LEFT_ENC_A) | (1ULL << LEFT_ENC_B) | (1ULL << RIGHT_ENC_A) | (1ULL << RIGHT_ENC_B),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&enc_cfg));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(LEFT_ENC_A, left_encoder_isr, NULL));
    // ESP_ERROR_CHECK(gpio_isr_handler_add(LEFT_ENC_B, left_encoder_isr, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(RIGHT_ENC_A, right_encoder_isr, NULL));
    // ESP_ERROR_CHECK(gpio_isr_handler_add(RIGHT_ENC_B, right_encoder_isr, NULL));
    
    ESP_LOGI(TAG, "ISR Setup done");
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

void encoder_task(void *) {

    while (1) {
        left_delta = left_ticks - left_prev;
        right_delta = right_ticks - right_prev;
        left_prev = left_ticks;
        right_prev = right_ticks;
        encoder_updated = true;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void motor_task(void *) {

    while (1) {
        switch (ohi_cmd) {
            case CMD_FORWARD:
                if (encoder_updated) {
                    encoder_updated = false;
                    if (left_delta > right_delta + 5) {
                        if (left_forward_procent > 80) 
                            left_forward_procent--;
                        else if (right_forward_procent < 100)
                            right_forward_procent++;                    
                    } else if (right_delta > left_delta + 5) {
                        if (right_forward_procent > 80) 
                            right_forward_procent--;
                        else if (left_forward_procent < 100)
                            left_forward_procent++;                     
                    }
                }
                // int32_t error = left_delta - right_delta;
                // if (error > 0) {
                //     left_forward_procent -= error / 10;
                // }
                // else {
                //     right_forward_procent += error / 10; // error ujemny
                // }
              
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, left_forward_procent);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, right_forward_procent);
            break;
            case CMD_BACKWARD:
                if (encoder_updated) {
                    encoder_updated = false;
                    if (left_delta > right_delta + 5) {
                        if (left_reverse_procent > 50)
                            left_reverse_procent--;
                        else if (left_reverse_procent < 80)
                            right_reverse_procent++;
                    } else if (right_delta > left_delta + 5) {
                        if (right_reverse_procent > 50)
                            right_reverse_procent--;
                        else if (right_reverse_procent < 80)
                            left_reverse_procent++;
                    }
                }
                // int32_t error = left_delta - right_delta;
                // if (error > 0) {
                //     left_reverse_procent -= error / 10;
                // }
                // else {
                //     right_reverse_procent += error / 10; // error ujemny
                // }

                motor_set(LEFT_MOTOR, MOTOR_REVERSE, left_reverse_procent);
                motor_set(RIGHT_MOTOR, MOTOR_REVERSE, right_reverse_procent);
            break;
            case CMD_LEFT:
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, 0);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, 70);
            break;
            case CMD_RIGHT:
                motor_set(LEFT_MOTOR, MOTOR_FORWARD, 70);
                motor_set(RIGHT_MOTOR, MOTOR_FORWARD, 0);
            break;

            default:
                motors_stop_all();
                right_forward_procent = 100;
                left_forward_procent = 100;
                right_reverse_procent = 80;
                left_reverse_procent = 80;                
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
