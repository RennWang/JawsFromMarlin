/**
  ******************************************************************************
  * @file    hw_I2CDigPot.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef DIGPOT_MCP44XX_H
#define DIGPOT_MCP44XX_H

#include <stdint.h>

#define MCP4431		127
#define MCP4441		127
#define MCP4451		255
#define MCP4461		255

#define EE_TYPE MCP4441

void digipot_set(int channel, int value);


#endif /* DIGPOT_MCP44XX_H */
