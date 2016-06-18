/**
  ******************************************************************************
  * @file    Heater.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>
#include "Thermistor.h"
#include "PID.h"
#include "Timeout.h"
#include "HeatingElement.h"

#define MAX_VALID_TEMP 280
#define MAX_HBP_TEMP   130

typedef enum{
	HEATER_FAIL_NONE = 0,
	HEATER_FAIL_NOT_PLUGGED_IN = 0x02,
	HEATER_FAIL_SOFTWARE_CUTOFF = 0x04,
	HEATER_FAIL_NOT_HEATING = 0x08,
	HEATER_FAIL_DROPPING_TEMP = 0x10,
	HEATER_FAIL_BAD_READS = 0x20
}HeaterFailMode;

class Heater
{
public:
    Heater(Thermistor& sensorIn, HeatingElement& elementIn, bool timingCheckOn);

public:
    void abort();

    void reset();

    void disable(bool on);

    bool isDisabled(){return is_Disabled;}

    /// Get the current sensor temperature, in degrees Celcius
    int16_t getCurrentTemperature() { return currentTemperature; }

    /// Get the setpoint temperature, in degrees Celcius
    int16_t getSetTemperature() { return ( is_Paused ) ? pausedSetTemperature : pid.getTarget(); }

    /// Set the target output temperature, temp New target temperature, in degrees Celcius.
    void setTargetTemperature(int16_t targetTemp);

    bool hasReachedTargetTemperature();
    bool hasFailed() { return failState; }

    /// Change the setpoint temperature, value New setpoint temperature, in degrees Celcius.
    void setOutput(uint8_t value);
    uint8_t getOutput(){ return outputvalue;}

    void Pause(bool on);

    bool isPaused() { return is_Paused;}

    /// is heater temperature target different than current
    bool isHeating();

    /// is heater temperature target less than current temperature
    bool isCooling();

    /// Get the current PID error term E term from the PID controller
    int16_t getPIDErrorTerm() { return pid.getErrorTerm(); }

    /// Get the current PID delta term D term from the PID controller
    int16_t getPIDDeltaTerm() { return pid.getDeltaTerm(); }

    /// Get the last PID output last output from the PID controller
    int16_t getPIDLastOutput() { return pid.getLastOutput(); }

    /// get the difference between the current temperature and the set temperature
    int16_t getDelta();

    void manageHeater();

private:
    Thermistor& sensor;
    HeatingElement& element;

    PID pid;

    uint8_t outputvalue;

    uint32_t sample_interval_micros;    ///< Interval that the temperature sensor should be updated at.

    volatile int16_t currentTemperature;        ///< Last known temperature reading
    int16_t startTemp;		                    ///< start temperature when new target is set.  used to assess heating up progress
    int16_t pausedSetTemperature;		        ///< we record the set temperature when a heater is "paused"
    bool newTargetReached;				///< flag set when heater reached target and cleared when a new temperature is set

    bool bypassingPID;                  ///< True if the heater is in full on

    uint16_t eepromBase;                ///< Base address to read EEPROM configuration from

    bool failState;                     ///< True if the heater has detected a hardware ,failure and is shut down.
    uint8_t failCount;                  ///< Count of the number of hardware failures that have been reported by #getTemperature().
                                        ///< If this goes over #SENSOR_MAX_BAD_READINGS, then the heater will go into a fail state.
    uint8_t valueFailCount;			    ///< a second failure counter for valid temp reads that are out of range (eg too hot)

    HeaterFailMode failMode;			///< queryable state to indicate WHY the heater fails

    Timeout heatingUpTimer;				///< timeout indicating how long heater has been heating
    Timeout heatProgressTimer;			///< timeout to flag if heater is not heating up from start
    bool progressChecked;				///< flag that heating up progress has been checked.
    const bool heatTimingCheck;         ///< allow disabling of heat progress timing for heated build platform.
    bool is_Paused;						///< set to true when we wish to pause the heater from heating up
    bool is_Disabled;					///< heaters are disabled when they are not present (user settable)

    /// While the calibration offset is silly, we leverage the calibration_eeprom_offset as a means of telling us if we're dealing with an extruder or HBP
    uint8_t calibration_eeprom_offset;  ///axis offset in HEATER_CALIBRATE

    /// Put the heater into a failure state, ensuring that the heating element is disabled.
    void fail();
};

extern    Heater                extruderHeater[3];
extern    Heater                bedHeater;

#endif /* HEATER_H */
