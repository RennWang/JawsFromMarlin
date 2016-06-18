/**
  ******************************************************************************
  * @file    Heat.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef HEAT_H_INCLUDED
#define HEAT_H_INCLUDED

#include "Jaws_main.h"
#include "Heater.h"

namespace heat {

extern Heater   extruder0_Heater;
extern Heater   extruder1_Heater;
extern Heater   extruder2_Heater;
extern Heater   bed_Heater;

void heatInit();
void disableHeat(void);
}

#endif /* HEAT_H_INCLUDED */
