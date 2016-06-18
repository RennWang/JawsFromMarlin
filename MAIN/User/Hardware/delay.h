/**
  ******************************************************************************
  * @file    delay.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

void HAL_Delay_us(uint32_t Delay_us);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H_INCLUDED */
