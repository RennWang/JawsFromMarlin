/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MICRO_SD_H
#define __MICRO_SD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/**
  * @brief SD Card information structure
  */
#define SD_CardInfo HAL_SD_CardInfoTypedef

/**
  * @brief  SD status structure definition
  */
#define MSD_OK         0x00
#define MSD_ERROR      0x01

/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/

/** @defgroup STM3210E_EVAL_SD_Exported_Constants Exported_Constants
  * @{
  */
//#define SD_DETECT_PIN                    GPIO_PIN_11
//#define SD_DETECT_GPIO_PORT              GPIOF
//#define __SD_DETECT_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOF_CLK_ENABLE()
//#define SD_DETECT_IRQn                   EXTI15_10_IRQn

#define SD_DATATIMEOUT           ((uint32_t)100000000)

#define SD_PRESENT               ((uint8_t)0x01)
#define SD_NOT_PRESENT           ((uint8_t)0x00)

/* DMA definitions for SD DMA transfer */
#define __DMAx_TxRx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE
#define SD_DMAx_Tx_INSTANCE               DMA2_Channel4
#define SD_DMAx_Rx_INSTANCE               DMA2_Channel4
#define SD_DMAx_Tx_IRQn                   DMA2_Channel4_5_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Channel4_5_IRQn
#define SD_DMAx_Tx_IRQHandler             DMA2_Channel4_5_IRQHandler
#define SD_DMAx_Rx_IRQHandler             DMA2_Channel4_5_IRQHandler

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/** @addtogroup STM3210E_EVAL_SD_Exported_Functions
  * @{
  */
uint8_t BSP_SD_Init(void);
//uint8_t BSP_SD_ITConfig(void);
//void    BSP_SD_DetectIT(void);
//void    BSP_SD_DetectCallback(void);
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr);
void    BSP_SD_IRQHandler(void);
void    BSP_SD_DMA_Tx_IRQHandler(void);
void    BSP_SD_DMA_Rx_IRQHandler(void);
HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void);
void    BSP_SD_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo);
//uint8_t BSP_SD_IsDetected(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM3210E_EVAL_SD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
