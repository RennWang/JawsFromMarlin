#include "watchdog.h"
#include "Jaws_main.h"

static IWDG_HandleTypeDef IwdgHandle;

void watchdog_init()
{
    IwdgHandle.Instance = IWDG;

    IwdgHandle.Init.Prescaler = IWDG_PRESCALER_128;
    IwdgHandle.Init.Reload    = 1446;       //5s

    HAL_IWDG_Init(&IwdgHandle);
    HAL_IWDG_Start(&IwdgHandle);
}

void watchdog_reset()
{
    HAL_IWDG_Refresh(&IwdgHandle);
}
