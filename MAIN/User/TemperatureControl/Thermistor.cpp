#include "Thermistor.h"
#include "ThermistorTables.h"
#include "Temperature.h"

Thermistor::Thermistor(ThermistorType ttype):rawValue(0)
{
    thermistorType = ttype;
    pThermistorTables = getThermistorTables();
    thermistorLen = getThermistorTablesLen();
}

SensorState Thermistor::update()
{
    if (thermistorType == THERMISTOR_HEATER_0)
        rawValue = getTemperatureRawValue(0);
    else if (thermistorType == THERMISTOR_BED)
        //rawValue = getTemperatureRawValue(3);
        rawValue = getTemperatureRawValue(1);
    else if (thermistorType == THERMISTOR_HEATER_1)
        rawValue = getTemperatureRawValue(2);
    else if (thermistorType == THERMISTOR_HEATER_2)
        rawValue = getTemperatureRawValue(3);
    return SS_OK;
}

float Thermistor::getTemperature()
{
    float celsius = 0;
    uint8_t i;
    uint16_t (*tt)[][2] = (uint16_t (*)[][2])pThermistorTables;
    uint8_t len =  thermistorLen;

    for (i=1; i<len; i++){
      if ((*tt)[i][0] > rawValue){
        celsius = (*tt)[i-1][1] + (rawValue - (*tt)[i-1][0]) *((float)(*tt)[i][1] - (float)(*tt)[i-1][1]) /((float)(*tt)[i][0] - (float)(*tt)[i-1][0]);
        break;
      }
    }

    if (i == len) celsius = (*tt)[i-1][1];

    return celsius;
}

uint16_t Thermistor::getRawValue()
{
    return rawValue;
}

uint16_t* Thermistor::getThermistorTables()
{
#ifdef THERMISTORHEATER_0
    if (thermistorType == THERMISTOR_HEATER_0)
        return (uint16_t*)HEATER_0_TEMPTABLE;
#endif

#ifdef THERMISTORHEATER_1
    if (thermistorType == THERMISTOR_HEATER_1)
        return (uint16_t*)HEATER_1_TEMPTABLE;
#endif

#ifdef THERMISTORHEATER_2
    if (thermistorType == THERMISTOR_HEATER_2)
        return (uint16_t*)HEATER_2_TEMPTABLE;
#endif

#ifdef THERMISTORBED
    if (thermistorType == THERMISTOR_BED)
        return (uint16_t*)HEATER_1_TEMPTABLE;
#endif
    return 0;
}

uint8_t Thermistor::getThermistorTablesLen()
{
#ifdef THERMISTORHEATER_0
    if (thermistorType == THERMISTOR_HEATER_0)
        return HEATER_0_TEMPTABLE_LEN;
#endif

#ifdef THERMISTORHEATER_1
    if (thermistorType == THERMISTOR_HEATER_1)
        return HEATER_1_TEMPTABLE_LEN;
#endif

#ifdef THERMISTORHEATER_2
    if (thermistorType == THERMISTOR_HEATER_2)
        return HEATER_2_TEMPTABLE_LEN;
#endif

#ifdef THERMISTORBED
    if (thermistorType == THERMISTOR_BED)
        return BEDTEMPTABLE_LEN;
#endif
    return 0;
}
