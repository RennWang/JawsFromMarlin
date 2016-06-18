/**
  ******************************************************************************
  * @file    stepper.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    18-May-2015
  * @brief
  *
  ******************************************************************************
  */
#include "stepper.h"

#include "Configuration.h"
#include "hw_gpio.h"
#include "speed_lookuptable.h"
#include "Jawsboard.h"
#include "Command.h"
#include "digpot_mcp44xx.h"

#define MAX_STEP_FREQUENCY          40000         /// Max step rate
#define MICROSTEP_MODES             {16,16,16,16,16} /// Microstep setting (Only functional when stepper driver microstep pins are connected to MCU).
#define DIGIPOT_MOTOR_CURRENT       {250,250,250,250}

//===========================================================================
//=============================public variables  ============================
//===========================================================================
block_t *current_block;  // A pointer to the block currently being traced


///===========================================================================
///=============================private variables ============================
///===========================================================================
/// Variables used by The Stepper Driver Interrupt
static unsigned char out_bits;        // The next stepping-bits to be output
static long counter_x, counter_y, counter_z, counter_e;    // Counter variables for the bresenham line tracer

static volatile unsigned long step_events_completed; // The number of step events executed in the current block

static long acceleration_time, deceleration_time;
static unsigned short acc_step_rate; // needed for deccelaration start point
static char step_loops;
static unsigned short timer_nominal;
static char step_loops_nominal;

static volatile long endstops_trigsteps[3]={0,0,0};     ///checkHitEndstops()函数 和 中断函数当中都使用到了
static volatile bool endstop_x_hit=false;               ///
static volatile bool endstop_y_hit=false;               ///
static volatile bool endstop_z_hit=false;               ///

static bool abort_on_endstop_hit = false;

static bool old_x_min_endstop=false;
static bool old_x_max_endstop=false;
static bool old_y_min_endstop=false;
static bool old_y_max_endstop=false;
static bool old_z_min_endstop=false;
static bool old_z_max_endstop=false;

static bool check_endstops = true;

static volatile long count_position[NUM_AXIS] = { 0, 0, 0, 0};
static volatile signed char count_direction[NUM_AXIS] = { 1, 1, 1, 1};

//===========================================================================
//=============================functions         ============================
//===========================================================================

#define CHECK_ENDSTOPS  if(check_endstops)

// intRes = intIn1 * intIn2 >> 16
#define MultiU16X8toH16(intRes, charIn1, intIn2)    do { (intRes) = (uint32_t(charIn1) * uint32_t(intIn2)) >> 16; } while(0)
// intRes = longIn1 * longIn2 >> 24
#define MultiU24X24toH16(intRes, longIn1, longIn2)  do { (intRes) = (uint64_t(longIn1) * uint64_t(longIn2)) >> 24; } while(0)

#define ENABLE_STEPPER_DRIVER_INTERRUPT()   HAL_TIM_Base_Start_IT(&TIM_STEPPER_HandleStruct)
#define DISABLE_STEPPER_DRIVER_INTERRUPT()  HAL_TIM_Base_Stop_IT(&TIM_STEPPER_HandleStruct)

#define StepperTimerAutoReload(value)       __HAL_TIM_SetAutoreload(&TIM_STEPPER_HandleStruct,(value));


static TIM_HandleTypeDef    TIM_STEPPER_HandleStruct;

static void StepperTimerInit();
static void StepperControlIRQHandler();

void checkHitEndstops()
{
    if( endstop_x_hit || endstop_y_hit || endstop_z_hit) {
        SERIAL_ECHO_START;
        SERIAL_ECHO(MSG_ENDSTOPS_HIT);

        if(endstop_x_hit) {
            SERIAL_ECHOPAIR(" X:",(float)endstops_trigsteps[X_AXIS]/axis_steps_per_unit[X_AXIS]);
        }
        if(endstop_y_hit) {
            SERIAL_ECHOPAIR(" Y:",(float)endstops_trigsteps[Y_AXIS]/axis_steps_per_unit[Y_AXIS]);
        }
        if(endstop_z_hit) {
            SERIAL_ECHOPAIR(" Z:",(float)endstops_trigsteps[Z_AXIS]/axis_steps_per_unit[Z_AXIS]);
        }

        SERIAL_ECHOLN("");
        endstop_x_hit=false;
        endstop_y_hit=false;
        endstop_z_hit=false;
        if (abort_on_endstop_hit){
            quickStop();
            Jawsboard::getBoard().setTargetDegExtruder(0,0);
            Jawsboard::getBoard().setTargetDegExtruder(0,1);
            Jawsboard::getBoard().setTargetDegExtruder(0,2);
        }
    }
}

