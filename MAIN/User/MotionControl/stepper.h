/**
  ******************************************************************************
  * @file    stepper.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    18-May-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef STEPPER_H
#define STEPPER_H

#include "Configuration.h"
#include "planner.h"
#include "hw_gpio.h"

#define X_STEP_PIN      PA_5
#define X_DIR_PIN       PA_6
#define X_ENABLE_PIN    PA_4
#define X_MIN_PIN       PE_1
#define X_MAX_PIN       PE_2

#define Y_STEP_PIN      PC_4
#define Y_DIR_PIN       PC_5
#define Y_ENABLE_PIN    PA_7
#define Y_MIN_PIN       PE_3
#define Y_MAX_PIN       PE_4

#define Z_STEP_PIN      PB_1
#define Z_DIR_PIN       PB_2
#define Z_ENABLE_PIN    PB_0
#define Z_MIN_PIN       PE_5
#define Z_MAX_PIN       PE_6

#define E0_STEP_PIN     PB_14
#define E0_DIR_PIN      PB_15
#define E0_ENABLE_PIN   PB_13

#if EXTRUDER_COUNT > 1
#define E1_STEP_PIN     PE_1
#define E1_DIR_PIN      PE_2
#define E1_ENABLE_PIN   PE_0
#endif

#if EXTRUDER_COUNT > 2
#define E2_STEP_PIN     PE_4
#define E2_DIR_PIN      PE_5
#define E2_ENABLE_PIN   PE_3
#endif

//By default pololu step drivers require an active high signal. However, some high power drivers require an active low signal as step.
#define INVERT_X_STEP_PIN   false
#define INVERT_Y_STEP_PIN   false
#define INVERT_Z_STEP_PIN   false
#define INVERT_E_STEP_PIN   false

#define INVERT_X_DIR        true    // for Mendel set to false, for Orca set to true
#define INVERT_Y_DIR        false    // for Mendel set to true, for Orca set to false
#define INVERT_Z_DIR        false     // for Mendel set to false, for Orca set to true

#define INVERT_E0_DIR       true   // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E1_DIR       false    // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E2_DIR       false   // for direct drive extruder v9 set to true, for geared extruder set to false

// The pullups are needed if you directly connect a mechanical endswitch between the signal and ground pins.
const bool X_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
const bool Y_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
const bool Z_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
const bool X_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
const bool Y_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
const bool Z_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.

#if EXTRUDER_COUNT > 2
    #define WRITE_E_STEP(v) { if(current_block->active_extruder == 2) { WRITE(E2_STEP_PIN, v); } else { if(current_block->active_extruder == 1) { WRITE(E1_STEP_PIN, v); } else { WRITE(E0_STEP_PIN, v); }}}
    #define NORM_E_DIR() { if(current_block->active_extruder == 2) { WRITE(E2_DIR_PIN, !INVERT_E2_DIR); } else { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); }}}
    #define REV_E_DIR() { if(current_block->active_extruder == 2) { WRITE(E2_DIR_PIN, INVERT_E2_DIR); } else { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, INVERT_E0_DIR); }}}
#elif EXTRUDER_COUNT > 1
    #define WRITE_E_STEP(v) { if(current_block->active_extruder == 1) { WRITE(E1_STEP_PIN, v); } else { WRITE(E0_STEP_PIN, v); }}
    #define NORM_E_DIR() { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); }}
    #define REV_E_DIR() { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, INVERT_E0_DIR); }}
#else
    #define WRITE_E_STEP(v) WRITE(E0_STEP_PIN, v)
    #define NORM_E_DIR() WRITE(E0_DIR_PIN, !INVERT_E0_DIR)
    #define REV_E_DIR() WRITE(E0_DIR_PIN, INVERT_E0_DIR)
#endif

#if defined(DUAL_X_CARRIAGE) && defined(X_ENABLE_PIN) && X_ENABLE_PIN > -1 \
    && defined(X2_ENABLE_PIN) && X2_ENABLE_PIN > -1
  #define  enable_x() do { WRITE(X_ENABLE_PIN, X_ENABLE_ON); WRITE(X2_ENABLE_PIN, X_ENABLE_ON); } while (0)
  #define disable_x() do { WRITE(X_ENABLE_PIN,!X_ENABLE_ON); WRITE(X2_ENABLE_PIN,!X_ENABLE_ON); command::axis_known_position[X_AXIS] = false; } while (0)
#elif defined(X_ENABLE_PIN) && X_ENABLE_PIN > -1
  #define  enable_x() WRITE(X_ENABLE_PIN, X_ENABLE_ON)
  #define disable_x() { WRITE(X_ENABLE_PIN,!X_ENABLE_ON); command::axis_known_position[X_AXIS] = false; }
#else
  #define enable_x() ;
  #define disable_x() ;
#endif

#if defined(Y_ENABLE_PIN) && Y_ENABLE_PIN > -1
  #ifdef Y_DUAL_STEPPER_DRIVERS
    #define  enable_y() { WRITE(Y_ENABLE_PIN, Y_ENABLE_ON); WRITE(Y2_ENABLE_PIN,  Y_ENABLE_ON); }
    #define disable_y() { WRITE(Y_ENABLE_PIN,!Y_ENABLE_ON); WRITE(Y2_ENABLE_PIN, !Y_ENABLE_ON); command::axis_known_position[Y_AXIS] = false; }
  #else
    #define  enable_y() WRITE(Y_ENABLE_PIN, Y_ENABLE_ON)
    #define disable_y() { WRITE(Y_ENABLE_PIN,!Y_ENABLE_ON); command::axis_known_position[Y_AXIS] = false; }
  #endif
#else
  #define enable_y() ;
  #define disable_y() ;
#endif

#if defined(Z_ENABLE_PIN) && Z_ENABLE_PIN > -1
  #ifdef Z_DUAL_STEPPER_DRIVERS
    #define  enable_z() { WRITE(Z_ENABLE_PIN, Z_ENABLE_ON); WRITE(Z2_ENABLE_PIN, Z_ENABLE_ON); }
    #define disable_z() { WRITE(Z_ENABLE_PIN,!Z_ENABLE_ON); WRITE(Z2_ENABLE_PIN,!Z_ENABLE_ON); command::axis_known_position[Z_AXIS] = false; }
  #else
    #define  enable_z() WRITE(Z_ENABLE_PIN, Z_ENABLE_ON)
    #define disable_z() { WRITE(Z_ENABLE_PIN,!Z_ENABLE_ON); command::axis_known_position[Z_AXIS] = false; }
  #endif
#else
  #define enable_z() ;
  #define disable_z() ;
#endif

#if defined(E0_ENABLE_PIN) && (E0_ENABLE_PIN > -1)
  #define enable_e0() WRITE(E0_ENABLE_PIN, E_ENABLE_ON)
  #define disable_e0() WRITE(E0_ENABLE_PIN,!E_ENABLE_ON)
#else
  #define enable_e0()  /* nothing */
  #define disable_e0() /* nothing */
