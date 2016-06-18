/**
  ******************************************************************************
  * @file    gpio_api.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    3-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef GPIO_API_H
#define GPIO_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "gpioNames.h"

typedef struct{
    PinName pinName;
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
}gpio_t;

/* Set the given pin as GPIO
 * @param pin The pin to be set as GPIO
 * @return The GPIO port mask for this pin
 **/
uint32_t gpio_set(PinName pin);

/* GPIO object */
void gpio_init(gpio_t *obj, PinName pin);

void gpio_mode (gpio_t *obj, PinMode mode);
void gpio_dir  (gpio_t *obj, PinDirection direction);

void gpio_write(gpio_t *obj, int value);
int  gpio_read (gpio_t *obj);

// the following set of functions are generic and are implemented in the common gpio.c file
void gpio_init_in(gpio_t* gpio, PinName pin);
void gpio_init_in_ex(gpio_t* gpio, PinName pin, PinMode mode);
void gpio_init_out(gpio_t* gpio, PinName pin);
void gpio_init_out_ex(gpio_t* gpio, PinName pin, PinMode mode, int value);
void gpio_init_inout(gpio_t* gpio, PinName pin, PinDirection direction, PinMode mode, int value);

void pin_function(PinName pin,int data);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_API_H */
