#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <stdint.h>

typedef enum{
    THERMISTOR_HEATER_0 = 0,
    THERMISTOR_HEATER_1,
    THERMISTOR_HEATER_2,
    THERMISTOR_BED
}ThermistorType;

typedef enum{
    SS_OK,              ///< Temperature measured correctly
    SS_BAD_READ         ///< Temperature is outside the expected range
}SensorState;

class Thermistor
{
public:
    Thermistor(ThermistorType ttype);

public:
    SensorState update();
    float getTemperature();
    uint16_t getRawValue();

private:
    uint16_t* getThermistorTables();
    uint8_t getThermistorTablesLen();


private:
    ThermistorType thermistorType;
    uint16_t* pThermistorTables;
    uint8_t thermistorLen;

    volatile uint16_t rawValue;
};

#endif /* THERMISTOR_H */
