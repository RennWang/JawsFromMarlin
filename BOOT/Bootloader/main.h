#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"

#include "ymodem.h"

#define  FACTORY_VERSION		1

/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef UartHandle;

extern uint8_t aFileName[FILE_NAME_LENGTH];

typedef  void (*pFunction)(void);

extern pFunction JumpToApplication;
extern uint32_t JumpAddress;

#endif /* __MAIN_H */
