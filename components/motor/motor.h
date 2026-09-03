#pragma once

#include <stdint.h>

#define BLADE_GPIO  GPIO_NUM_10

extern volatile int8_t right_speed;
extern volatile int8_t left_speed;

#define LEFT_RPWM_GPIO   5
#define LEFT_FPWM_GPIO   4
#define RIGHT_RPWM_GPIO  6
#define RIGHT_FPWM_GPIO  7

typedef enum {
    LEFT_MOTOR = 0,
    RIGHT_MOTOR
} motor_side_t;

typedef enum {
    MOTOR_FORWARD = 0,
    MOTOR_REVERSE
} motor_dir_t;


void blade_init (void);
void blade_set(uint8_t percent);
void motors_init(void);
void motors_enable(void);
void motors_disable(void);

void motor_set(motor_side_t motor, motor_dir_t dir, uint8_t speed);

void motor_stop(motor_side_t motor);
void motors_stop_all(void);

void motor_task(void *pvParameter);