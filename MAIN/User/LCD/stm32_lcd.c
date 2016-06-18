#include "LCDMiddle.h"

typedef struct
{
  __IO uint16_t REG;
  __IO uint16_t RAM;
}LCD_CONTROLLER_TypeDef;

#define TFT_LCD_BASE  ((uint32_t)(0x6C000000 | 0X0000007E))
#define TFT_LCD       ((LCD_CONTROLLER_TypeDef *) TFT_LCD_BASE)

static uint8_t Is_LCD_IO_Initialized = 0;

static void TFT_LCD_Init(void);
static void TFT_LCD_MspInit(void);

void LCD_IO_Init(void)
{
    if(Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1;
        TFT_LCD_Init();
    }
}

void LCD_IO_WriteData(uint16_t RegValue)
{
    TFT_LCD->RAM = RegValue;
}

void LCD_IO_WriteReg(uint8_t Reg)
{
    TFT_LCD->REG = Reg;
}

uint16_t LCD_IO_ReadData(void)
{
    return TFT_LCD->RAM;
}

static void TFT_LCD_Init(void)
{
    SRAM_HandleTypeDef hsram;
    FSMC_NORSRAM_TimingTypeDef Write_Sram_Timing;
    FSMC_NORSRAM_TimingTypeDef Read_Sram_Timing;

    hsram.Instance  = FSMC_NORSRAM_DEVICE;
    hsram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;

    Read_Sram_Timing.AddressSetupTime      = 15;
    Read_Sram_Timing.AddressHoldTime       = 1;
    Read_Sram_Timing.DataSetupTime         = 24;
    Read_Sram_Timing.BusTurnAroundDuration = 0;
    Read_Sram_Timing.CLKDivision           = 2;
    Read_Sram_Timing.DataLatency           = 2;
    Read_Sram_Timing.AccessMode            = FSMC_ACCESS_MODE_A;

    Write_Sram_Timing.AddressSetupTime      = 3;
    Write_Sram_Timing.AddressHoldTime       = 1;
    Write_Sram_Timing.DataSetupTime         = 2;
    Write_Sram_Timing.BusTurnAroundDuration = 0;
    Write_Sram_Timing.CLKDivision           = 2;
    Write_Sram_Timing.DataLatency           = 2;
    Write_Sram_Timing.AccessMode            = FSMC_ACCESS_MODE_A;

    hsram.Init.NSBank             = FSMC_NORSRAM_BANK4;
    hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_ENABLE;
    hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;

    TFT_LCD_MspInit();
    HAL_SRAM_Init(&hsram, &Read_Sram_Timing, &Write_Sram_Timing);
}

static void TFT_LCD_MspInit(void)
{
    GPIO_InitTypeDef GPIO_Init_Structure;

    __FSMC_CLK_ENABLE();

    __GPIOB_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();

    /* Common GPIO configuration */
    GPIO_Init_Structure.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_Init_Structure.Pull      = GPIO_PULLUP;
    GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;

    GPIO_Init_Structure.Pin       = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);

    GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FSMC;

    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8     |\
                                GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin   = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11   |\
                                GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin   = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin   = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
}
