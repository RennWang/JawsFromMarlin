/**
  ******************************************************************************
  * @file    Jaws_main.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    7-April-2015
  * @brief
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef JAWS_MAIN_H
#define JAWS_MAIN_H


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
//#include "GUI.h"


/* Macros --------------------------------------------------------------------*/
#define PI          3.1415926535897932384626433832795
#define HALF_PI     1.5707963267948966192313216916398
#define TWO_PI      6.283185307179586476925286766559
#define DEG_TO_RAD  0.017453292519943295769236907684886
#define RAD_TO_DEG  57.295779513082320876798154814105

#define min(a,b)    ((a)<(b)?(a):(b))
#define max(a,b)    ((a)>(b)?(a):(b))
#define constrain(amt,low,high)     ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)        ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)    ((deg)*DEG_TO_RAD)
#define degrees(rad)    ((rad)*RAD_TO_DEG)
#define sq(x)           ((x)*(x))
#define square(x)       ((x)*(x))


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
