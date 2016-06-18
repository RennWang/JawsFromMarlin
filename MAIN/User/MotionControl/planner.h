/**
  ******************************************************************************
  * @file    planner.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    9-April-2015
  * @brief   This module is to be considered a sub-module of stepper.c. Please don't include
  *          this file from any other module
  ******************************************************************************
  */

#ifndef PLANNER_H
#define PLANNER_H

#include "Jaws_main.h"
#include "Jawsdef.h"
#include "Configuration.h"

#ifdef ENABLE_AUTO_BED_LEVELING
#include "vector_3.h"
#endif // ENABLE_AUTO_BED_LEVELING



//this prevents dangerous Extruder moves, i.e. if the temperature is under the limit can be software-disabled for whatever purposes by
#define PREVENT_DANGEROUS_EXTRUDE
#define PREVENT_LENGTHY_EXTRUDE     //if PREVENT_DANGEROUS_EXTRUDE is on, you can still disable (uncomment) very long bits of extrusion separately.
#define EXTRUDE_MINTEMP             170
#define EXTRUDE_MAXLENGTH           1000.0 //prevent extrusion of very large distances.

#define FAN_KICKSTART_TIME          100

#define SLOWDOWN
#define MINIMUM_PLANNER_SPEED       0.05// (mm/sec)



#define BLOCK_BUFFER_SIZE 16

// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
  // Fields used by the bresenham algorithm for tracing the line
    long steps_x, steps_y, steps_z, steps_e;  // Step count along each axis
    unsigned long step_event_count;           // The number of step events required to complete this block
    long accelerate_until;                    // The index of the step event on which to stop acceleration
    long decelerate_after;                    // The index of the step event on which to start decelerating
    long acceleration_rate;                   // The acceleration rate used for acceleration calculation
    unsigned char direction_bits;             // The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
    unsigned char active_extruder;            // Selects the active extruder

    // Fields used by the motion planner to manage acceleration
    float nominal_speed;                               // The nominal speed for this block in mm/sec
    float entry_speed;                                 // Entry speed at previous-current junction in mm/sec
    float max_entry_speed;                             // Maximum allowable junction entry speed in mm/sec
    float millimeters;                                 // The total travel of this block in mm
    float acceleration;                                // acceleration mm/sec^2
    unsigned char recalculate_flag;                    // Planner flag to recalculate trapezoids on entry junction
    unsigned char nominal_length_flag;                 // Planner flag for nominal speed always reached

    // Settings for the trapezoid generator
    unsigned long nominal_rate;                        // The nominal step rate for this block in step_events/sec
    unsigned long initial_rate;                        // The jerk-adjusted step rate at start of block
    unsigned long final_rate;                          // The minimal rate at exit
    unsigned long acceleration_st;                     // acceleration steps/sec^2
    unsigned long fan_speed;
    volatile char busy;
} block_t;

// Initialize the motion plan subsystem
void plan_init();

#ifdef ENABLE_AUTO_BED_LEVELING
// this holds the required transform to compensate for bed level
extern matrix_3x3 plan_bed_level_matrix;
// Add a new linear movement to the buffer. x, y and z is the signed, absolute target position in millimaters. Feed rate specifies the speed of the motion.
void plan_buffer_line(float x, float y, float z, const float &e, float feed_rate, const uint8_t &extruder);
// Get the position applying the bed level matrix if enabled
vector_3 plan_get_position();
// Set position. Used for G92 instructions.
void plan_set_position(float x, float y, float z, const float &e);
#else
void plan_buffer_line(const float &x, const float &y, const float &z, const float &e, float feed_rate, const uint8_t &extruder);
void plan_set_position(const float &x, const float &y, const float &z, const float &e);
#endif // ENABLE_AUTO_BED_LEVELING


void plan_set_e_position(const float &e);

void check_axes_activity();

uint8_t movesplanned(); //return the nr of buffered moves

extern unsigned long minsegmenttime;
extern float max_feedrate[NUM_AXIS]; // set the max speeds
extern float axis_steps_per_unit[NUM_AXIS];
extern unsigned long max_acceleration_units_per_sq_second[NUM_AXIS]; // Use M201 to override by software
extern float minimumfeedrate;
extern float acceleration;         // Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
extern float retract_acceleration; //  mm/s^2   filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
extern float max_xy_jerk; //speed than can be stopped at once, if i understand correctly.
extern float max_z_jerk;
extern float max_e_jerk;
extern float mintravelfeedrate;
extern unsigned long axis_steps_per_sqr_second[NUM_AXIS];

extern block_t block_buffer[BLOCK_BUFFER_SIZE];            // A ring buffer for motion instfructions
extern volatile unsigned char block_buffer_head;           // Index of the next block to be pushed
extern volatile unsigned char block_buffer_tail;

// Called when the current block is no longer needed. Discards the block and makes the memory
// availible for new blocks.
FORCE_INLINE void plan_discard_current_block()
{
  if (block_buffer_head != block_buffer_tail) {
    block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);
  }
}

// Gets the current block. Returns NULL if buffer empty
FORCE_INLINE block_t *plan_get_current_block()
{
  if (block_buffer_head == block_buffer_tail) {
    return(NULL);
  }
  block_t *block = &block_buffer[block_buffer_tail];
  block->busy = true;
  return(block);
}

// Returns true if the buffer has a queued block, false otherwise
FORCE_INLINE bool blocks_queued()
{
    return (block_buffer_head != block_buffer_tail);
}

#ifdef PREVENT_DANGEROUS_EXTRUDE
void set_extrude_min_temp(float temp);
#endif

void reset_acceleration_rates();

#endif