#endif

#if (EXTRUDER_COUNT > 1) && defined(E1_ENABLE_PIN) && (E1_ENABLE_PIN > -1)
  #define enable_e1() WRITE(E1_ENABLE_PIN, E_ENABLE_ON)
  #define disable_e1() WRITE(E1_ENABLE_PIN,!E_ENABLE_ON)
#else
  #define enable_e1()  /* nothing */
  #define disable_e1() /* nothing */
#endif

#if (EXTRUDER_COUNT > 2) && defined(E2_ENABLE_PIN) && (E2_ENABLE_PIN > -1)
  #define enable_e2() WRITE(E2_ENABLE_PIN, E_ENABLE_ON)
  #define disable_e2() WRITE(E2_ENABLE_PIN,!E_ENABLE_ON)
#else
  #define enable_e2()  /* nothing */
  #define disable_e2() /* nothing */
#endif

// Initialize and start the stepper motor subsystem
void st_init();

// Block until all buffered steps are executed
void st_synchronize();

// Set current position in steps
void st_set_position(const long &x, const long &y, const long &z, const long &e);
void st_set_e_position(const long &e);

// Get current position in steps
long st_get_position(uint8_t axis);

#ifdef ENABLE_AUTO_BED_LEVELING
// Get current position in mm
float st_get_position_mm(uint8_t axis);
#endif  //ENABLE_AUTO_BED_LEVELING

// The stepper subsystem goes to sleep when it runs out of things to execute. Call this
// to notify the subsystem that it is time to go to work.
void st_wake_up();


void checkHitEndstops(); //call from somewhere to create an serial error message with the locations the endstops where hit, in case they were triggered
void endstops_hit_on_purpose(); //avoid creation of the message, i.e. after homing and before a routine call of checkHitEndstops();

void enable_endstops(bool check); // Enable/disable endstop checking

void checkStepperErrors(); //Print errors detected by the stepper

void finishAndDisableSteppers();

extern block_t *current_block;  // A pointer to the block currently being traced

void quickStop();

void digitalPotWrite(int address, int value);
void microstep_ms(uint8_t driver, int8_t ms1, int8_t ms2);
void microstep_mode(uint8_t driver, uint8_t stepping);
void digipot_init();
void microstep_init();
void microstep_readings();

void Stepper_Time_IRQHandler(void);

#endif /* STEPPER_H */
