/**
  ******************************************************************************
  * @file    Timeout.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "Timeout.h"
#include "Jaws_main.h"

Timeout::Timeout() : flags(0)
{

}

void Timeout::start(uint32_t durationMillis)
{
    flags = TIMEOUT_FLAGS_ACTIVE;

    startTimeMillis = HAL_GetTick();
    delayTimeMillis = durationMillis;
}

bool Timeout::hasElapsed()
{
     if ( flags == TIMEOUT_FLAGS_ACTIVE ) {
        if ( HAL_GetTick() - startTimeMillis >= delayTimeMillis)
            flags = TIMEOUT_FLAGS_ELAPSED;
     }

     return 0 != (flags & TIMEOUT_FLAGS_ELAPSED);
}

