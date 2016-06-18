#include "JawsBoard.h"
#include "SerialPort.h"
#include "Temperature.h"
#include "watchdog.h"
#include "FilamentDetect.h"
#include "Estop.h"
#include "PiezoAudio.h"


#define SAMPLE_INTERVAL_MILLIS_PLATFORM     (250L)
#define SAMPLE_INTERVAL_MILLIS_EXTRUDER     (250L)

/// Instantiate static motherboard instance
Jawsboard Jawsboard::jawsboard;

Jawsboard::Jawsboard():
    using_platform(false),
    tool_count(1),
    pstop_enabled(true)
{
    pExtruder_Heater[0] = &extruderHeater[0];
    pExtruder_Heater[1] = &extruderHeater[1];
    pExtruder_Heater[2] = &extruderHeater[2];
    pBed_Heater = &bedHeater;

    pExtruder_Fan = &pwmfan_one;
    pExtra_Fan = &pwmfan_two;
}

void Jawsboard::init()
{
    TemperatureManageInit();

    piezoaudio.init();
    piezoaudio.playTune(TUNE_SAILFISH_STARTUP);
    //watchdog_init();

    reset(true);
}

void Jawsboard::reset(bool hard_reset)
{
    using_platform = isUsingPlatform();
    tool_count =  getToolCount();

    pBed_Heater->reset();
    platform_timeout.start(SAMPLE_INTERVAL_MILLIS_PLATFORM);

    pExtruder_Heater[0]->reset();
    pExtruder_Heater[1]->reset();
    pExtruder_Heater[2]->reset();
	extruder_timeout.start(SAMPLE_INTERVAL_MILLIS_EXTRUDER);

    heatersOff(true);

    if (!using_platform)
        pBed_Heater->disable(true);

    if (tool_count < 3)
        pExtruder_Heater[2]->disable(true);
    if (tool_count < 2)
        pExtruder_Heater[1]->disable(true);
}

bool Jawsboard::isUsingPlatform()
{
    return false;
}

uint8_t Jawsboard::getToolCount()
{
    return 1;
}

void Jawsboard::heatersPause(bool platform)
{
    for (uint8_t i = 0; i < tool_count; i++){
        pExtruder_Heater[i]->Pause(true);
    }
    if ( platform && using_platform ) {
        pBed_Heater->Pause(true);
    }
}

void Jawsboard::heatersResume(bool platform)
{
    for (uint8_t i = 0; i < tool_count; i++){
        pExtruder_Heater[i]->Pause(false);
    }
    if ( platform && using_platform ) {
        pBed_Heater->Pause(false);
    }
}

bool Jawsboard::heatersHasReachedTargetTemperature(bool platform)
{
    bool res = true;
    for (uint8_t i = 0; i < tool_count; i++){
        res = res && pExtruder_Heater[i]->hasReachedTargetTemperature();
    }
    if ( platform  && using_platform) {
        res = res && pBed_Heater->hasReachedTargetTemperature();
    }
    return res;
}

void Jawsboard::heatersOff(bool platform)
{
	pExtruder_Heater[0]->Pause(false);
	pExtruder_Heater[0]->setTargetTemperature(0);
	pExtruder_Heater[1]->Pause(false);
	pExtruder_Heater[1]->setTargetTemperature(0);
    pExtruder_Heater[2]->Pause(false);
	pExtruder_Heater[2]->setTargetTemperature(0);
	if ( platform ) {
        pBed_Heater->Pause(false);
        pBed_Heater->setTargetTemperature(0);
    }
}

void Jawsboard::setTargetDegExtruder(float celsius, uint8_t extruder)
{
    pExtruder_Heater[extruder]->setTargetTemperature((int16_t)celsius);
}

bool Jawsboard::isHeatingExtruder(uint8_t extruder)
{
    return pExtruder_Heater[extruder]->isHeating();
}

bool Jawsboard::isCoolingExtruder(uint8_t extruder)
{
    return pExtruder_Heater[extruder]->isCooling();
}

float Jawsboard::getCurrentDegExtruder(uint8_t extruder)
{
    return (float)pExtruder_Heater[extruder]->getCurrentTemperature();
}

float Jawsboard::getTargetDegExtruder(uint8_t extruder)
{
    return (float)pExtruder_Heater[extruder]->getSetTemperature();
}

void Jawsboard::setTargetDegBed(float celsius)
{
    pBed_Heater->setTargetTemperature((int16_t)celsius);
}

bool Jawsboard::isHeatingBed()
{
    return pBed_Heater->isHeating();
}

bool Jawsboard::isCoolingBed()
{
    return pBed_Heater->isCooling();
}

float Jawsboard::getCurrentDegBed()
{
    return (float)pBed_Heater->getCurrentTemperature();
}

float Jawsboard::getTargetDegBed()
{
    return (float)pBed_Heater->getSetTemperature();
}

int Jawsboard::getHeaterPower(int heater)
{
	if (heater<0)
		return (int)pBed_Heater->getOutput();
    return pExtruder_Heater[heater]->getOutput();
}

void Jawsboard::runboardSlice()
{
    if ( isUsingPlatform() && platform_timeout.hasElapsed() ) {
		// manage heating loops for the HBP
		pBed_Heater->manageHeater();
		platform_timeout.start(SAMPLE_INTERVAL_MILLIS_PLATFORM);
	}

    if ( extruder_timeout.hasElapsed() ) {
		// manage heating loops for the HBP
		pExtruder_Heater[0]->manageHeater();
        pExtruder_Heater[1]->manageHeater();
        pExtruder_Heater[2]->manageHeater();
		extruder_timeout.start(SAMPLE_INTERVAL_MILLIS_EXTRUDER);
	}

    piezoaudio.runPiezoSlice();
    //watchdog_reset();
}


