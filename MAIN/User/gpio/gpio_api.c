/**
  ******************************************************************************
  * @file    gpio_api.c
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    3-April-2015
  * @brief
  *
  ******************************************************************************
  */

#include "gpio_api.h"

static const uint32_t gpiomode[13] = {
    GPIO_MODE_INPUT,                //  0 = GPIO_MODE_INPUT
    GPIO_MODE_OUTPUT_PP,            //  1 = GPIO_MODE_OUTPUT_PP
    GPIO_MODE_OUTPUT_OD,            //  2 = GPIO_MODE_OUTPUT_OD
    GPIO_MODE_AF_PP,                //  3 = GPIO_MODE_AF_PP
    GPIO_MODE_AF_OD,                //  4 = GPIO_MODE_AF_OD
    GPIO_MODE_ANALOG,               //  5 = GPIO_MODE_ANALOG
    GPIO_MODE_IT_RISING,            //  6 = GPIO_MODE_IT_RISING
    GPIO_MODE_IT_FALLING,           //  7 = GPIO_MODE_IT_FALLING
    GPIO_MODE_IT_RISING_FALLING,    //  8 = GPIO_MODE_IT_RISING_FALLING
    GPIO_MODE_EVT_RISING,           //  9 = GPIO_MODE_EVT_RISING
    GPIO_MODE_EVT_FALLING,          // 10 = GPIO_MODE_EVT_FALLING
    GPIO_MODE_EVT_RISING_FALLING,   // 11 = GPIO_MODE_EVT_RISING_FALLING
    0x10000000                      // 12 = Reset GPIO_MODE_IT_EVT
};

uint32_t Set_GPIO_Clock(uint32_t port_idx) {
    uint32_t gpio_add = 0;
    switch (port_idx) {
        case PortA:
            gpio_add = (uint32_t)GPIOA;
            __GPIOA_CLK_ENABLE();
            break;
        case PortB:
            gpio_add = (uint32_t)GPIOB;
            __GPIOB_CLK_ENABLE();
            break;
        case PortC:
            gpio_add = (uint32_t)GPIOC;
            __GPIOC_CLK_ENABLE();
            break;
        case PortD:
            gpio_add = (uint32_t)GPIOD;
            __GPIOD_CLK_ENABLE();
            break;
        case PortE:
            gpio_add = (uint32_t)GPIOE;
            __GPIOE_CLK_ENABLE();
            break;
        case PortF:
            gpio_add = (uint32_t)GPIOF;
            __GPIOF_CLK_ENABLE();
            break;
        case PortG:
            gpio_add = (uint32_t)GPIOG;
            __GPIOG_CLK_ENABLE();
            break;
        case PortH:
            gpio_add = (uint32_t)GPIOH;
            __GPIOH_CLK_ENABLE();
            break;
        case PortI:
            gpio_add = (uint32_t)GPIOI;
            __GPIOI_CLK_ENABLE();
            break;
        default:
//            error("Pinmap error: wrong port number.");
            break;
    }
    return gpio_add;
}

void pin_function(PinName pin,int data)
{
    if (pin == (PinName)NC)
        return;
    // Get the pin informations
    uint32_t mode  = STM_PIN_MODE(data);
    uint32_t pupd  = STM_PIN_PUPD(data);
    uint32_t afnum = STM_PIN_AFNUM(data);

    uint32_t port_index = STM_PORT(pin);
    uint32_t pin_index  = STM_PIN(pin);

    // Enable GPIO clock
    uint32_t gpio_add = Set_GPIO_Clock(port_index);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)gpio_add;

    // Configure GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin       = (uint32_t)(1 << pin_index);
    GPIO_InitStructure.Mode      = gpiomode[mode];
    GPIO_InitStructure.Pull      = pupd;
    GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Alternate = afnum;
    HAL_GPIO_Init(gpio, &GPIO_InitStructure);
}

void pin_mode(PinName pin, PinMode mode) {
    if (pin == (PinName)NC)
        return;

    uint32_t port_index = STM_PORT(pin);
    uint32_t pin_index  = STM_PIN(pin);

    // Enable GPIO clock
    uint32_t gpio_add = Set_GPIO_Clock(port_index);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)gpio_add;

    // Configure pull-up/pull-down resistors
    uint32_t pupd = (uint32_t)mode;
    if (pupd > 2)
        pupd = 0; // Open-drain = No pull-up/No pull-down

    gpio->PUPDR &= (uint32_t)(~(GPIO_PUPDR_PUPDR0 << (pin_index * 2)));
    gpio->PUPDR |= (uint32_t)(pupd << (pin_index * 2));

}

uint32_t gpio_set(PinName pin)
{
    pin_function(pin, STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, 0));
    return (uint32_t)(1 << ((uint32_t)pin & 0xF)); // Return the pin mask
}

void gpio_init(gpio_t *obj, PinName pin)
{
    if (pin == (PinName)NC)
        return;
    uint32_t port_index = STM_PORT(pin);

    // Enable GPIO clock
    uint32_t gpio_add = Set_GPIO_Clock(port_index);
    obj->pinName    = pin;
    obj->GPIOx      = (GPIO_TypeDef *)gpio_add;
    obj->GPIO_Pin   = (uint16_t)(1 << STM_PIN(pin));

    pin_function(pin,STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, 0));
}

void gpio_mode(gpio_t *obj, PinMode mode)
{
    pin_mode(obj->pinName, mode);
}

void gpio_dir(gpio_t *obj, PinDirection direction)
{
    if (direction == PIN_OUTPUT) {
        pin_function(obj->pinName, STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0));
    } else { // PIN_INPUT
        pin_function(obj->pinName, STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, 0));
    }
}

void gpio_write(gpio_t *obj, int value)
{
    HAL_GPIO_WritePin(obj->GPIOx, obj->GPIO_Pin, (value?GPIO_PIN_SET:GPIO_PIN_RESET));
}

int  gpio_read(gpio_t *obj)
{
    GPIO_PinState pinValue = HAL_GPIO_ReadPin(obj->GPIOx,obj->GPIO_Pin);
    return ((pinValue)?1:0);
}

void gpio_init_in(gpio_t* gpio, PinName pin)
{
    gpio_init_in_ex(gpio, pin, PullDefault);
}

void gpio_init_in_ex(gpio_t* gpio, PinName pin, PinMode mode)
{
    gpio_init(gpio, pin);
    if (pin != NC) {
        gpio_dir(gpio, PIN_INPUT);
        gpio_mode(gpio, mode);
    }
}

void gpio_init_out(gpio_t* gpio, PinName pin)
{
    gpio_init_out_ex(gpio, pin, PullNone, 0);
}

void gpio_init_out_ex(gpio_t* gpio, PinName pin, PinMode mode, int value)
{
    gpio_init(gpio, pin);
    if (pin != NC) {
        gpio_write(gpio, value);
        gpio_dir(gpio, PIN_OUTPUT);
        gpio_mode(gpio, mode);
    }
}

void gpio_init_inout(gpio_t* gpio, PinName pin, PinDirection direction, PinMode mode, int value)
{
    if (direction == PIN_INPUT) {
        gpio_init_in_ex(gpio, pin, mode);
        if (pin != NC)
            gpio_write(gpio, value); // we prepare the value in case it is switched later
    } else {
        gpio_init_out_ex(gpio, pin, mode, value);
    }
}
