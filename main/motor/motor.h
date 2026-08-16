#pragma once

#include <stdint.h>
#define LEFT_EN_GPIO   GPIO_NUM_8
#define RIGHT_EN_GPIO  GPIO_NUM_9

extern volatile int8_t right_speed;
extern volatile int8_t left_speed;

#define LEFT_RPWM_GPIO   4
#define LEFT_LPWM_GPIO   5
#define RIGHT_RPWM_GPIO  6
#define RIGHT_LPWM_GPIO  7

typedef enum {
    LEFT_MOTOR = 0,
    RIGHT_MOTOR
} motor_side_t;

typedef enum {
    MOTOR_FORWARD = 0,
    MOTOR_REVERSE
} motor_dir_t;

void motors_init(void);
void motors_enable(void);
void motors_disable(void);

void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed);

void motor_stop(motor_side_t motor);
void motors_stop_all(void);

void motor_task(void *pvParameter);