#include "Fan.h"

static TIM_HandleTypeDef  TIM_PWM_HandleStruct;

Fan::Fan(uint8_t index):
    fan_speed(0),
    fan_index(index),
    fan_init(0)
{
}

void Fan::init()
{
    GPIO_InitTypeDef    GPIO_InitStruct;
    TIM_OC_InitTypeDef  TIM_InitStruct;

    if (fan_index == 0){
        __HAL_RCC_GPIOC_CLK_ENABLE();

        __HAL_RCC_AFIO_CLK_ENABLE();
        AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_FULLREMAP;

        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;             ///内部不做上下拉电阻
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        ///PWM时钟配置
        __HAL_RCC_TIM3_CLK_ENABLE();
        TIM_PWM_HandleStruct.Instance = TIM3;
        TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock / 2000000) - 1;
        TIM_PWM_HandleStruct.Init.Period = 65534;         ///总共255
        TIM_PWM_HandleStruct.Init.ClockDivision = 0;
        TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
        HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);

        TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
        TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
        TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
        TIM_InitStruct.Pulse = 0;

        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_1);

//        __HAL_RCC_GPIOC_CLK_ENABLE();
//
//        GPIO_InitStruct.Pin = GPIO_PIN_8;
//        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;             ///内部不做上下拉电阻
//        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
//        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//
//
//        ///PWM时钟配置
//        __HAL_RCC_TIM3_CLK_ENABLE();
//        TIM_PWM_HandleStruct.Instance = TIM3;
//        TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock / 2000000) - 1;
//        TIM_PWM_HandleStruct.Init.Period = 65534;         ///总共255
//        TIM_PWM_HandleStruct.Init.ClockDivision = 0;
//        TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
//        HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);
//
//        TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
//        TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
//        TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
//        TIM_InitStruct.Pulse = 0;
//
//        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_3);
    }
    else if (fan_index == 1){
        __HAL_RCC_GPIOC_CLK_ENABLE();

        __HAL_RCC_AFIO_CLK_ENABLE();
        AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_FULLREMAP;

        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;             ///内部不做上下拉电阻
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        ///PWM时钟配置
        __HAL_RCC_TIM3_CLK_ENABLE();
        TIM_PWM_HandleStruct.Instance = TIM3;
        TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock /2/2000000) - 1;
        TIM_PWM_HandleStruct.Init.Period = 65534;         ///总共255
        TIM_PWM_HandleStruct.Init.ClockDivision = 0;
        TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
        HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);

        TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
        TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
        TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
        TIM_InitStruct.Pulse = 0;

        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_2);
    }

    fan_init = 1;
}

void Fan::setSpeed(uint8_t fanspeed)
{
    if (fan_init != 1){
        init();
        fan_init = 1;
    }

    TIM_OC_InitTypeDef  TIM_InitStruct;

    TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TIM_InitStruct.Pulse = (uint32_t)fanspeed * 257;

    fan_speed = fanspeed;

    if (fan_index == 0){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_1);
//        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_3);
//        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_3);
    }
    else if (fan_index == 1){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_2);
    }
}

Fan pwmfan_one(0);
Fan pwmfan_two(1);

//static uint32_t pulse = 0;
//
//void freqInit()
//{
//    GPIO_InitTypeDef    GPIO_InitStruct;
////   TIM_OC_InitTypeDef  TIM_InitStruct;
//
//    __HAL_RCC_GPIOD_CLK_ENABLE();
//
//    GPIO_InitStruct.Pin         = GPIO_PIN_15;
//    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull        = GPIO_NOPULL;             ///内部不做上下拉电阻
//    GPIO_InitStruct.Speed       = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate   = GPIO_AF2_TIM4;
//    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//
//    ///PWM时钟配置
//    __HAL_RCC_TIM4_CLK_ENABLE();
//    TIM_PWM_HandleStruct.Instance = TIM4;
//    TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock / 2000000) - 1;
//    TIM_PWM_HandleStruct.Init.Period = 65534;         ///总共255
//    TIM_PWM_HandleStruct.Init.ClockDivision = 0;
//    TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
//    HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);
//
//    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(TIM4_IRQn);
//}
//
//void setFreqOut(uint32_t Freq)
//{
//    TIM_OC_InitTypeDef  TIM_InitStruct;
//
//    TIM_InitStruct.OCMode = TIM_OCMODE_TOGGLE;
//    TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
//    TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
//    pulse = 2000000/Freq/2;
//    TIM_InitStruct.Pulse = pulse;
//
//    HAL_TIM_OC_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_4);
//    HAL_TIM_OC_Start_IT(&TIM_PWM_HandleStruct,TIM_CHANNEL_4);
//}
//
//void stopFreqOut()
//{
//    HAL_TIM_OC_Stop_IT(&TIM_PWM_HandleStruct,TIM_CHANNEL_4);
//}
//
//void TIM4_IRQHandler(void)
//{
//    if(__HAL_TIM_GET_FLAG(&TIM_PWM_HandleStruct, TIM_FLAG_CC4) != RESET)
//    {
//        if(__HAL_TIM_GET_IT_SOURCE(&TIM_PWM_HandleStruct, TIM_IT_CC4) !=RESET)
//        {
//            __HAL_TIM_CLEAR_IT(&TIM_PWM_HandleStruct, TIM_IT_CC4);
//
//            if((TIM_PWM_HandleStruct.Instance->CCMR2 & TIM_CCMR2_CC4S) == 0x00)
//            {
//                TIM_PWM_HandleStruct.Instance->CCR4 = (TIM_PWM_HandleStruct.Instance->CCR4 + pulse > 65534) ? TIM_PWM_HandleStruct.Instance->CCR4 + pulse - 65535 : TIM_PWM_HandleStruct.Instance->CCR4 + pulse;
//            }
//        }
//    }
//}
