#include "EepromMap.h"

#include "planner.h"
#include "command.h"

#define APP_EEPROM_VERSION      "V11"


namespace eeprom{

static void setDefaultPID(void);

void resetManufacturerEEPROM()
{

}

void resetUserEEPROM()
{
    char ver[4] = "000";
    EEPROM_WRITE_VAR(eeprom_offsets::EEPROM_VERSION, ver); // invalidate data first

    setDefault_plan();
    setDefault_command();
    setDefaultPID();

    char ver2[4] = APP_EEPROM_VERSION;
    EEPROM_WRITE_VAR(eeprom_offsets::EEPROM_VERSION, ver2);
}

void fullResetEEPROM()
{
    resetManufacturerEEPROM();
    resetUserEEPROM();
}

void init()
{
    char stored_version[4];
    char eeprom_version[4] = APP_EEPROM_VERSION;
    EEPROM_READ_VAR(eeprom_offsets::EEPROM_VERSION, stored_version);

    if (strncmp(eeprom_version,stored_version,3) == 0)
        return;

    resetUserEEPROM();
}

void getHeaterPID(float* Kp, float* Ki, float* Kd)
{
    uint16_t location = eeprom_offsets::HEATER_PID;
    eeprom_read_block((uint8_t*)Kp, location, sizeof(float));
    location += sizeof(float);
    eeprom_read_block((uint8_t*)Ki, location, sizeof(float));
    location += sizeof(float);
    eeprom_read_block((uint8_t*)Kd, location, sizeof(float));
}

static void setDefaultPID(void)
{
    float p = DEFAULT_P;
    uint16_t location = eeprom_offsets::HEATER_PID;
    EEPROM_WRITE_VAR(location, p);
    float i = DEFAULT_I;
    location += sizeof(float);
    EEPROM_WRITE_VAR(location, i);
    float d = DEFAULT_D;
    location += sizeof(float);
    EEPROM_WRITE_VAR(location, d);
}

void setDefault_plan()
{
    float tmp1[] = DEFAULT_AXIS_STEPS_PER_UNIT;
    float tmp2[] = DEFAULT_MAX_FEEDRATE;
    long tmp3[] = DEFAULT_MAX_ACCELERATION;

    for (short i=0;i<4;i++){
        axis_steps_per_unit[i] = tmp1[i];
        max_feedrate[i] = tmp2[i];
        max_acceleration_units_per_sq_second[i] = tmp3[i];
    }

    // steps per sq second need to be updated to agree with the units per sq second
    reset_acceleration_rates();

    acceleration = DEFAULT_ACCELERATION;
    retract_acceleration = DEFAULT_RETRACT_ACCELERATION;
    minimumfeedrate = DEFAULT_MINIMUMFEEDRATE;
    minsegmenttime = DEFAULT_MINSEGMENTTIME;
    mintravelfeedrate = DEFAULT_MINTRAVELFEEDRATE;
    max_xy_jerk = DEFAULT_XYJERK;
    max_z_jerk = DEFAULT_ZJERK;
    max_e_jerk = DEFAULT_EJERK;

    EEPROM_WRITE_VAR(eeprom_offsets::AXIS_STEPS_PER_UNIT, axis_steps_per_unit);
    EEPROM_WRITE_VAR(eeprom_offsets::MAX_FEEDRATE, max_feedrate);
    EEPROM_WRITE_VAR(eeprom_offsets::MAX_ACCELERATION, max_acceleration_units_per_sq_second);
    EEPROM_WRITE_VAR(eeprom_offsets::ACCELERATION, acceleration);
    EEPROM_WRITE_VAR(eeprom_offsets::RETRACT_ACCELERATION, retract_acceleration);
    EEPROM_WRITE_VAR(eeprom_offsets::MINIMUMFEEDRATE, minimumfeedrate);
    EEPROM_WRITE_VAR(eeprom_offsets::MINTRAVELFEEDRATE, mintravelfeedrate);
    EEPROM_WRITE_VAR(eeprom_offsets::MINSEGMENTTIME, minsegmenttime);
    EEPROM_WRITE_VAR(eeprom_offsets::MAX_XYJERK, max_xy_jerk);
    EEPROM_WRITE_VAR(eeprom_offsets::MAX_ZJERK, max_z_jerk);
    EEPROM_WRITE_VAR(eeprom_offsets::MAX_EJERK, max_e_jerk);
}

void eeprom_plan_init()
{
    EEPROM_READ_VAR(eeprom_offsets::AXIS_STEPS_PER_UNIT, axis_steps_per_unit);
    EEPROM_READ_VAR(eeprom_offsets::MAX_FEEDRATE, max_feedrate);
    EEPROM_READ_VAR(eeprom_offsets::MAX_ACCELERATION, max_acceleration_units_per_sq_second);
    EEPROM_READ_VAR(eeprom_offsets::ACCELERATION, acceleration);
    EEPROM_READ_VAR(eeprom_offsets::RETRACT_ACCELERATION, retract_acceleration);
    EEPROM_READ_VAR(eeprom_offsets::MINIMUMFEEDRATE, minimumfeedrate);
    EEPROM_READ_VAR(eeprom_offsets::MINTRAVELFEEDRATE, mintravelfeedrate);
    EEPROM_READ_VAR(eeprom_offsets::MINSEGMENTTIME, minsegmenttime);
    EEPROM_READ_VAR(eeprom_offsets::MAX_XYJERK, max_xy_jerk);
    EEPROM_READ_VAR(eeprom_offsets::MAX_ZJERK, max_z_jerk);
    EEPROM_READ_VAR(eeprom_offsets::MAX_EJERK, max_e_jerk);

    reset_acceleration_rates();
}

void setDefault_command()
{
    command::add_homing[0] = command::add_homing[1] = command::add_homing[2] = 0;
    command::zprobe_zoffset = 0;
    EEPROM_WRITE_VAR(eeprom_offsets::ADD_HOMING, command::add_homing);
    EEPROM_WRITE_VAR(eeprom_offsets::ZPROBE_ZOFFSET,command::zprobe_zoffset);
}

void eeprom_command_init()
{
    EEPROM_READ_VAR(eeprom_offsets::ADD_HOMING, command::add_homing);
    EEPROM_READ_VAR(eeprom_offsets::ZPROBE_ZOFFSET,command::zprobe_zoffset);
}

void setCustom_addHoming(float* add_homing)
{
    EEPROM_WRITE_VAR(eeprom_offsets::ADD_HOMING, command::add_homing);
}

}