void endstops_hit_on_purpose()
{
    endstop_x_hit=false;
    endstop_y_hit=false;
    endstop_z_hit=false;
}

void enable_endstops(bool check)
{
    check_endstops = check;
}

//         __________________________
//        /|                        |\     _________________         ^
//       / |                        | \   /|               |\        |
//      /  |                        |  \ / |               | \       s
//     /   |                        |   |  |               |  \      p
//    /    |                        |   |  |               |   \     e
//   +-----+------------------------+---+--+---------------+----+    e
//   |               BLOCK 1            |      BLOCK 2          |    d
//
//                           time ----->
//
//  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates
//  first block->accelerate_until step_events_completed, then keeps going at constant speed until
//  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
//  The slope of acceleration is calculated with the leib ramp alghorithm.

void st_wake_up() {
    ENABLE_STEPPER_DRIVER_INTERRUPT();
}

FORCE_INLINE unsigned short calc_timer(unsigned short step_rate) {
    unsigned short timer;
    if(step_rate > MAX_STEP_FREQUENCY) step_rate = MAX_STEP_FREQUENCY;

    if(step_rate > 20000) { // If steprate > 20kHz >> step 4 times
        step_rate = (step_rate >> 2)&0x3fff;
        step_loops = 4;
    }
    else if(step_rate > 10000) { // If steprate > 10kHz >> step 2 times
        step_rate = (step_rate >> 1)&0x7fff;
        step_loops = 2;
    }
    else {
        step_loops = 1;
    }

    if(step_rate < (16000000/500000)) step_rate = (16000000/500000);

    step_rate -= (16000000/500000); // Correct for minimal speed

    if(step_rate >= (8*256)){ // higher step rate
        unsigned short* table_address = (unsigned short*)&speed_lookuptable_fast[(unsigned char)(step_rate>>8)][0];
        unsigned char tmp_step_rate = (step_rate & 0x00ff);
        unsigned short gain = table_address[1];
        MultiU16X8toH16(timer, tmp_step_rate, gain);
        timer = table_address[0] - timer;
    }
    else { // lower step rates
        unsigned short* table_address = (unsigned short*)&speed_lookuptable_slow[0][0];
        table_address += ((step_rate)>>2) & 0xfffe;
        timer = table_address[0];
        timer -= ((table_address[1] * (unsigned char)(step_rate & 0x0007))>>3);
    }
    if(timer < 100) {
        timer = 100;
        SERIAL_ECHO(MSG_STEPPER_TOO_HIGH);
        SERIAL_ECHOLN(step_rate);
    }//(20kHz this should never happen)
    return timer;
}

/// Initializes the trapezoid generator from the current block. Called whenever a new block begins.
FORCE_INLINE void trapezoid_generator_reset() {

    deceleration_time = 0;

    //step_rate to timer interval, make a note of the number of step loops required at nominal speed
    timer_nominal = calc_timer(current_block->nominal_rate);
    step_loops_nominal = step_loops;

    acc_step_rate = current_block->initial_rate;
    acceleration_time = calc_timer(acc_step_rate);

    StepperTimerAutoReload(acceleration_time);
}

