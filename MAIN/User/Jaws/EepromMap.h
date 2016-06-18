#ifndef EEPROMMAP_H
#define EEPROMMAP_H

#include <stdint.h>
#include "Eeprom.h"

#define DEFAULT_AXIS_STEPS_PER_UNIT   {88.573186, 88.573186, 400.0, 96.275202}  // default steps per unit for ultimaker2
#define DEFAULT_MAX_FEEDRATE          {200, 200, 20, 45}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION      {2000,2000,35,6000}    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.

#define DEFAULT_ACCELERATION          1000    // X, Y, Z and E max acceleration in mm/s^2 for printing moves
#define DEFAULT_RETRACT_ACCELERATION  1000   // X, Y, Z and E max acceleration in mm/s^2 for retracts

// The speed change that does not require acceleration (i.e. the software might assume it can be done instantaneously)
#define DEFAULT_XYJERK                20.0    // (mm/sec)
#define DEFAULT_ZJERK                 0.4     // (mm/sec)
#define DEFAULT_EJERK                 5.0    // (mm/sec)

#define DEFAULT_MINIMUMFEEDRATE       0.0     // minimum feedrate
#define DEFAULT_MINTRAVELFEEDRATE     0.0

#define DEFAULT_MINSEGMENTTIME        20000     // minimum time in microseconds that a movement needs to take if the buffer is emptied.

// Preheat Constants
#define PLA_PREHEAT_HOTEND_TEMP 180
#define PLA_PREHEAT_HPB_TEMP 70
#define PLA_PREHEAT_FAN_SPEED 0     // Insert Value between 0 and 255

#define ABS_PREHEAT_HOTEND_TEMP 240
#define ABS_PREHEAT_HPB_TEMP 100
#define ABS_PREHEAT_FAN_SPEED 0     // Insert Value between 0 and 255

//#define DEFAULT_P 7.0
//#define DEFAULT_I 0.325
//#define DEFAULT_D 36.0

#define DEFAULT_P 7
#define DEFAULT_I 0.32
#define DEFAULT_D 72

#define EEPROM_WRITE_VAR(pos, value)    eeprom_write_block((uint8_t*)&value, pos, sizeof(value))
#define EEPROM_READ_VAR(pos, value)     eeprom_read_block((uint8_t*)&value, pos, sizeof(value))

///EEPROM is split to 2 sectors
///First sector with 256 bytes is used to save those params modified after download application for Manufacturer
///Second sector with left space is used to save those params modified after download application for User
namespace eeprom_offsets{
const static uint16_t FIRMWARE_AVAILABLE                = 0x0800;
// standard UUID 8-4-4-4-12 total 128 bits
const static uint16_t MACHINE_UUID                      = 0x0804;

//char * 4
const static uint16_t EEPROM_VERSION                    = 0x0900;
//float * 4
const static uint16_t AXIS_STEPS_PER_UNIT		        = EEPROM_VERSION + 4;
//float * 4
const static uint16_t MAX_FEEDRATE                      = AXIS_STEPS_PER_UNIT + 16;
//float * 4
const static uint16_t MAX_ACCELERATION                  = MAX_FEEDRATE + 16;
//float
const static uint16_t ACCELERATION		                = MAX_ACCELERATION + 16;
//float
const static uint16_t RETRACT_ACCELERATION              = ACCELERATION + 4;
//float
const static uint16_t MINIMUMFEEDRATE                   = RETRACT_ACCELERATION + 4;
//float
const static uint16_t MINSEGMENTTIME		            = MINIMUMFEEDRATE + 4;
//float
const static uint16_t MINTRAVELFEEDRATE                 = MINSEGMENTTIME + 4;
//float
const static uint16_t MAX_XYJERK                        = MINTRAVELFEEDRATE + 4;
//float
const static uint16_t MAX_ZJERK		                    = MAX_XYJERK + 4;
//float
const static uint16_t MAX_EJERK                         = MAX_ZJERK + 4;
//float * 3
const static uint16_t ADD_HOMING                        = MAX_EJERK + 4;
//float
const static uint16_t ZPROBE_ZOFFSET                    = ADD_HOMING + 12;

//float * 3
const static uint16_t HEATER_PID                        = ZPROBE_ZOFFSET + 4;

const static uint16_t HBP_PRESENT			            = HEATER_PID + 12;

const static uint16_t PSTOP_ENABLE                      = HBP_PRESENT + 1;

}

namespace eeprom{
void resetManufacturerEEPROM();
void resetUserEEPROM();
void fullResetEEPROM();
void init();

void getHeaterPID(float* Kp, float* Ki, float* Kd);

void setDefault_plan();
void setDefault_command();

void eeprom_plan_init();
void eeprom_command_init();

void setCustom_addHoming(float* add_homing);

}

#endif /* EEPROMMAP_H */
