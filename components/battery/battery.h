#pragma once

// #define BAT_ADC_GPIO GPIO_NUM_16
#define BAT_ADC_CH   ADC_CHANNEL_5
extern volatile float battery_voltage;

void batt_init(void);
void battery_task(void *);