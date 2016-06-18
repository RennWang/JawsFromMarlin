/**
  ******************************************************************************
  * @file    Heater.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "Heater.h"
#include "Eeprom.h"
#include "EepromMap.h"

/// Offset to compensate for range clipping and bleed-off
#define HEATER_OFFSET_ADJUSTMENT 0

/// PID bypass: If the set point is more than this many degrees over the
///             current temperature, bypass the PID loop altogether.
/// Note that Marlin uses 10C
#define PID_BYPASS_DELTA 10

/// Number of bad sensor readings we need to get in a row before shutting off the heater
const uint8_t SENSOR_MAX_BAD_READINGS = 15;

/// Number of temp readings to be at target value before triggering newTargetReached
/// with bad seating of thermocouples, we sometimes get innacurate reads
const uint16_t TARGET_CHECK_COUNT = 5;

/// If we read a temperature higher than this, shut down the heater
const int16_t HEATER_CUTOFF_TEMPERATURE = 290;

/// temperatures below setting by this amount will flag as "not heating up"
const int16_t HEAT_FAIL_THRESHOLD = 30;

// don't trigger heating up checking for target temperatures less than this
const int16_t HEAT_FAIL_CHECK_THRESHOLD = 30;

/// if the starting temperature is less than this amount, we will check heating progress
/// to get to this temperature, the heater has already been checked.
const int16_t HEAT_CHECKED_THRESHOLD = 50;

/// timeout for heating all the way up
const uint32_t HEAT_UP_TIME = 300000;  //five minutes

/// timeout for showing heating progress
const uint32_t HEAT_PROGRESS_TIME = 90000; // 90 seconds

/// threshold above starting temperature we check for heating progres
const int16_t HEAT_PROGRESS_THRESHOLD = 10;

///
const int16_t BAD_TEMPERATURE = 290;


Heater::Heater(Thermistor& sensorIn, HeatingElement& elementIn, bool timingCheckOn):
    sensor(sensorIn),
    element(elementIn),
    heatTimingCheck(timingCheckOn)
{
    //reset();
}

void Heater::reset()
{
	currentTemperature = 0;
	startTemp = 0;
	pausedSetTemperature = 0;

	abort();
}

void Heater::abort() {

	failState = false;
	failCount = 0;
	failMode = HEATER_FAIL_NONE;
	valueFailCount = 0;

	bypassingPID = false;
	heatingUpTimer = Timeout();
	heatProgressTimer = Timeout();
	progressChecked = false;
	newTargetReached = false;
	is_Paused = false;
	is_Disabled = false;

	float p = 0, i = 0, d = 0;
	eeprom::getHeaterPID(&p, &i, &d);

	pid.reset();

	//if (p == 0 && i == 0 && d == 0) {
		p = DEFAULT_P; i = DEFAULT_I; d = DEFAULT_D;
	//}

	pid.setPGain(p);
	pid.setIGain(i);
	pid.setDGain(d);
	pid.setTarget(0);
}

void Heater::disable(bool on) {
	if ( on )
		reset();
	is_Disabled = on;
}

void Heater::setTargetTemperature(int16_t targetTemp)
{
	int16_t maxtemp = (calibration_eeprom_offset == 2) ? MAX_HBP_TEMP : MAX_VALID_TEMP;

	if ( targetTemp > maxtemp )
		targetTemp = maxtemp;
	else if ( targetTemp < 0 )
		targetTemp = 0;

	if ( is_Paused ) {
        if (targetTemp < pid.getTarget())
            pid.setTarget(targetTemp);
        if (targetTemp == 0)
            is_Paused = false;
        pausedSetTemperature = targetTemp;
        return;
	}

	if ( targetTemp > 0 ) {
		//BOARD_STATUS_CLEAR(Motherboard::STATUS_HEAT_INACTIVE_SHUTDOWN);
	}

	newTargetReached = false;

	if ( hasFailed() || isDisabled() ) {
		pid.setTarget(0);
		return;
	}

	if ( heatTimingCheck ) {
		startTemp = currentTemperature;
		progressChecked = false;
		valueFailCount = 0;

		/// start a progress timer to verify we are getting temp change over time.
		if(targetTemp > HEAT_FAIL_CHECK_THRESHOLD){
			/// if the current temp is greater than a (low) threshold, don't check the heating up time, because
			/// we've already done that to get to this temperature
			if((targetTemp > (currentTemperature + HEAT_PROGRESS_THRESHOLD)) && (currentTemperature < HEAT_CHECKED_THRESHOLD)){
                heatProgressTimer.start(HEAT_PROGRESS_TIME);
            }else{
                heatProgressTimer = Timeout();
            }

			heatingUpTimer.start(HEAT_UP_TIME);
		}
		else{
			heatingUpTimer = Timeout();
			heatProgressTimer = Timeout();
		}
	}
	pid.setTarget(targetTemp);
}

/// We now define target hysteresis, used as PID over/under range.
#define TARGET_HYSTERESIS 2

/// Returns true if the current PID temperature is within tolerance of the expected current temperature.
bool Heater::hasReachedTargetTemperature()
{
	// MBI firmware does not check for a BAD_TEMPERATURE
	// that can lead to this routine returning TRUE when
	// BAD_TEMPERATURE is a large value such as 1024

	// Not clear offhand what this should return for a disabled heater.
	//
	// If we do not test is_disabled, then this call will return true since
	// for a disabled heater, newTargetReached == false, current_temperatute == 0,
	// and pid.getTarget() == 0.
	//
	// However if some code is errantly waiting for a disabled heater to
	// come to temp, then that is a bug that the code is inquiring about
	// a disabled heater.  So let's return false here and if it causes
	// a problem upstack, then fix the broken upstack logic.

	if ( is_Paused || is_Disabled || currentTemperature >= BAD_TEMPERATURE)
		return false;

	// flag temperature reached so that PID variations don't trigger this a second time
	if ( !newTargetReached ) {
		if ( (currentTemperature >= (pid.getTarget() - TARGET_HYSTERESIS)) && (currentTemperature <= (pid.getTarget() + TARGET_HYSTERESIS)) ) {
			newTargetReached = true;
		}
	}
	return newTargetReached;
}

bool Heater::isHeating(){
    return (pid.getTarget() > 0) && !hasReachedTargetTemperature() && !failState;
}

bool Heater::isCooling(){
	return (currentTemperature > getSetTemperature()) && !hasReachedTargetTemperature() && !failState;
}

int16_t Heater::getDelta(){
	int target = pid.getTarget();
	int temp   = (int)(0.5 + sensor.getTemperature());
	return (int16_t)((target > temp) ? target - temp : temp - target);
}

void Heater::Pause(bool on){

	// don't pause / un-pause again
	if(is_Disabled || is_Paused == on)
		return;

	// don't pause if heater is not on
	if (on && !isHeating())
		return;

	if ( on ) {
		//set output to zero
		pausedSetTemperature = getSetTemperature();
		// Don't accidentally set the temp to 1024!
		setTargetTemperature(currentTemperature < BAD_TEMPERATURE ? currentTemperature : 0);
		// clear heatup timers
		heatingUpTimer = Timeout();
		heatProgressTimer = Timeout();
		// clear reached target temperature
		newTargetReached = false;
		is_Paused = true; // do after get_set_temperature()
	}else{
		// restart heatup
		is_Paused = false;
		setTargetTemperature(pausedSetTemperature);
	}
}

void Heater::setOutput(uint8_t value)
{
    outputvalue = value;
	element.setHeatingElement(value);
}

// mark as failed and report to motherboard for user messaging
void Heater::fail()
{
	failState = true;
	setTargetTemperature(0);
	setOutput(0);
	//Motherboard::getBoard().heaterFail(fail_mode, calibration_eeprom_offset);
}

void Heater::manageHeater()
{
    if ( is_Disabled )
    return;

    sensor.update();

    float fp_current_temp = sensor.getTemperature();
    currentTemperature = (int)(0.5 + fp_current_temp);

    if (!is_Paused){
        uint8_t oldValueCount = valueFailCount;
        // check that the the heater isn't reading above the maximum allowable temp
        if (currentTemperature > HEATER_CUTOFF_TEMPERATURE) {
            valueFailCount++;

            if (valueFailCount > SENSOR_MAX_BAD_READINGS) {
                failMode = HEATER_FAIL_SOFTWARE_CUTOFF;
                fail();
                return;
            }
        }
        // check that the heater is heating up after target is set
        if(!progressChecked){
            if(heatProgressTimer.hasElapsed()){
                if(currentTemperature < (startTemp + HEAT_PROGRESS_THRESHOLD )){
                    valueFailCount++;

                    if (valueFailCount > SENSOR_MAX_BAD_READINGS) {
                        failMode = HEATER_FAIL_NOT_HEATING;
                        fail();
                        return;
                    }
                }else
                    progressChecked = true;
            }
        }
        // check that the heater temperature does not drop when still set to high temp
        if(heatingUpTimer.hasElapsed() && hasReachedTargetTemperature() && (currentTemperature < (pid.getTarget() - HEAT_FAIL_THRESHOLD))){
					valueFailCount++;

					if (valueFailCount > SENSOR_MAX_BAD_READINGS) {
						failMode = HEATER_FAIL_DROPPING_TEMP;
						fail();
						return;
					}
        }
        // if no bad heat reads have occured, clear the fail count
        // we don't want this to add up continually forever
        if(valueFailCount == oldValueCount)
            valueFailCount = 0;
    }

	if (failState) {
		return;
	}

	int delta = pid.getTarget() - currentTemperature;

	if ( bypassingPID && (delta < PID_BYPASS_DELTA) ) {
		bypassingPID = false;
		pid.reset_state();
	}else if ( !bypassingPID && (delta > PID_BYPASS_DELTA + 10) ) {
		bypassingPID = true;
	}

	if ( bypassingPID )
		setOutput(255);
	else {
		int mv = 0;
		if ( pid.getTarget() != 0 ) {
			mv = pid.calculate(fp_current_temp);
			// offset value to compensate for heat bleed-off.
			// There are probably more elegant ways to do this,
			// but this works pretty well.
#if HEATER_OFFSET_ADJUSTMENT
			mv += HEATER_OFFSET_ADJUSTMENT;
#endif
			// clamp value
			if (mv < 0) mv = 0;
			else if (mv > 255) mv = 255;
		}
		setOutput(mv);
	}
}


ExtruderHeatingElement          extruder_HeatingElement[3] = {ExtruderHeatingElement(0),ExtruderHeatingElement(1),ExtruderHeatingElement(2)};
Thermistor                      extruder_Thermistor[3] = {Thermistor(THERMISTOR_HEATER_0),Thermistor(THERMISTOR_HEATER_1),Thermistor(THERMISTOR_HEATER_2)};
Heater                          extruderHeater[3] = {Heater(extruder_Thermistor[0], extruder_HeatingElement[0], true),Heater(extruder_Thermistor[1], extruder_HeatingElement[1], true),Heater(extruder_Thermistor[2], extruder_HeatingElement[2], true)};

BuildPlatformHeatingElement     bed_HeatingElement;
Thermistor                      bed_Thermistor(THERMISTOR_BED);
Heater                          bedHeater(bed_Thermistor, bed_HeatingElement, true);

