#pragma once
#include "stdint.h"

#define BLADE_GPIO 10

void blade_init(void);
void blade_on(void);
void blade_off(void);
void blade_task(void *);
