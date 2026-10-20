#pragma once

#include <stdint.h>


extern volatile int32_t right_ticks;
extern volatile int32_t left_ticks;

extern volatile int32_t left_delta;
extern volatile int32_t right_delta;

extern volatile int32_t left_delta_c;
extern volatile int32_t right_delta_c;

extern volatile int8_t right_forward_procent;
extern volatile int8_t left_forward_procent;

#define LEFT_RPWM_GPIO   5
#define LEFT_FPWM_GPIO   4
#define RIGHT_RPWM_GPIO  6
#define RIGHT_FPWM_GPIO  7

#define LEFT_ENC_A   GPIO_NUM_12
#define LEFT_ENC_B   GPIO_NUM_11
#define RIGHT_ENC_A  GPIO_NUM_13
#define RIGHT_ENC_B  GPIO_NUM_14

typedef enum {
    LEFT_MOTOR = 0,
    RIGHT_MOTOR
} motor_side_t;

typedef enum {
    MOTOR_FORWARD = 0,
    MOTOR_REVERSE
} motor_dir_t;

typedef enum {
    OFF = 0,
    ON
} blade_on_off;

void blade (blade_on_off state);
void motors_init(void);
void motors_enable(void);
void motors_disable(void);

void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed);

void motor_stop(motor_side_t motor);
void motors_stop_all(void);

void motor_task(void *pvParameter);
void encoder_task(void *pvParameter);
void encoder_gpio_test_task(void *arg);
