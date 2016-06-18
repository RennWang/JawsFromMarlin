#ifndef CONFIGURATION_H
#define CONFIGURATION_H

//Extruder and Heatbed defines for Heaters
#define EXTRUDER_COUNT      1
#define HEATBED_ENABLED

#ifdef HEATBED_ENABLED
    #define HEATER_COUNT       (EXTRUDER_COUNT + 1)
#else
    #define HEATER_COUNT       EXTRUDER_COUNT
#endif

#if (EXTRUDER_COUNT == 2)
    #define EXTRUDER_2_ENABLED
#endif
#if (EXTRUDER_COUNT == 3)
    #define EXTRUDER_2_ENABLED
    #define EXTRUDER_3_ENABLED
#endif

// The axis order in all axis related arrays is X, Y, Z, E
#define NUM_AXIS            4

// For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1
#define X_ENABLE_ON         0
#define Y_ENABLE_ON         0
#define Z_ENABLE_ON         0
#define E_ENABLE_ON         0 // For all extruders

// Disables axis when it's not being used
#define DISABLE_X           false
#define DISABLE_Y           false
#define DISABLE_Z           false
#define DISABLE_E           false // For all extruders

#define min_software_endstops   true // If true, axis won't move to coordinates less than HOME_POS.
#define max_software_endstops   true  // If true, axis won't move to coordinates greater than the defined lengths below.

#define EXTRUDER_OFFSET_X       {0.0, 20.00, 0.0}  // (in mm) for each extruder, offset of the hotend on the X axis
#define EXTRUDER_OFFSET_Y       {0.0, 0.0, 20.00}   // (in mm) for each extruder, offset of the hotend on the Y axis

#define ENABLE_AUTO_BED_LEVELING

#define X_PROBE_OFFSET_FROM_EXTRUDER    -25
#define Y_PROBE_OFFSET_FROM_EXTRUDER    -29
#define Z_PROBE_OFFSET_FROM_EXTRUDER    -12.35

// Sets direction of endstops when homing; 1=MAX, -1=MIN
#define X_HOME_DIR      1
#define Y_HOME_DIR      -1
#define Z_HOME_DIR      1

// Travel limits after homing
#define X_MAX_POS       140
#define X_MIN_POS       0
#define Y_MAX_POS       140
#define Y_MIN_POS       0
#define Z_MAX_POS       135
#define Z_MIN_POS       0

#define X_MAX_LENGTH (X_MAX_POS - X_MIN_POS)
#define Y_MAX_LENGTH (Y_MAX_POS - Y_MIN_POS)
#define Z_MAX_LENGTH (Z_MAX_POS - Z_MIN_POS)

//homing hits the endstop, then retracts by this distance, before it tries to slowly bump again:
#define X_HOME_RETRACT_MM   5
#define Y_HOME_RETRACT_MM   5
#define Z_HOME_RETRACT_MM   5 // deltas need the same for all three axis

#if X_HOME_DIR == -1
    #ifdef BED_CENTER_AT_0_0
      #define X_HOME_POS X_MAX_LENGTH * -0.5
    #else
      #define X_HOME_POS X_MIN_POS
    #endif //BED_CENTER_AT_0_0
#else
    #ifdef BED_CENTER_AT_0_0
      #define X_HOME_POS X_MAX_LENGTH * 0.5
    #else
      #define X_HOME_POS X_MAX_POS
    #endif //BED_CENTER_AT_0_0
#endif //X_HOME_DIR == -1

//Y axis
#if Y_HOME_DIR == -1
    #ifdef BED_CENTER_AT_0_0
      #define Y_HOME_POS Y_MAX_LENGTH * -0.5
    #else
      #define Y_HOME_POS Y_MIN_POS
    #endif //BED_CENTER_AT_0_0
#else
    #ifdef BED_CENTER_AT_0_0
      #define Y_HOME_POS Y_MAX_LENGTH * 0.5
    #else
      #define Y_HOME_POS Y_MAX_POS
    #endif //BED_CENTER_AT_0_0
#endif //Y_HOME_DIR == -1

// Z axis
#if Z_HOME_DIR == -1 //BED_CENTER_AT_0_0 not used
    #define Z_HOME_POS Z_MIN_POS
  #else
    #define Z_HOME_POS Z_MAX_POS
#endif //Z_HOME_DIR == -1

//#define ENDSTOPS_ONLY_FOR_HOMING // If defined the endstops will only be used for homing

#define QUICK_HOME

#endif /* CONFIGURATION_H */
