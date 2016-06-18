/**
  ******************************************************************************
  * @file    HeatingElement.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    30-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "HeatingElement.h"
#include "Temperature.h"

ExtruderHeatingElement::ExtruderHeatingElement(uint8_t id):
	heaterId(id)
{
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value)
{
    if (heaterId == 0){
        setPWMHeaterValue(0, value);
        setPWMHeaterOn(0);
    }else if (heaterId == 1){
        setPWMHeaterValue(2, value);
        setPWMHeaterOn(2);
    }else if (heaterId == 2){
        setPWMHeaterValue(3, value);
        setPWMHeaterOn(3);
    }
}

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value)
{
    setPWMHeaterValue(1, value);
    setPWMHeaterOn(1);
}
