#include "Temperature.h"
#include "ThermistorTables.h"
#include "Jaws_main.h"

//__IO uint32_t using_channel;
__IO uint16_t temprature_raw_sum[HEATER_COUNT];
uint16_t temprature_raw_buffer[HEATER_COUNT*OVERSAMPLENR];

static ADC_HandleTypeDef  ADC_HandleStruct;
static TIM_HandleTypeDef  TIM_ADC_HandleStruct;
static DMA_HandleTypeDef  DMA_HandleStruct;
static TIM_HandleTypeDef  TIM_PWM_HandleStruct;

static void init_BeforeADC(void);
static void drv_StartMeasureTemperatureRaw(void);
static void drv_StopMeasureTemperatureRaw(void);

#define drv_StartTemperatureTimer()     HAL_TIM_Base_Start_IT(&TIM_ADC_HandleStruct)

void TemperatureManageInit(void)
{
    hw_TemperatureManageInit();
    drv_StartTemperatureTimer();
}

/** \brief 温度控制硬件初始化
 *
 * \return void
 */
void hw_TemperatureManageInit(void)
{
    ///ADC配置，ADC1,in10,in11,in12,in13
    GPIO_InitTypeDef    GPIO_InitStruct;
    TIM_OC_InitTypeDef  TIM_InitStruct;
    ADC_ChannelConfTypeDef sConfig;

    ADC_HandleStruct.Instance                       = ADC1;

    if (HAL_ADC_DeInit(&ADC_HandleStruct) != HAL_OK)
    {
        return;
    }

    ADC_HandleStruct.Init.ScanConvMode              = ADC_SCAN_ENABLE;
    ADC_HandleStruct.Init.ContinuousConvMode        = ENABLE;
    ADC_HandleStruct.Init.DiscontinuousConvMode     = DISABLE;
    ADC_HandleStruct.Init.DataAlign                 = ADC_DATAALIGN_RIGHT;
    ADC_HandleStruct.Init.ExternalTrigConv          = ADC_SOFTWARE_START;
    ADC_HandleStruct.Init.NbrOfConversion           = HEATER_COUNT;

    init_BeforeADC();
    HAL_ADC_Init(&ADC_HandleStruct);


    sConfig.Channel = ADC_CHANNEL_10;                        ///Channel_Extruder_0
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&ADC_HandleStruct, &sConfig);

#ifdef HEATBED_ENABLED
    sConfig.Channel = ADC_CHANNEL_11;                        ///Channel_bed
    sConfig.Rank = 2;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&ADC_HandleStruct, &sConfig);
#endif
#ifdef EXTRUDER_2_ENABLED
    sConfig.Channel = ADC_CHANNEL_12;                        ///Channel_Extruder_1
    sConfig.Rank = 3;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&ADC_HandleStruct, &sConfig);
#endif
#ifdef EXTRUDER_3_ENABLED
    sConfig.Channel = ADC_CHANNEL_13;                        ///Channel_Extruder_2
    sConfig.Rank = 4;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&ADC_HandleStruct, &sConfig);
#endif
    HAL_ADCEx_Calibration_Start(&ADC_HandleStruct);

    ///ADC时钟配置  采样周期100ms
    __HAL_RCC_TIM6_CLK_ENABLE();
    TIM_ADC_HandleStruct.Instance           = TIM6;
    TIM_ADC_HandleStruct.Init.Period        = 1000 - 1;
    TIM_ADC_HandleStruct.Init.Prescaler     = (uint32_t) (SystemCoreClock/2/10000) - 1;
    TIM_ADC_HandleStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM_ADC_HandleStruct.Init.CounterMode   = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TIM_ADC_HandleStruct);
    __HAL_TIM_SET_COUNTER(&TIM_ADC_HandleStruct, 0);

    HAL_NVIC_SetPriority(TIM6_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);

    ///加热PWM配置  PB6,7,8,9
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6;
#ifdef HEATBED_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_7;
#endif
#ifdef EXTRUDER_2_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_8;
#endif
#ifdef EXTRUDER_3_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_9;
#endif
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;             ///内部不做上下拉电阻
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    ///PWM时钟配置
    __HAL_RCC_TIM4_CLK_ENABLE();
    TIM_PWM_HandleStruct.Instance = TIM4;
    TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock/2/250000) - 1;
    TIM_PWM_HandleStruct.Init.Period = 254;         ///总共255
    TIM_PWM_HandleStruct.Init.ClockDivision = 0;
    TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);

    TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TIM_InitStruct.Pulse = 0;

    HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_1);
#ifdef HEATBED_ENABLED
    HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_2);
#endif
#ifdef EXTRUDER_2_ENABLED
    HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_3);
#endif
#ifdef EXTRUDER_3_ENABLED
    HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_4);
#endif
}

/** \brief  开始进行温度AD转换
 *
 * \return  void
 */
static void drv_StartMeasureTemperatureRaw(void)
{
//    using_channel = channel;
    HAL_ADC_Start_DMA(&ADC_HandleStruct, (uint32_t*)temprature_raw_buffer, HEATER_COUNT*OVERSAMPLENR);
}

