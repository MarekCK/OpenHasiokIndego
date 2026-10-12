#pragma once
#include "stdint.h"

#define BLADE_PWM_GPIO 10


#include <stdint.h>

void blade_init(void);
void blade_set(uint8_t percent);
void blade_task(void *arg);
