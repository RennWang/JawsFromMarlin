#include "hw_gpio.h"

GPIO_TypeDef* const GPIO_ADDR[8] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, 0, 0, 0};

void SET_OUTPUT(PinName pin)
{
    if (pin == (PinName)NC)
        return;

    uint32_t port_index = STM_PORT(pin);
    uint32_t pin_index = STM_PIN(pin);
    Set_GPIO_Clock((PortName)port_index);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin       = (uint32_t)(1 << pin_index);
    GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIO_ADDR[port_index], &GPIO_InitStructure);
}

void SET_INPUT(PinName pin)
{
    if (pin == (PinName)NC)
        return;

    uint32_t port_index = STM_PORT(pin);
    uint32_t pin_index = STM_PIN(pin);
    Set_GPIO_Clock((PortName)port_index);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin       = (uint32_t)(1 << pin_index);
    GPIO_InitStructure.Mode      = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIO_ADDR[port_index], &GPIO_InitStructure);
}

void WRITE(PinName pin, bool value)
{
    __NOP();
    __NOP();
    if (value)
    {
        GPIO_ADDR[STM_PORT(pin)]->BSRR = (uint32_t)(1 << STM_PIN(pin));
    }
    else
    {
        GPIO_ADDR[STM_PORT(pin)]->BSRR = (uint32_t)(0x00010000 << STM_PIN(pin));
    }
}

bool READ(PinName pin)
{
    return ((GPIO_ADDR[STM_PORT(pin)]->IDR & (1 << STM_PIN(pin))) != (uint32_t)GPIO_PIN_RESET)? true : false;
}

void Set_GPIO_Clock(PortName port)
{
    if (port == PortA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == PortB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == PortC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == PortD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == PortE)
        __HAL_RCC_GPIOE_CLK_ENABLE();
}