/** \brief  停止进行温度AD转换
 *
 * \return void
 */
static void drv_StopMeasureTemperatureRaw()
{
    HAL_ADC_Stop_DMA(&ADC_HandleStruct);
}

/** \brief  温度采样值的求和
 *
 * \param void
 * \return void
 */
void CalcTemperatureRawValueSum(void)
{
    for (int i=0; i<HEATER_COUNT;i++){
        temprature_raw_sum[i] = 0;
        for (int j=0; j<OVERSAMPLENR;j++)
            temprature_raw_sum[i] += temprature_raw_buffer[i+j*HEATER_COUNT];
    }
}

/** \brief 获取温度采样值的和
 *
 * \param index uint8_t   0-ex1; 1-bed; 2-ex2; 3-ex3
 * \return uint16_t
 */
uint16_t getTemperatureRawValue(uint8_t index)
{
    return  ((index < HEATER_COUNT) ? temprature_raw_sum[index] : 0);
}

/** \brief 设置加热PWM值
 *
 * \param index uint8_t   0-ex1; 1-bed; 2-ex2; 3-ex3
 * \return uint16_t
 */
void setPWMHeaterValue(uint8_t index, uint8_t value)
{
    TIM_OC_InitTypeDef  TIM_InitStruct;

    TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TIM_InitStruct.Pulse = (uint32_t)value;

    if (index == 0){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_1);
    }
#ifdef HEATBED_ENABLED
    else if (index == 1){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_2);
    }
#endif
#ifdef EXTRUDER_2_ENABLED
    else if (index == 2){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_3);
    }
#endif
#ifdef EXTRUDER_3_ENABLED
    else if (index == 3){
        HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_4);
    }
#endif
}

/** \brief 启动加热PWM
 *
 * \param index uint8_t   0-ex1; 1-bed; 2-ex2; 3-ex3
 * \return uint16_t
 */
void setPWMHeaterOn(uint8_t index)
{
    if (index == 0){
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_1);
    }
#ifdef HEATBED_ENABLED
    else if (index == 1){
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_2);
    }
#endif
#ifdef EXTRUDER_2_ENABLED
    else if (index == 2){
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_3);
    }
#endif
#ifdef EXTRUDER_3_ENABLED
    else if (index == 3){
        HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_4);
    }
#endif
}

static void init_BeforeADC(void)
{
    GPIO_InitTypeDef            GPIO_InitStruct;
    RCC_OscInitTypeDef          RCC_OscInitStructure;
    RCC_PeriphCLKInitTypeDef    PeriphClkInit;

//    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

//    HAL_RCC_GetOscConfig(&RCC_OscInitStructure);
//    RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//    RCC_OscInitStructure.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//    RCC_OscInitStructure.HSIState = RCC_HSI_ON;
//    HAL_RCC_OscConfig(&RCC_OscInitStructure);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
#ifdef HEATBED_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_11;
#endif
#ifdef EXTRUDER_2_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_12;
#endif
#ifdef EXTRUDER_3_ENABLED
    GPIO_InitStruct.Pin |= GPIO_PIN_13;
#endif
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_DMA1_CLK_ENABLE();
    DMA_HandleStruct.Instance = DMA1_Channel1;

    DMA_HandleStruct.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    DMA_HandleStruct.Init.PeriphInc           = DMA_PINC_DISABLE;
    DMA_HandleStruct.Init.MemInc              = DMA_MINC_ENABLE;
    DMA_HandleStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;   /* Transfer from ADC by half-word to match with ADC configuration: ADC resolution 10 or 12 bits */
    DMA_HandleStruct.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;   /* Transfer to memory by half-word to match with buffer variable type: half-word */
    DMA_HandleStruct.Init.Mode                = DMA_CIRCULAR;              /* DMA in circular mode to match with ADC configuration: DMA continuous requests */
    DMA_HandleStruct.Init.Priority            = DMA_PRIORITY_HIGH;

    HAL_DMA_DeInit(&DMA_HandleStruct);
    HAL_DMA_Init(&DMA_HandleStruct);

    __HAL_LINKDMA(&ADC_HandleStruct, DMA_Handle, DMA_HandleStruct);

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/** \brief  温度采样完成回调函数的强引用  替代stm32f4xx_hal_adc.c中的弱引用
 *
 * \param hadc ADC_HandleTypeDef*
 * \return void
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    drv_StopMeasureTemperatureRaw();
    CalcTemperatureRawValueSum();
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&DMA_HandleStruct);
}

void TIM6_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&TIM_ADC_HandleStruct, TIM_FLAG_UPDATE) != RESET){
        if(__HAL_TIM_GET_IT_SOURCE(&TIM_ADC_HandleStruct, TIM_IT_UPDATE) !=RESET){
            __HAL_TIM_CLEAR_IT(&TIM_ADC_HandleStruct, TIM_IT_UPDATE);
            ///时基中断
            drv_StartMeasureTemperatureRaw();
        }
    }
}