void st_init()
{
    digipot_init(); //Initialize Digipot Motor Current

    microstep_init(); //Initialize Microstepping Pins

//Initialize Dir Pins
#if defined(X_DIR_PIN) && X_DIR_PIN > -1
    SET_OUTPUT(X_DIR_PIN);
#endif
#if defined(X2_DIR_PIN) && X2_DIR_PIN > -1
    SET_OUTPUT(X2_DIR_PIN);
#endif
#if defined(Y_DIR_PIN) && Y_DIR_PIN > -1
    SET_OUTPUT(Y_DIR_PIN);

    #if defined(Y_DUAL_STEPPER_DRIVERS) && defined(Y2_DIR_PIN) && defined(Y2_DIR_PIN) && (Y2_DIR_PIN > -1)
        SET_OUTPUT(Y2_DIR_PIN);
    #endif
#endif
#if defined(Z_DIR_PIN) && Z_DIR_PIN > -1
    SET_OUTPUT(Z_DIR_PIN);

    #if defined(Z_DUAL_STEPPER_DRIVERS) && defined(Z2_DIR_PIN) && (Z2_DIR_PIN > -1)
        SET_OUTPUT(Z2_DIR_PIN);
    #endif
#endif

#if defined(E0_DIR_PIN) && E0_DIR_PIN > -1
    SET_OUTPUT(E0_DIR_PIN);
#endif
#if defined(E1_DIR_PIN) && (E1_DIR_PIN > -1)
    SET_OUTPUT(E1_DIR_PIN);
#endif
#if defined(E2_DIR_PIN) && (E2_DIR_PIN > -1)
    SET_OUTPUT(E2_DIR_PIN);
#endif

  //Initialize Enable Pins - steppers default to disabled.
#if defined(X_ENABLE_PIN) && X_ENABLE_PIN > -1
    SET_OUTPUT(X_ENABLE_PIN);
    if(!X_ENABLE_ON) WRITE(X_ENABLE_PIN,HIGH);
#endif
#if defined(X2_ENABLE_PIN) && X2_ENABLE_PIN > -1
    SET_OUTPUT(X2_ENABLE_PIN);
    if(!X_ENABLE_ON) WRITE(X2_ENABLE_PIN,HIGH);
#endif
#if defined(Y_ENABLE_PIN) && Y_ENABLE_PIN > -1
    SET_OUTPUT(Y_ENABLE_PIN);
    if(!Y_ENABLE_ON) WRITE(Y_ENABLE_PIN,HIGH);

	#if defined(Y_DUAL_STEPPER_DRIVERS) && defined(Y2_ENABLE_PIN) && (Y2_ENABLE_PIN > -1)
	  SET_OUTPUT(Y2_ENABLE_PIN);
	  if(!Y_ENABLE_ON) WRITE(Y2_ENABLE_PIN,HIGH);
	#endif
#endif
#if defined(Z_ENABLE_PIN) && Z_ENABLE_PIN > -1
    SET_OUTPUT(Z_ENABLE_PIN);
    if(!Z_ENABLE_ON) WRITE(Z_ENABLE_PIN,HIGH);

    #if defined(Z_DUAL_STEPPER_DRIVERS) && defined(Z2_ENABLE_PIN) && (Z2_ENABLE_PIN > -1)
      SET_OUTPUT(Z2_ENABLE_PIN);
      if(!Z_ENABLE_ON) WRITE(Z2_ENABLE_PIN,HIGH);
    #endif
#endif
#if defined(E0_ENABLE_PIN) && (E0_ENABLE_PIN > -1)
    SET_OUTPUT(E0_ENABLE_PIN);
    if(!E_ENABLE_ON) WRITE(E0_ENABLE_PIN,HIGH);
#endif
#if defined(E1_ENABLE_PIN) && (E1_ENABLE_PIN > -1)
    SET_OUTPUT(E1_ENABLE_PIN);
    if(!E_ENABLE_ON) WRITE(E1_ENABLE_PIN,HIGH);
#endif
#if defined(E2_ENABLE_PIN) && (E2_ENABLE_PIN > -1)
    SET_OUTPUT(E2_ENABLE_PIN);
    if(!E_ENABLE_ON) WRITE(E2_ENABLE_PIN,HIGH);
#endif

//endstops and pullups
#if defined(X_MIN_PIN) && X_MIN_PIN > -1
    SET_INPUT(X_MIN_PIN);
    #ifdef ENDSTOPPULLUP_XMIN
        WRITE(X_MIN_PIN,HIGH);
    #endif
#endif

#if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
    SET_INPUT(Y_MIN_PIN);
    #ifdef ENDSTOPPULLUP_YMIN
        WRITE(Y_MIN_PIN,HIGH);
    #endif
#endif

#if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
    SET_INPUT(Z_MIN_PIN);
    #ifdef ENDSTOPPULLUP_ZMIN
        WRITE(Z_MIN_PIN,HIGH);
    #endif
#endif

#if defined(X_MAX_PIN) && X_MAX_PIN > -1
    SET_INPUT(X_MAX_PIN);
    #ifdef ENDSTOPPULLUP_XMAX
        WRITE(X_MAX_PIN,HIGH);
    #endif
#endif

#if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
    SET_INPUT(Y_MAX_PIN);
    #ifdef ENDSTOPPULLUP_YMAX
        WRITE(Y_MAX_PIN,HIGH);
    #endif
#endif

#if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
    SET_INPUT(Z_MAX_PIN);
    #ifdef ENDSTOPPULLUP_ZMAX
        WRITE(Z_MAX_PIN,HIGH);
    #endif
#endif


//Initialize Step Pins
#if defined(X_STEP_PIN) && (X_STEP_PIN > -1)
    SET_OUTPUT(X_STEP_PIN);
    WRITE(X_STEP_PIN,INVERT_X_STEP_PIN);
    disable_x();
#endif
#if defined(X2_STEP_PIN) && (X2_STEP_PIN > -1)
    SET_OUTPUT(X2_STEP_PIN);
    WRITE(X2_STEP_PIN,INVERT_X_STEP_PIN);
    disable_x();
#endif
#if defined(Y_STEP_PIN) && (Y_STEP_PIN > -1)
    SET_OUTPUT(Y_STEP_PIN);
    WRITE(Y_STEP_PIN,INVERT_Y_STEP_PIN);
    #if defined(Y_DUAL_STEPPER_DRIVERS) && defined(Y2_STEP_PIN) && (Y2_STEP_PIN > -1)
        SET_OUTPUT(Y2_STEP_PIN);
        WRITE(Y2_STEP_PIN,INVERT_Y_STEP_PIN);
    #endif
    disable_y();
#endif
#if defined(Z_STEP_PIN) && (Z_STEP_PIN > -1)
    SET_OUTPUT(Z_STEP_PIN);
    WRITE(Z_STEP_PIN,INVERT_Z_STEP_PIN);
    #if defined(Z_DUAL_STEPPER_DRIVERS) && defined(Z2_STEP_PIN) && (Z2_STEP_PIN > -1)
        SET_OUTPUT(Z2_STEP_PIN);
        WRITE(Z2_STEP_PIN,INVERT_Z_STEP_PIN);
    #endif
    disable_z();
#endif
#if defined(E0_STEP_PIN) && (E0_STEP_PIN > -1)
    SET_OUTPUT(E0_STEP_PIN);
    WRITE(E0_STEP_PIN,INVERT_E_STEP_PIN);
    disable_e0();
#endif
#if defined(E1_STEP_PIN) && (E1_STEP_PIN > -1)
    SET_OUTPUT(E1_STEP_PIN);
    WRITE(E1_STEP_PIN,INVERT_E_STEP_PIN);
    disable_e1();
#endif
#if defined(E2_STEP_PIN) && (E2_STEP_PIN > -1)
    SET_OUTPUT(E2_STEP_PIN);
    WRITE(E2_STEP_PIN,INVERT_E_STEP_PIN);
    disable_e2();
#endif

    StepperTimerInit();
    ENABLE_STEPPER_DRIVER_INTERRUPT();

    enable_endstops(true); // Start with endstops active. After homing they can be disabled
}

