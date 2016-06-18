#ifndef JAWSBOARD_H
#define JAWSBOARD_H

#include "Jaws_main.h"
#include "Timeout.h"
#include "Heater.h"
#include "Fan.h"

class Jawsboard
{
private:
    /// Static instance of the jawsboard
    static Jawsboard jawsboard;

public:
    /// Get the jawsboard instance.
    static Jawsboard& getBoard() { return jawsboard; }

private:
	/// Private constructor; use the singleton
	Jawsboard();

public:
    bool using_platform;
    uint8_t tool_count;
    bool pstop_enabled;

    Heater*  pExtruder_Heater[3];
    Heater*  pBed_Heater;

    Timeout extruder_timeout;
	Timeout platform_timeout;

    Fan* pExtruder_Fan;
    Fan* pExtra_Fan;

    void init();
    void reset(bool hard_reset);

    bool isUsingPlatform();
    uint8_t getToolCount();

    void heatersPause(bool platform);
    void heatersResume(bool platform);
    bool heatersHasReachedTargetTemperature(bool platform);

    void heatersOff(bool platform);
    void setTargetDegExtruder(float celsius, uint8_t extruder);
    bool isHeatingExtruder(uint8_t extruder);
    bool isCoolingExtruder(uint8_t extruder);
    float getCurrentDegExtruder(uint8_t extruder);
    float getTargetDegExtruder(uint8_t extruder);

    void setTargetDegBed(float celsius);
    bool isHeatingBed();
    bool isCoolingBed();
    float getCurrentDegBed();
    float getTargetDegBed();

    int getHeaterPower(int heater);

    void runboardSlice();
};



#endif /* JAWSBOARD_H */
