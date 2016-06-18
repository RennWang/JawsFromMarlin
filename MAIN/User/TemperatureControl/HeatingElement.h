/**
  ******************************************************************************
  * @file    HeatingElement.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    30-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef HEATINGELEMENT_H
#define HEATINGELEMENT_H

#include <stdint.h>

/// The heating element interface is a standard interface used to communicate with things that can control
class HeatingElement
{
public:
        /// Set the output of the heating element
        virtual void setHeatingElement(uint8_t value) =0;
};

class ExtruderHeatingElement : public HeatingElement
{
public:
	ExtruderHeatingElement(uint8_t id);
	void setHeatingElement(uint8_t value);
	uint8_t heaterId;
};

class BuildPlatformHeatingElement : public HeatingElement
{
public:
	void setHeatingElement(uint8_t value);
};

#endif /* HEATINGELEMENT_H */