// Block until all buffered steps are executed
void st_synchronize()
{
    while(blocks_queued()) {
        Jawsboard::getBoard().runboardSlice();
        command::manage_inactivity();
    }
}

void st_set_position(const long &x, const long &y, const long &z, const long &e)
{
    CRITICAL_SECTION_START;
    count_position[X_AXIS] = x;
    count_position[Y_AXIS] = y;
    count_position[Z_AXIS] = z;
    count_position[E_AXIS] = e;
    CRITICAL_SECTION_END;
}

void st_set_e_position(const long &e)
{
    CRITICAL_SECTION_START;
    count_position[E_AXIS] = e;
    CRITICAL_SECTION_END;
}

long st_get_position(uint8_t axis)
{
    long count_pos;
    CRITICAL_SECTION_START;
    count_pos = count_position[axis];
    CRITICAL_SECTION_END;
    return count_pos;
}

#ifdef ENABLE_AUTO_BED_LEVELING
float st_get_position_mm(uint8_t axis)
{
    float steper_position_in_steps = st_get_position(axis);
    return steper_position_in_steps / axis_steps_per_unit[axis];
}
#endif  // ENABLE_AUTO_BED_LEVELING

void finishAndDisableSteppers()
{
    st_synchronize();
    disable_x();
    disable_y();
    disable_z();
    disable_e0();
    disable_e1();
    disable_e2();
}

