#ifndef TEMPERATURE_HH
#define TEMPERATURE_HH

#include <stdint.h>
#include "Configuration.h"

uint16_t getTemperatureRawValue(uint8_t index);

void TemperatureManageInit(void);
void hw_TemperatureManageInit(void);

void setPWMHeaterValue(uint8_t index, uint8_t value);
void setPWMHeaterOn(uint8_t index);

void Temperature_DMA_IRQHandler(void);
void Temperature_Time_IRQHandler(void);

void SysTick_Temperature(void);
#endif /* TEMPERATURE_HH */
