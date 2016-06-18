#include "main.h"
#include "flash_if.h"
#include "common.h"
#include "string.h"
#include "eeprom.h"

UART_HandleTypeDef UartHandle;

uint8_t aFileName[FILE_NAME_LENGTH];

pFunction JumpToApplication;
uint32_t JumpAddress;
static void IAP_Init(void);
static void SerialDownload(void);

static void executeApplication(void);
static uint8_t isSerialFirmwareUpdate(void);
static uint16_t hasAvailableFirmware(void);
static void firmwareIsAvailable(uint16_t flag);

#if (FACTORY_VERSION != 0)
static uint16_t getFactoryFirmwareFlag(void);
static void setFactoryFirmwareFlag(uint16_t flag);
#endif

int main(void)
{
		//HAL_StatusTypeDef status;
		uint32_t uwticks = HAL_GetTick();
		
  	FLASH_If_Init();
    IAP_Init();
	
		Serial_PutString("booting...\n");
	
		while (HAL_GetTick()-uwticks < 10000){
				if (isSerialFirmwareUpdate()){
						//Serial_PutByte('C');
						SerialDownload();
						break;
				}
		}
		
#if (FACTORY_VERSION != 0)
		if (getFactoryFirmwareFlag() != 0x1234){
				setFactoryFirmwareFlag(0x1234);	
				executeApplication();
		}else{
				if (hasAvailableFirmware()){
					executeApplication();
				}
		}
#else
		if (hasAvailableFirmware()){
			executeApplication();
		}	
#endif
		while(1){};
}

void IAP_Init(void)
{
  GPIO_InitTypeDef gpioInit;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioInit.Pin 					= GPIO_PIN_9 | GPIO_PIN_10;
	gpioInit.Mode 				= GPIO_MODE_AF_PP;
	gpioInit.Pull 				= GPIO_PULLUP;
	gpioInit.Speed 				= GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &gpioInit);
	
	__HAL_RCC_USART1_CLK_ENABLE();
	
	UartHandle.Instance							= USART1;
  UartHandle.Init.BaudRate 				= 115200;
  UartHandle.Init.WordLength 			= UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits 				= UART_STOPBITS_1;
  UartHandle.Init.Parity 					= UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl 			= UART_HWCONTROL_NONE;
  UartHandle.Init.Mode 						= UART_MODE_TX_RX;

	HAL_UART_Init(&UartHandle);
}
	
void SerialDownload(void)
{
	uint8_t number[11] = {0};
  uint32_t size = 0;
  COM_StatusTypeDef result;
	
	firmwareIsAvailable(0);
	result = Ymodem_Receive( &size );
  if (result == COM_OK)
  {
		firmwareIsAvailable(1);
    Serial_PutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
    Serial_PutString(aFileName);
    Int2Str(number, size);
    Serial_PutString("\n\r Size: ");
    Serial_PutString(number);
    Serial_PutString(" Bytes\r\n");
    Serial_PutString("-------------------\n");
  }
  else if (result == COM_LIMIT)
  {
    Serial_PutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (result == COM_DATA)
  {
    Serial_PutString("\n\n\rVerification failed!\n\r");
  }
  else if (result == COM_ABORT)
  {
    Serial_PutString("\r\n\nAborted by user.\n\r");
  }
  else
  {
    Serial_PutString("\n\rFailed to receive the file!\n\r");
  }
}

void executeApplication(void)
{
	if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
	{
			/* Jump to user application */
			JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
			JumpToApplication = (pFunction) JumpAddress;
			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
			JumpToApplication();
	}
}

uint8_t isSerialFirmwareUpdate(void)
{
  uint32_t packet_size = 0;
	uint8_t updatestr[]="Firmware update";
  HAL_StatusTypeDef status;

  status = HAL_UART_Receive(&UartHandle, aFileName, 1, DOWNLOAD_TIMEOUT);
	
	if (status == HAL_OK){
		if (aFileName[0] == 'F'){
				packet_size = 14;
				status = HAL_UART_Receive(&UartHandle, aFileName + 1, packet_size, DOWNLOAD_TIMEOUT);
			
				if (status == HAL_OK){
						if (0 == strncmp((const char *)aFileName, (char *)updatestr, 15))
							return 1;
				}
		}
	}
	
	return 0;
}

static uint16_t hasAvailableFirmware(void)
{
	return eeprom_read(0);
}

static void firmwareIsAvailable(uint16_t flag)
{
	  eeprom_write(0,flag);
}

#if (FACTORY_VERSION != 0)
static uint16_t getFactoryFirmwareFlag(void)
{
    return eeprom_read(2);
}
	
static void setFactoryFirmwareFlag(uint16_t flag)
{
	  eeprom_write(2,flag);
}		
#endif

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