void quickStop()
{
    DISABLE_STEPPER_DRIVER_INTERRUPT();
    while(blocks_queued())
        plan_discard_current_block();
    current_block = NULL;
    ENABLE_STEPPER_DRIVER_INTERRUPT();
}

void digitalPotWrite(int address, int value) // From Arduino DigitalPotControl example
{
    digipot_set(address, value);
}

void digipot_init() //Initialize Digipot Motor Current
{
    const uint8_t digipot_motor_current[] = DIGIPOT_MOTOR_CURRENT;

    for(int i=0; i<sizeof(digipot_motor_current); i++)
        digitalPotWrite(i, digipot_motor_current[i]);
}

void microstep_init()
{

}

void microstep_ms(uint8_t driver, int8_t ms1, int8_t ms2)
{

}

void microstep_mode(uint8_t driver, uint8_t stepping_mode)
{

}

void microstep_readings()
{

}

void StepperControlIRQHandler()
{
    // If there is no current block, attempt to pop one from the buffer
    if (current_block == NULL) {
        // Anything in the buffer?
        current_block = plan_get_current_block();
        if (current_block != NULL) {
            current_block->busy = true;
            trapezoid_generator_reset();
            counter_x = -(current_block->step_event_count >> 1);
            counter_y = counter_x;
            counter_z = counter_x;
            counter_e = counter_x;
            step_events_completed = 0;
        }
        else {
            StepperTimerAutoReload(2000); // 1kHz.
        }
    }

    if (current_block != NULL) {
        // Set directions TO DO This should be done once during init of trapezoid. Endstops -> interrupt
        out_bits = current_block->direction_bits;

        // Set the direction bits (X_AXIS=A_AXIS and Y_AXIS=B_AXIS for COREXY)
        if((out_bits & (1<<X_AXIS))!=0){
        #ifdef DUAL_X_CARRIAGE
            if (extruder_duplication_enabled){
                WRITE(X_DIR_PIN, INVERT_X_DIR);
                WRITE(X2_DIR_PIN, INVERT_X_DIR);
            }
            else{
                if (current_block->active_extruder != 0)
                    WRITE(X2_DIR_PIN, INVERT_X_DIR);
                else
                    WRITE(X_DIR_PIN, INVERT_X_DIR);
            }
        #else
            WRITE(X_DIR_PIN, INVERT_X_DIR);
        #endif
            count_direction[X_AXIS]=-1;
        }
        else{
        #ifdef DUAL_X_CARRIAGE
            if (extruder_duplication_enabled){
                WRITE(X_DIR_PIN, !INVERT_X_DIR);
                WRITE(X2_DIR_PIN, !INVERT_X_DIR);
            }
            else{
                if (current_block->active_extruder != 0)
                    WRITE(X2_DIR_PIN, !INVERT_X_DIR);
                else
                    WRITE(X_DIR_PIN, !INVERT_X_DIR);
            }
        #else
            WRITE(X_DIR_PIN, !INVERT_X_DIR);
        #endif
            count_direction[X_AXIS]=1;
        }
        if((out_bits & (1<<Y_AXIS))!=0){
            WRITE(Y_DIR_PIN, INVERT_Y_DIR);

            #ifdef Y_DUAL_STEPPER_DRIVERS
            WRITE(Y2_DIR_PIN, !(INVERT_Y_DIR == INVERT_Y2_VS_Y_DIR));
            #endif

            count_direction[Y_AXIS]=-1;
        }
        else{
            WRITE(Y_DIR_PIN, !INVERT_Y_DIR);

            #ifdef Y_DUAL_STEPPER_DRIVERS
            WRITE(Y2_DIR_PIN, (INVERT_Y_DIR == INVERT_Y2_VS_Y_DIR));
            #endif

            count_direction[Y_AXIS]=1;
        }

        // Set direction en check limit switches
        #ifndef COREXY
        if ((out_bits & (1<<X_AXIS)) != 0) {   // stepping along -X axis
        #else
        if (((out_bits & (1<<X_AXIS)) != 0)&&(out_bits & (1<<Y_AXIS)) != 0) {   //-X occurs for -A and -B
        #endif
            CHECK_ENDSTOPS
            {
                #ifdef DUAL_X_CARRIAGE
                // with 2 x-carriages, endstops are only checked in the homing direction for the active extruder
                if ((current_block->active_extruder == 0 && X_HOME_DIR == -1) || (current_block->active_extruder != 0 && X2_HOME_DIR == -1))
                #endif
                {
                #if defined(X_MIN_PIN) && X_MIN_PIN > -1
                    bool x_min_endstop=(READ(X_MIN_PIN) != X_MIN_ENDSTOP_INVERTING);
                    if(x_min_endstop && old_x_min_endstop && (current_block->steps_x > 0)) {
                        endstops_trigsteps[X_AXIS] = count_position[X_AXIS];
                        endstop_x_hit=true;
                        step_events_completed = current_block->step_event_count;
                    }
                    old_x_min_endstop = x_min_endstop;
                #endif
                }
            }
        }
        else { // +direction
            CHECK_ENDSTOPS
            {
                #ifdef DUAL_X_CARRIAGE
                // with 2 x-carriages, endstops are only checked in the homing direction for the active extruder
                if ((current_block->active_extruder == 0 && X_HOME_DIR == 1) || (current_block->active_extruder != 0 && X2_HOME_DIR == 1))
                #endif
                {
                #if defined(X_MAX_PIN) && X_MAX_PIN > -1
                    bool x_max_endstop=(READ(X_MAX_PIN) != X_MAX_ENDSTOP_INVERTING);
                    if(x_max_endstop && old_x_max_endstop && (current_block->steps_x > 0)){
                        endstops_trigsteps[X_AXIS] = count_position[X_AXIS];
                        endstop_x_hit=true;
                        step_events_completed = current_block->step_event_count;
                    }
                    old_x_max_endstop = x_max_endstop;
                #endif
                }
            }
        }

        #ifndef COREXY
        if ((out_bits & (1<<Y_AXIS)) != 0) {   // -direction
        #else
        if (((out_bits & (1<<X_AXIS)) != 0)&&(out_bits & (1<<Y_AXIS)) == 0) {   // -Y occurs for -A and +B
        #endif
            CHECK_ENDSTOPS
            {
            #if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
                bool y_min_endstop=(READ(Y_MIN_PIN) != Y_MIN_ENDSTOP_INVERTING);
                if(y_min_endstop && old_y_min_endstop && (current_block->steps_y > 0)) {
                    endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
                    endstop_y_hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                old_y_min_endstop = y_min_endstop;
            #endif
            }
        }
        else { // +direction
            CHECK_ENDSTOPS
            {
                #if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
                bool y_max_endstop=(READ(Y_MAX_PIN) != Y_MAX_ENDSTOP_INVERTING);
                if(y_max_endstop && old_y_max_endstop && (current_block->steps_y > 0)){
                    endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
                    endstop_y_hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                old_y_max_endstop = y_max_endstop;
                #endif
            }
        }

        if ((out_bits & (1<<Z_AXIS)) != 0) {   // -direction
            WRITE(Z_DIR_PIN,INVERT_Z_DIR);

            #ifdef Z_DUAL_STEPPER_DRIVERS
            WRITE(Z2_DIR_PIN,INVERT_Z_DIR);
            #endif

            count_direction[Z_AXIS]=-1;
            CHECK_ENDSTOPS
            {
                #if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
                bool z_min_endstop=(READ(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING);
                if(z_min_endstop && old_z_min_endstop && (current_block->steps_z > 0)) {
                    endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
                    endstop_z_hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                old_z_min_endstop = z_min_endstop;
                #endif
            }
        }
        else { // +direction
            WRITE(Z_DIR_PIN,!INVERT_Z_DIR);

            #ifdef Z_DUAL_STEPPER_DRIVERS
            WRITE(Z2_DIR_PIN,!INVERT_Z_DIR);
            #endif

            count_direction[Z_AXIS]=1;
            CHECK_ENDSTOPS
            {
                #if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
                bool z_max_endstop=(READ(Z_MAX_PIN) != Z_MAX_ENDSTOP_INVERTING);
                if(z_max_endstop && old_z_max_endstop && (current_block->steps_z > 0)) {
                    endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
                    endstop_z_hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                old_z_max_endstop = z_max_endstop;
                #endif
            }
        }

        if ((out_bits & (1<<E_AXIS)) != 0) {  // -direction
            REV_E_DIR();
            count_direction[E_AXIS]=-1;
        }
        else { // +direction
            NORM_E_DIR();
            count_direction[E_AXIS]=1;
        }

        for(int8_t i=0; i < step_loops; i++) { // Take multiple steps per interrupt (For high speed moves)


            counter_x += current_block->steps_x;
        #ifdef CONFIG_STEPPERS_TOSHIBA
            /* The toshiba stepper controller require much longer pulses
            * tjerfore we 'stage' decompose the pulses between high, and
            * low instead of doing each in turn. The extra tests add enough
            * lag to allow it work with without needing NOPs */
            if (counter_x > 0) {
                WRITE(X_STEP_PIN, HIGH);
            }

            counter_y += current_block->steps_y;
            if (counter_y > 0) {
                WRITE(Y_STEP_PIN, HIGH);
            }

            counter_z += current_block->steps_z;
            if (counter_z > 0) {
                WRITE(Z_STEP_PIN, HIGH);
            }

            counter_e += current_block->steps_e;
            if (counter_e > 0) {
                WRITE_E_STEP(HIGH);
            }

            if (counter_x > 0) {
                counter_x -= current_block->step_event_count;
                count_position[X_AXIS]+=count_direction[X_AXIS];
                WRITE(X_STEP_PIN, LOW);
            }

            if (counter_y > 0) {
                counter_y -= current_block->step_event_count;
                count_position[Y_AXIS]+=count_direction[Y_AXIS];
                WRITE(Y_STEP_PIN, LOW);
            }

            if (counter_z > 0) {
                counter_z -= current_block->step_event_count;
                count_position[Z_AXIS]+=count_direction[Z_AXIS];
                WRITE(Z_STEP_PIN, LOW);
            }

            if (counter_e > 0) {
                counter_e -= current_block->step_event_count;
                count_position[E_AXIS]+=count_direction[E_AXIS];
                WRITE_E_STEP(LOW);
            }

        #else
            if (counter_x > 0) {
                #ifdef DUAL_X_CARRIAGE
                if (extruder_duplication_enabled){
                    WRITE(X_STEP_PIN, !INVERT_X_STEP_PIN);
                    WRITE(X2_STEP_PIN, !INVERT_X_STEP_PIN);
                }
                else {
                    if (current_block->active_extruder != 0)
                        WRITE(X2_STEP_PIN, !INVERT_X_STEP_PIN);
                    else
                        WRITE(X_STEP_PIN, !INVERT_X_STEP_PIN);
                }
                #else
                WRITE(X_STEP_PIN, !INVERT_X_STEP_PIN);
                #endif
                counter_x -= current_block->step_event_count;
                count_position[X_AXIS]+=count_direction[X_AXIS];
                #ifdef DUAL_X_CARRIAGE
                if (extruder_duplication_enabled){
                    WRITE(X_STEP_PIN, INVERT_X_STEP_PIN);
                    WRITE(X2_STEP_PIN, INVERT_X_STEP_PIN);
                }
                else {
                    if (current_block->active_extruder != 0)
                        WRITE(X2_STEP_PIN, INVERT_X_STEP_PIN);
                    else
                        WRITE(X_STEP_PIN, INVERT_X_STEP_PIN);
                }
                #else
                WRITE(X_STEP_PIN, INVERT_X_STEP_PIN);
                #endif
            }

            counter_y += current_block->steps_y;
            if (counter_y > 0) {
                WRITE(Y_STEP_PIN, !INVERT_Y_STEP_PIN);

                #ifdef Y_DUAL_STEPPER_DRIVERS
                WRITE(Y2_STEP_PIN, !INVERT_Y_STEP_PIN);
                #endif

                counter_y -= current_block->step_event_count;
                count_position[Y_AXIS]+=count_direction[Y_AXIS];
                WRITE(Y_STEP_PIN, INVERT_Y_STEP_PIN);

                #ifdef Y_DUAL_STEPPER_DRIVERS
                WRITE(Y2_STEP_PIN, INVERT_Y_STEP_PIN);
                #endif
            }

            counter_z += current_block->steps_z;
            if (counter_z > 0) {
                WRITE(Z_STEP_PIN, !INVERT_Z_STEP_PIN);

                #ifdef Z_DUAL_STEPPER_DRIVERS
                WRITE(Z2_STEP_PIN, !INVERT_Z_STEP_PIN);
                #endif

                counter_z -= current_block->step_event_count;
                count_position[Z_AXIS]+=count_direction[Z_AXIS];
                WRITE(Z_STEP_PIN, INVERT_Z_STEP_PIN);

                #ifdef Z_DUAL_STEPPER_DRIVERS
                WRITE(Z2_STEP_PIN, INVERT_Z_STEP_PIN);
                #endif
            }

            counter_e += current_block->steps_e;
            if (counter_e > 0) {
                WRITE_E_STEP(!INVERT_E_STEP_PIN);
                counter_e -= current_block->step_event_count;
                count_position[E_AXIS]+=count_direction[E_AXIS];
                WRITE_E_STEP(INVERT_E_STEP_PIN);
            }

        #endif
            step_events_completed += 1;
            if(step_events_completed >= current_block->step_event_count) break;
        }
        // Calculare new timer value
        unsigned short timer;
        unsigned short step_rate;
        if (step_events_completed <= (unsigned long int)current_block->accelerate_until) {

            MultiU24X24toH16(acc_step_rate, acceleration_time, current_block->acceleration_rate);
            acc_step_rate += current_block->initial_rate;

            // upper limit
            if(acc_step_rate > current_block->nominal_rate)
                acc_step_rate = current_block->nominal_rate;

                // step_rate to timer interval
                timer = calc_timer(acc_step_rate);
                StepperTimerAutoReload(timer);
                acceleration_time += timer;
        }
        else if (step_events_completed > (unsigned long int)current_block->decelerate_after) {
            MultiU24X24toH16(step_rate, deceleration_time, current_block->acceleration_rate);

            if(step_rate > acc_step_rate) { // Check step_rate stays positive
                step_rate = current_block->final_rate;
            }
            else {
                step_rate = acc_step_rate - step_rate; // Decelerate from aceleration end point.
            }

            // lower limit
            if(step_rate < current_block->final_rate)
                step_rate = current_block->final_rate;

            // step_rate to timer interval
            timer = calc_timer(step_rate);
            StepperTimerAutoReload(timer);
            deceleration_time += timer;
        }
        else {
            ///ensure we're running at the correct step rate, even if we just came off an acceleration
            StepperTimerAutoReload(timer_nominal);
            step_loops = step_loops_nominal;
        }

        /// If current block is finished, reset pointer
        if (step_events_completed >= current_block->step_event_count) {
            current_block = NULL;
            plan_discard_current_block();
        }
    }
}

void StepperTimerInit()
{
    ///2M Hz
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_STEPPER_HandleStruct.Instance = TIM2;
    TIM_STEPPER_HandleStruct.Init.Period = 0x4000;
    TIM_STEPPER_HandleStruct.Init.Prescaler = (uint32_t) (SystemCoreClock/2/2000000) - 1;
    TIM_STEPPER_HandleStruct.Init.ClockDivision = 0;
    TIM_STEPPER_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TIM_STEPPER_HandleStruct);
    __HAL_TIM_SetCounter(&TIM_STEPPER_HandleStruct,0);

    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{
    /* TIM Update event */
    if(__HAL_TIM_GET_FLAG(&TIM_STEPPER_HandleStruct, TIM_FLAG_UPDATE) != RESET){
        if(__HAL_TIM_GET_IT_SOURCE(&TIM_STEPPER_HandleStruct, TIM_IT_UPDATE) !=RESET){
            __HAL_TIM_CLEAR_IT(&TIM_STEPPER_HandleStruct, TIM_IT_UPDATE);
            StepperControlIRQHandler();
        }
    }
}






