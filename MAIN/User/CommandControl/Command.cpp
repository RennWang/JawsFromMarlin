/**
  ******************************************************************************
  * @file    Command.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    8-May-2015
  * @brief
  *
  ******************************************************************************
  */
#include "Command.h"
#include "planner.h"
#include "stepper.h"
#include "motion_arc.h"
#include "JawsBoard.h"
#include "EepromMap.h"
#include "FilamentDetect.h"
#include "PiezoAudio.h"
#include "SDCard.h"

#define MAX_CMD_SIZE 96
#define BUFSIZE 8

#define ABL_PROBE_PT_1_X 15
#define ABL_PROBE_PT_1_Y 180
#define ABL_PROBE_PT_2_X 15
#define ABL_PROBE_PT_2_Y 20
#define ABL_PROBE_PT_3_X 170
#define ABL_PROBE_PT_3_Y 20

#define Z_RAISE_BEFORE_PROBING      15    //How much the extruder will be raised before traveling to the first probing point.
#define Z_RAISE_BETWEEN_PROBINGS    5  //How much the extruder will be raised when traveling from between next probing points
#define XY_TRAVEL_SPEED             4000

#define HOMING_FEEDRATE             {100*60, 100*60, 20*60, 0}// set the homing speeds (mm/min)
#define AXIS_RELATIVE_MODES         {false, false, false, false}

#define DEFAULT_STEPPER_DEACTIVE_TIME 60


namespace command{

float homing_feedrate[] = HOMING_FEEDRATE;
bool axis_relative_modes[] = AXIS_RELATIVE_MODES;

int feedmultiply = 100; //100->1 200->2
int extrudemultiply = 100; //100->1 200->2

float current_position[NUM_AXIS] = { 0.0, 0.0, 0.0, 0.0 };
float last_position[NUM_AXIS] = { 0.0, 0.0, 0.0, 0.0 };
float add_homing[3] = { 0, 0, 0 };
float last_e_position = 0.0;

float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };

bool axis_known_position[3] = { false, false, false };
float zprobe_zoffset;

// Extruder offset, only in XY plane
#if EXTRUDER_COUNT > 1
float extruder_offset[2][EXTRUDER_COUNT] = {
#if defined(EXTRUDER_OFFSET_X) && defined(EXTRUDER_OFFSET_Y)
    EXTRUDER_OFFSET_X, EXTRUDER_OFFSET_Y
#endif
};
#endif

bool autoretract_enabled = false;
bool retracted = false;
float retract_length = 4.5, retract_feedrate = 25*60, retract_zlift = 0.8;
float retract_recover_length = 0, retract_recover_feedrate = 25*60;

uint8_t active_extruder = 0;
uint8_t fanSpeed = 0;
uint8_t fanSpeedPercent = 100;

///===========================================================================
///=============================semi-private variables========================
///===========================================================================

bool Paused= false;
bool MovedinPause = false;
bool Stopped = false;
bool cancel_heatup = false ;

///===========================================================================
///=============================private variables=============================
///===========================================================================

static int saved_feedmultiply;

static uint8_t tmp_extruder;

static bool CooldownNoWait = true;

static bool target_direction;

static const char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
static float destination[NUM_AXIS] = {  0.0, 0.0, 0.0, 0.0 };

static float offset[3] = { 0.0, 0.0, 0.0 };
static bool home_all_axis = true;
static float feedrate = 1500.0, next_feedrate, saved_feedrate;

static bool relative_mode = false;  //Determines Absolute or Relative Coordinates

//Inactivity shutdown variables
static unsigned long previous_millis_cmd = 0;
static unsigned long max_inactive_time = 0;
static unsigned long stepper_inactive_time = DEFAULT_STEPPER_DEACTIVE_TIME * 1000L;

static unsigned long starttime = 0;
static unsigned long stoptime = 0;

///===========================================================================
///=============================private variables=============================
///===========================================================================

static char cmdbuffer[BUFSIZE][MAX_CMD_SIZE];
static bool fromsd[BUFSIZE];
static int bufindr = 0;             //cmd read index
static int bufindw = 0;             //cmd write index
static int buflen = 0;

static char serial_char;
static int serial_count = 0;
static bool comment_mode = false;
static char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc
static long gcode_N, gcode_LastN, Stopped_gcode_LastN = 0;

static void FlushSerialRequestResend();
static void ClearToSend();

static void pausePrint();
static void resumePrint();
static void moveEinPause();
static void stopEinPause();

static void selectSDFile();
static void deleteSDFile();
static void deleteAllSDFiles();
static void startSDPrint();
static void pauseSDPrint();
static void cancelSDPrint();

static void startSDWrite();
static void stopSDWrite();

///===========================================================================
///=============================private functions=============================
///===========================================================================

static void doMove();
static void doArcMove(bool clockwise);
static void doDwell();
static void doHome();

static void doDetailZProbe();
static void doSingleZProbe();
static void doSetPosition();

static void enableDrives();
static void disableDrives();

static void setTempForExtruder();
static void setTempForHeatingbed();

static void waitTempForExtruder();
static void waitTempForHeatingbed();

static void readCurrentTemperature();
static void readCurrentPosition();
static void takeTimeSinceLast();
static void readEndstopStatus();

static void setSpeedFactor();
static void setExtrudeFactor();

static void setHomingoffset();
static void filamentChange();
static void setMotorCurrent();
static void restartAfterStopped();

static void get_coordinates();
static void prepare_move();

static void get_arc_coordinates();
static void prepare_arc_move(char isclockwise);

static void engage_z_probe();
static void retract_z_probe();
static void run_z_probe();
static void do_blocking_move_to(float x, float y, float z);
static void do_blocking_move_relative(float offset_x, float offset_y, float offset_z);
static void setup_for_endstop_move();
static float probe_pt(float x, float y, float z_before);
static void clean_up_after_endstop_move();
static void set_bed_level_equation_3pts(float z_at_pt_1, float z_at_pt_2, float z_at_pt_3);

static bool setTargetedHotend(int code);

static void homeaxis(int axis);
static void axis_is_at_home(int axis);

#define XYZ_CONSTS_FROM_CONFIG(type, array, CONFIG) \
static const type array##_P[3] =        \
    { X_##CONFIG, Y_##CONFIG, Z_##CONFIG };     \
static inline type array(int axis)          \
    { return (array##_P[axis]); }

XYZ_CONSTS_FROM_CONFIG(float, base_min_pos,    MIN_POS);
XYZ_CONSTS_FROM_CONFIG(float, base_max_pos,    MAX_POS);
XYZ_CONSTS_FROM_CONFIG(float, base_home_pos,   HOME_POS);
XYZ_CONSTS_FROM_CONFIG(float, max_length,      MAX_LENGTH);
XYZ_CONSTS_FROM_CONFIG(float, home_retract_mm, HOME_RETRACT_MM);
XYZ_CONSTS_FROM_CONFIG(signed char, home_dir,  HOME_DIR);

void enquecommand(const char *cmd)
{
    if(buflen < BUFSIZE){
        //this is dangerous if a mixing of serial and this happens
        strcpy(&(cmdbuffer[bufindw][0]),cmd);
        bufindw= (bufindw + 1)%BUFSIZE;
        buflen += 1;
    }
}

float code_value()
{
    return (strtod(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL));
}

long code_value_long()
{
    return (strtol(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL, 10));
}

bool code_seen(char code)
{
    strchr_pointer = strchr(cmdbuffer[bufindr], code);
    return (strchr_pointer != NULL);  //Return True if a character was found
}

void get_command()
{
    while( commandSerial.available() > 0  && buflen < BUFSIZE) {
        serial_char = commandSerial.read();
        if(serial_char == '\n' || serial_char == '\r' || (serial_char == ':' && comment_mode == false) || serial_count >= (MAX_CMD_SIZE - 1) ){
            if(!serial_count) { //if empty line
                comment_mode = false; //for new command
                return;
            }
            cmdbuffer[bufindw][serial_count] = 0; //terminate string
            if(!comment_mode){
                fromsd[bufindw] = false;
                if(strchr(cmdbuffer[bufindw], 'N') != NULL){
                    strchr_pointer = strchr(cmdbuffer[bufindw], 'N');
                    gcode_N = (strtol(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL, 10));
                    if(gcode_N != gcode_LastN + 1 && (strstr(cmdbuffer[bufindw], "M110") == NULL) ) {
                        SERIAL_ERROR_START;
                        SERIAL_ERROR(MSG_ERR_LINE_NO);
                        SERIAL_ERRORLN(gcode_LastN);
                        FlushSerialRequestResend();
                        serial_count = 0;
                        return;
                    }

                    if(strchr(cmdbuffer[bufindw], '*') != NULL){
                        uint8_t checksum = 0;
                        uint8_t count = 0;
                        while(cmdbuffer[bufindw][count] != '*') checksum = checksum^cmdbuffer[bufindw][count++];
                        strchr_pointer = strchr(cmdbuffer[bufindw], '*');

                        if( (int)(strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)) != checksum) {
                            SERIAL_ERROR_START;
                            SERIAL_ERROR(MSG_ERR_CHECKSUM_MISMATCH);
                            SERIAL_ERRORLN(gcode_LastN);
                            FlushSerialRequestResend();
                            serial_count = 0;
                            return;
                        }
                    //if no errors, continue parsing
                    }else{
                        SERIAL_ERROR_START;
                        SERIAL_ERROR(MSG_ERR_NO_CHECKSUM);
                        SERIAL_ERRORLN(gcode_LastN);
                        FlushSerialRequestResend();
                        serial_count = 0;
                        return;
                    }

//                    //Filament probe
//                    if (filamentDetect_check()){
//                        SERIAL_ERROR_START;
//                        SERIAL_ERRORLN(MSG_ERR_FILAMENT);
//                        commandSerial.flush();
//                        ClearToSend();
//                        serial_count = 0;
//                        return;
//                    }

                    gcode_LastN = gcode_N;
                //if no errors, continue parsing
                }else{  // if we don't receive 'N' but still see '*'
                    if((strchr(cmdbuffer[bufindw], '*') != NULL)){
                        SERIAL_ERROR_START;
                        SERIAL_ERROR(MSG_ERR_NO_LINENUMBER_WITH_CHECKSUM);
                        SERIAL_ERRORLN(gcode_LastN);
                        serial_count = 0;
                        return;
                    }
                }

                if((strchr(cmdbuffer[bufindw], 'G') != NULL)){
                    strchr_pointer = strchr(cmdbuffer[bufindw], 'G');
                    switch((int)((strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)))){
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        if (Stopped == true) {
                            SERIAL_ERRORLN(MSG_ERR_STOPPED);
                            break;
                        }
                        if (Paused == true){
                            SERIAL_ERRORLN(MSG_ERR_PAUSED);
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }

                //If command was e-stop process now
                if(strcmp(cmdbuffer[bufindw], "M112") == 0)
                    kill();

                bufindw = (bufindw + 1)%BUFSIZE;
                buflen += 1;
            }
            serial_count = 0; //clear buffer
        }else{
            if(serial_char == ';') comment_mode = true;
            if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
        }
    }

    //sd card print get command
    if(!sd_card.isSDPrinted() || serial_count!=0){
        return;
    }

    static bool stop_buffering=false;
    if(buflen==0) stop_buffering=false;

    while( !sd_card.eof()  && buflen < BUFSIZE && !stop_buffering) {
        int n = sd_card.read();
        serial_char = (char)n;
        if(serial_char == '\n' || serial_char == '\r' || (serial_char == '#' && comment_mode == false) ||
            (serial_char == ':' && comment_mode == false) || serial_count >= (MAX_CMD_SIZE - 1)||n==-1)
        {
            if(sd_card.eof()){
                SERIAL_PROTOCOLLN(MSG_FILE_PRINTED);
                stoptime=millis();
                sd_card.printingHasFinished();
                //card.checkautostart(true);
            }
            if(serial_char=='#')
                stop_buffering=true;

            if(!serial_count)
            {
                comment_mode = false; //for new command
                return; //if empty line
            }
            cmdbuffer[bufindw][serial_count] = 0; //terminate string

            fromsd[bufindw] = true;
            buflen += 1;
            bufindw = (bufindw + 1)%BUFSIZE;

            comment_mode = false; //for new command
            serial_count = 0; //clear buffer
        }
        else
        {
            if(serial_char == ';') comment_mode = true;
            if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
        }
    }
}

void process_commands()
{
    if(code_seen('G')){
        process_Gcommands((int)code_value());
    }else if(code_seen('M')){
        process_Mcommands((int)code_value());
    }else if(code_seen('T')){
        process_Tcommands((int)code_value());
    }else{
        SERIAL_ECHO_START;
        SERIAL_ECHO(MSG_UNKNOWN_COMMAND);
        SERIAL_ECHO(cmdbuffer[bufindr]);
        SERIAL_ECHOLN("\"");
        ClearToSend();
    }
}

void runCommandSlice()
{
    if(buflen < (BUFSIZE-1))
        get_command();

    if(buflen){
        process_commands();
        buflen = (buflen - 1);
        bufindr = (bufindr + 1)%BUFSIZE;
    }

    manage_inactivity();
    checkHitEndstops();
}

static void FlushSerialRequestResend()
{
    commandSerial.flush();
    SERIAL_PROTOCOL(MSG_RESEND);
    SERIAL_PROTOCOLLN(gcode_LastN + 1);
    ClearToSend();
}

static void ClearToSend()
{
    previous_millis_cmd = millis();
    if(fromsd[bufindr])
        return;

    SERIAL_PROTOCOLLN(MSG_OK);
}

void process_Gcommands(int codevalue)
{
    switch(codevalue){
    case 0: // G0 -> G1
    case 1: // G1
        if(Stopped == false && Paused == false) {
            doMove();
        }
        break;
    case 2: // G2  - CW ARC
        if(Stopped == false  && Paused == false) {
            doArcMove(true);
        }
        break;
    case 3: // G3  - CCW ARC
        if(Stopped == false  && Paused == false) {
            doArcMove(false);
        }
        break;
    case 4:
        doDwell();
        break;
    case 28:
        doHome();
        break;
    #ifdef ENABLE_AUTO_BED_LEVELING
    case 29:
        doDetailZProbe();
        break;
    case 30:
        doSingleZProbe();
        break;
    #endif
    case 90: // G90
        relative_mode = false;
        break;
    case 91: // G91
        relative_mode = true;
        break;
    case 92: // G92
        doSetPosition();
        break;
    }
    ClearToSend();
}

void process_Mcommands(int codevalue)
{
    switch(codevalue){
    case 17:
        enableDrives();
        break;
    case 18:
    case 84:
        disableDrives();
        break;

    case 23:
        selectSDFile();
        break;
    case 24:
        startSDPrint();
        break;
    case 25:
        pauseSDPrint();
        break;
    case 26:
        cancelSDPrint();
        break;
    case 28:
        startSDWrite();
        break;
    case 29:
        stopSDWrite();
        break;
    case 30:
        deleteSDFile();
        break;

    case 31:
        takeTimeSinceLast();
        break;
    case 82:
        axis_relative_modes[3] = false;
        break;
    case 83:
        axis_relative_modes[3] = true;
        break;
    case 106:
        if (code_seen('S')){
            fanSpeed = constrain(code_value() * fanSpeedPercent / 100, 0, 255);
        }else {
            fanSpeed = 255;
        }
        break;
    case 107:
        fanSpeed = 0;
        break;
    case 112:
        kill();
        break;
    case 104:
        setTempForExtruder();
        break;
    case 109:
        waitTempForExtruder();
        break;
    case 140:
        setTempForHeatingbed();
        break;
    case 190:
        waitTempForHeatingbed();
        break;
    case 105:
        readCurrentTemperature();
        return;
    case 114:
        readCurrentPosition();
        break;
    case 115:
        SERIAL_PROTOCOL(MSG_M115_REPORT);
        break;
    case 119:
        readEndstopStatus();
        break;
    case 120: // M120
        enable_endstops(false) ;
        break;
    case 121: // M121
        enable_endstops(true) ;
        break;
    case 220:
        setSpeedFactor();
        break;
    case 221:
        setExtrudeFactor();
        break;
    case 206:
        setHomingoffset();
        break;
    case 400:
        st_synchronize();
        break;
    case 401:
        quickStop();
        break;
    case 324:
        pausePrint();
        break;
    case 325:
        resumePrint();
        break;
    case 326:
        moveEinPause();
        break;
    case 327:
        stopEinPause();
        break;
    case 600:
        filamentChange();
        break;
    case 907:
        setMotorCurrent();
        break;
    case 999:
        restartAfterStopped();
        break;
    }
    ClearToSend();
}

void process_Tcommands(int codevalue)
{
    tmp_extruder = code_value();
    if(tmp_extruder >= Jawsboard::getBoard().tool_count) {
        SERIAL_ECHO_START;
        SERIAL_ECHO("T");
        SERIAL_ECHO(tmp_extruder);
        SERIAL_ECHOLN(MSG_INVALID_EXTRUDER);
    }else {
        bool make_move = false;
        if(code_seen('F')) {
            make_move = true;
            next_feedrate = code_value();
            if(next_feedrate > 0.0) {
                feedrate = next_feedrate;
            }
        }

#if EXTRUDER_COUNT > 1
        if(tmp_extruder != active_extruder) {
            // Save current position to return to after applying extruder offset
            memcpy(destination, current_position, sizeof(destination));

            // Offset extruder (only by XY)
            int i;
            for(i = 0; i < 2; i++) {
                current_position[i] = current_position[i] - extruder_offset[i][active_extruder] + extruder_offset[i][tmp_extruder];
            }
            // Set the new active extruder and position
            active_extruder = tmp_extruder;

            plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
            // Move to the old position if 'F' was in the parameters
            if(make_move && Stopped == false) {
                prepare_move();
            }
        }
#endif
    }
    ClearToSend();
}

static void pausePrint()
{
    float target[4];

    if (Paused == true)
        return;

    Paused = true;

    st_synchronize();

    piezoaudio.playTune(TUNE_PAUSE);

    last_position[X_AXIS] = current_position[X_AXIS];
    last_position[Y_AXIS] = current_position[Y_AXIS];
    last_position[Z_AXIS] = current_position[Z_AXIS];
    last_position[E_AXIS] = current_position[E_AXIS];

    target[X_AXIS] = current_position[X_AXIS];
    target[Y_AXIS] = current_position[Y_AXIS];
    target[Z_AXIS] = current_position[Z_AXIS];
    target[E_AXIS] = current_position[E_AXIS];

    //retract
    target[E_AXIS] -= retract_length;
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], retract_feedrate/60, active_extruder);

    //zlift
    if (current_position[Z_AXIS] < Z_MAX_POS - 5){
        target[Z_AXIS] = Z_MAX_POS - 5;
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], homing_feedrate[Z_AXIS]/60, active_extruder);
    }

    //move XY
    target[X_AXIS] = X_MAX_POS - 5;
    target[Y_AXIS] = Y_MAX_POS - 5;
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], homing_feedrate[X_AXIS]/60, active_extruder);

    current_position[X_AXIS] = target[X_AXIS];
    current_position[Y_AXIS] = target[Y_AXIS];
    current_position[Z_AXIS] = target[Z_AXIS];
    current_position[E_AXIS] = target[E_AXIS];

    //Jawsboard::getBoard().heatersPause(false);
}

static void resumePrint()
{
    float target[4];

//    Jawsboard::getBoard().heatersResume(false);
//    while (!Jawsboard::getBoard().heatersHasReachedTargetTemperature(false)){
//        Jawsboard::getBoard().runboardSlice();
//        manage_inactivity();
//    }
    if (Paused == false)
        return;

    Paused = false;

    st_synchronize();
    piezoaudio.playTune(TUNE_FILAMENT_START);

    target[X_AXIS] = current_position[X_AXIS];
    target[Y_AXIS] = current_position[Y_AXIS];
    target[Z_AXIS] = current_position[Z_AXIS];
    target[E_AXIS] = current_position[E_AXIS];

    //move XY
    target[X_AXIS] = last_position[X_AXIS];
    target[Y_AXIS] = last_position[Y_AXIS];
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], homing_feedrate[X_AXIS]/60, active_extruder);

    //lift Z
    target[Z_AXIS] = last_position[Z_AXIS];
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], homing_feedrate[Z_AXIS]/60, active_extruder);

    //retract recover
    target[E_AXIS] = last_position[E_AXIS];
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], retract_feedrate/60, active_extruder);

    current_position[X_AXIS] = target[X_AXIS];
    current_position[Y_AXIS] = target[Y_AXIS];
    current_position[Z_AXIS] = target[Z_AXIS];
    current_position[E_AXIS] = target[E_AXIS];
}

static void moveEinPause()
{
    float target,feedrate;

    MovedinPause = true;

    st_synchronize();

    last_e_position = current_position[E_AXIS];

    if(code_seen(axis_codes[E_AXIS])){
        if(code_value_long() != 0) {
            target = current_position[E_AXIS] + code_value();
        }
    }

    if(code_seen('F')) {
        feedrate = code_value();
    }else{
        feedrate = retract_feedrate;
    }

    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], target, feedrate/60, active_extruder);
}

static void stopEinPause()
{
    quickStop();
    if (MovedinPause == false)
        return;

    MovedinPause = false;
    current_position[E_AXIS] = last_e_position;
    plan_set_e_position(last_e_position);
}

static void doMove()
{
    get_coordinates(); // For X Y Z E F
    prepare_move();
}

static void doArcMove(bool clockwise)
{
    get_arc_coordinates();
    prepare_arc_move(clockwise);
}

static void doDwell()
{
    unsigned long codenum = 0;
    if(code_seen('P')) codenum = code_value(); // milliseconds to wait
    if(code_seen('S')) codenum = code_value() * 1000; // seconds to wait

    st_synchronize();
    codenum += millis();  // keep track of when we started waiting
    previous_millis_cmd = millis();
    while(millis() < codenum) {
        Jawsboard::getBoard().runboardSlice();
        manage_inactivity();
    }
}

static void doHome()
{
    #ifdef ENABLE_AUTO_BED_LEVELING
    plan_bed_level_matrix.set_to_identity();    //Reset the plane ("erase" all leveling data)
    #endif //ENABLE_AUTO_BED_LEVELING

    saved_feedrate = feedrate;
    saved_feedmultiply = feedmultiply;
    feedmultiply = 100;
    previous_millis_cmd = millis();

    enable_endstops(true);

    for(int8_t i=0; i < NUM_AXIS; i++) {
        destination[i] = current_position[i];
    }
    feedrate = 0.0;

    home_all_axis = !((code_seen(axis_codes[X_AXIS])) || (code_seen(axis_codes[Y_AXIS])) || (code_seen(axis_codes[Z_AXIS])));

    #if Z_HOME_DIR > 0                      // If homing away from BED do Z first
    if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) {
        homeaxis(Z_AXIS);
    }
    #endif

    #ifdef QUICK_HOME
    if((home_all_axis)||( code_seen(axis_codes[X_AXIS]) && code_seen(axis_codes[Y_AXIS])) )  //first diagonal move
    {
        current_position[X_AXIS] = 0;
        current_position[Y_AXIS] = 0;

        int x_axis_home_dir = home_dir(X_AXIS);

        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
        destination[X_AXIS] = 1.5 * max_length(X_AXIS) * x_axis_home_dir;
        destination[Y_AXIS] = 1.5 * max_length(Y_AXIS) * home_dir(Y_AXIS);
        feedrate = homing_feedrate[X_AXIS];
        if(homing_feedrate[Y_AXIS] < feedrate)
            feedrate = homing_feedrate[Y_AXIS];
        if (max_length(X_AXIS) > max_length(Y_AXIS)) {
            feedrate *= sqrt(pow(max_length(Y_AXIS) / max_length(X_AXIS), 2) + 1);
        } else {
            feedrate *= sqrt(pow(max_length(X_AXIS) / max_length(Y_AXIS), 2) + 1);
        }
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
        st_synchronize();

        axis_is_at_home(X_AXIS);
        axis_is_at_home(Y_AXIS);
        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
        destination[X_AXIS] = current_position[X_AXIS];
        destination[Y_AXIS] = current_position[Y_AXIS];
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
        feedrate = 0.0;
        st_synchronize();
        endstops_hit_on_purpose();

        current_position[X_AXIS] = destination[X_AXIS];
        current_position[Y_AXIS] = destination[Y_AXIS];
        current_position[Z_AXIS] = destination[Z_AXIS];
    }
    #endif

    if((home_all_axis) || (code_seen(axis_codes[X_AXIS]))){
        homeaxis(X_AXIS);
    }

    if((home_all_axis) || (code_seen(axis_codes[Y_AXIS]))){
        homeaxis(Y_AXIS);
    }

    if(code_seen(axis_codes[X_AXIS])){
        if(code_value_long() != 0) {
            current_position[X_AXIS] = code_value() + add_homing[X_AXIS];
        }
    }

    if(code_seen(axis_codes[Y_AXIS])) {
        if(code_value_long() != 0) {
            current_position[Y_AXIS] = code_value() + add_homing[Y_AXIS];
        }
    }

#if Z_HOME_DIR < 0                      // If homing towards BED do Z last
    if(home_all_axis) {
        destination[X_AXIS] = round(Z_SAFE_HOMING_X_POINT - X_PROBE_OFFSET_FROM_EXTRUDER);
        destination[Y_AXIS] = round(Z_SAFE_HOMING_Y_POINT - Y_PROBE_OFFSET_FROM_EXTRUDER);
        destination[Z_AXIS] = Z_RAISE_BEFORE_HOMING * home_dir(Z_AXIS) * (-1);    // Set destination away from bed
        feedrate = XY_TRAVEL_SPEED/60;
        current_position[Z_AXIS] = 0;

        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate, active_extruder);
        st_synchronize();
        current_position[X_AXIS] = destination[X_AXIS];
        current_position[Y_AXIS] = destination[Y_AXIS];

        homeaxis(Z_AXIS);
    }

    // Let's see if X and Y are homed and probe is inside bed area.
    if(code_seen(axis_codes[Z_AXIS])) {
        if ( (axis_known_position[X_AXIS]) && (axis_known_position[Y_AXIS])
            && (current_position[X_AXIS] + X_PROBE_OFFSET_FROM_EXTRUDER >= X_MIN_POS)
            && (current_position[X_AXIS] + X_PROBE_OFFSET_FROM_EXTRUDER <= X_MAX_POS)
            && (current_position[Y_AXIS] + Y_PROBE_OFFSET_FROM_EXTRUDER >= Y_MIN_POS)
            && (current_position[Y_AXIS] + Y_PROBE_OFFSET_FROM_EXTRUDER <= Y_MAX_POS)) {

              current_position[Z_AXIS] = 0;
              plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
              destination[Z_AXIS] = Z_RAISE_BEFORE_HOMING * home_dir(Z_AXIS) * (-1);    // Set destination away from bed
              feedrate = max_feedrate[Z_AXIS];
              plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate, active_extruder);
              st_synchronize();

              homeaxis(Z_AXIS);
        } else if (!((axis_known_position[X_AXIS]) && (axis_known_position[Y_AXIS]))) {
            MULTI_ECHO_START;
            MULTI_PROTOCOLLN(MSG_POSITION_UNKNOWN);
        } else {
            MULTI_ECHO_START;
            MULTI_PROTOCOLLN(MSG_ZPROBE_OUT);
        }
    }
#endif

    if(code_seen(axis_codes[Z_AXIS])) {
        if(code_value_long() != 0) {
            current_position[Z_AXIS] = code_value() + add_homing[Z_AXIS];
        }
    }

#if Z_HOME_DIR < 0
    #ifdef ENABLE_AUTO_BED_LEVELING
    if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) {
          current_position[Z_AXIS] += zprobe_zoffset;  //Add Z_Probe offset (the distance is negative)
    }
    #endif
#endif

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

    #ifdef ENDSTOPS_ONLY_FOR_HOMING
    enable_endstops(false);
    #endif

    feedrate = saved_feedrate;
    feedmultiply = saved_feedmultiply;
    previous_millis_cmd = millis();
    endstops_hit_on_purpose();
}

static void doDetailZProbe()
{
#if Z_MIN_PIN == -1
    #error "You must have a Z_MIN endstop in order to enable Auto Bed Leveling feature!!! Z_MIN_PIN must point to a valid hardware pin."
#endif
    float real_z, x_tmp, y_tmp, z_tmp;
    // Prevent user from running a G29 without first homing in X and Y
    if (! (axis_known_position[X_AXIS] && axis_known_position[Y_AXIS]) ){
        SERIAL_ECHO_START;
        SERIAL_ECHOLN(MSG_POSITION_UNKNOWN);
        return; // abort G29, since we don't know where we are
    }

    st_synchronize();
    // make sure the bed_level_rotation_matrix is identity or the planner will get it incorectly. vector_3 corrected_position = plan_get_position_mm();
    //corrected_position.debug("position before G29");
    plan_bed_level_matrix.set_to_identity();
    vector_3 uncorrected_position = plan_get_position();
    //uncorrected_position.debug("position durring G29");
    current_position[X_AXIS] = uncorrected_position.x;
    current_position[Y_AXIS] = uncorrected_position.y;
    current_position[Z_AXIS] = uncorrected_position.z;
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
    setup_for_endstop_move();

    feedrate = homing_feedrate[Z_AXIS];

    // Probe at 3 arbitrary points
    // probe 1
    float z_at_pt_1 = probe_pt(ABL_PROBE_PT_1_X, ABL_PROBE_PT_1_Y, Z_RAISE_BEFORE_PROBING);

    // probe 2
    float z_at_pt_2 = probe_pt(ABL_PROBE_PT_2_X, ABL_PROBE_PT_2_Y, current_position[Z_AXIS] + Z_RAISE_BETWEEN_PROBINGS);

    // probe 3
    float z_at_pt_3 = probe_pt(ABL_PROBE_PT_3_X, ABL_PROBE_PT_3_Y, current_position[Z_AXIS] + Z_RAISE_BETWEEN_PROBINGS);

    clean_up_after_endstop_move();

    set_bed_level_equation_3pts(z_at_pt_1, z_at_pt_2, z_at_pt_3);

    st_synchronize();

    // The following code correct the Z height difference from z-probe position and hotend tip position.
    // The Z height on homing is measured by Z-Probe, but the probe is quite far from the hotend.
    // When the bed is uneven, this height must be corrected.
    real_z = float(st_get_position(Z_AXIS))/axis_steps_per_unit[Z_AXIS];  //get the real Z (since the auto bed leveling is already correcting the plane)
    x_tmp = current_position[X_AXIS] + X_PROBE_OFFSET_FROM_EXTRUDER;
    y_tmp = current_position[Y_AXIS] + Y_PROBE_OFFSET_FROM_EXTRUDER;
    z_tmp = current_position[Z_AXIS];

    apply_rotation_xyz(plan_bed_level_matrix, x_tmp, y_tmp, z_tmp);         //Apply the correction sending the probe offset
    current_position[Z_AXIS] = z_tmp - real_z + current_position[Z_AXIS];   //The difference is added to current position and sent to planner.
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

static void doSingleZProbe()
{
    //engage probe
    engage_z_probe();

    st_synchronize();
    //make sure the bed_level_rotation_matrix is identity or the planner will get set incorectly
    setup_for_endstop_move();

    feedrate = homing_feedrate[Z_AXIS];

    run_z_probe();
    SERIAL_PROTOCOL(MSG_BED);
    SERIAL_PROTOCOL(" X: ");
    SERIAL_PROTOCOL(current_position[X_AXIS]);
    SERIAL_PROTOCOL(" Y: ");
    SERIAL_PROTOCOL(current_position[Y_AXIS]);
    SERIAL_PROTOCOL(" Z: ");
    SERIAL_PROTOCOL(current_position[Z_AXIS]);
    SERIAL_PROTOCOL("\n");

    clean_up_after_endstop_move();

    //retract probe
    retract_z_probe();
}

static void doSetPosition()
{
    if(!code_seen(axis_codes[E_AXIS]))
        st_synchronize();
    for(int8_t i=0; i < NUM_AXIS; i++) {
        if(code_seen(axis_codes[i])) {
            if(i == E_AXIS) {
                current_position[i] = code_value();
                plan_set_e_position(current_position[E_AXIS]);
            }else {
                current_position[i] = code_value()+add_homing[i];
                plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
            }
        }
    }
}

static void enableDrives()
{
    enable_x();
    enable_y();
    enable_z();
    enable_e0();
    enable_e1();
    enable_e2();
    piezoaudio.playTune(TUNE_STARTUP);
}

static void disableDrives()
{
    if(code_seen('S')){
        stepper_inactive_time = code_value() * 1000;
    }else{
        bool all_axis = !((code_seen(axis_codes[X_AXIS])) || (code_seen(axis_codes[Y_AXIS])) || (code_seen(axis_codes[Z_AXIS]))|| (code_seen(axis_codes[E_AXIS])));
        if(all_axis){
            st_synchronize();
            disable_e0();
            disable_e1();
            disable_e2();
            finishAndDisableSteppers();
            piezoaudio.playTune(TUNE_PRINT_DONE);
        }else{
            st_synchronize();
            if(code_seen('X')) disable_x();
            if(code_seen('Y')) disable_y();
            if(code_seen('Z')) disable_z();
            if(code_seen('E')) {
                disable_e0();
                disable_e1();
                disable_e2();
            }
        }
    }
}

static void setTempForExtruder()
{
    if(setTargetedHotend(104)){
        return;
    }

    if (code_seen('S'))
        Jawsboard::getBoard().setTargetDegExtruder(code_value(), tmp_extruder);
}

static void setTempForHeatingbed()
{
    if (code_seen('S'))
        Jawsboard::getBoard().setTargetDegBed((int16_t)code_value());
}

static void waitTempForExtruder()
{
    if(setTargetedHotend(109)){
        return;
    }

    if (code_seen('S')) {
        Jawsboard::getBoard().setTargetDegExtruder(code_value(),tmp_extruder);
        CooldownNoWait = true;
    } else if (code_seen('R')) {
        Jawsboard::getBoard().setTargetDegExtruder(code_value(),tmp_extruder);
        CooldownNoWait = false;
    }

    uint32_t codenum = millis();

    /* See if we are heating up or cooling down */
    target_direction = Jawsboard::getBoard().isHeatingExtruder(tmp_extruder); // true if heating, false if cooling

    cancel_heatup = false;

    while ( (target_direction)&&(!cancel_heatup) ? (Jawsboard::getBoard().isHeatingExtruder(tmp_extruder)) : (Jawsboard::getBoard().isCoolingExtruder(tmp_extruder) && (CooldownNoWait==false)) ) {
        if( (millis() - codenum) > 1000UL ){ //Print Temp Reading and remaining time every 1 second while heating up/cooling down
            SERIAL_PROTOCOL("T:");
            SERIAL_PROTOCOL_F(Jawsboard::getBoard().getCurrentDegExtruder(tmp_extruder),1);
            SERIAL_PROTOCOL(" E:");
            SERIAL_PROTOCOL((int)tmp_extruder);
            SERIAL_PROTOCOLLN("");
            codenum = millis();
        }
        Jawsboard::getBoard().runboardSlice();
        manage_inactivity();
    }

    starttime = millis();
    previous_millis_cmd = millis();
}

static void waitTempForHeatingbed()
{
    if (!Jawsboard::getBoard().using_platform)
        return;

    Heater& bed_Heater = *(Jawsboard::getBoard().pBed_Heater);
    if (code_seen('S')) {
        bed_Heater.setTargetTemperature((int16_t)code_value());
        CooldownNoWait = true;
    } else if (code_seen('R')) {
        bed_Heater.setTargetTemperature((int16_t)code_value());
        CooldownNoWait = false;
    }

    uint32_t codenum = millis();

    cancel_heatup = false;
    bool target_direction = bed_Heater.isHeating(); // true if heating, false if cooling

    while ((target_direction)&&(!cancel_heatup) ? (bed_Heater.isHeating()) : (bed_Heater.isCooling()&&(CooldownNoWait==false)) ){
        if(( millis() - codenum) > 1000 ){ //Print Temp Reading every 1 second while heating up.
            float tt = Jawsboard::getBoard().getCurrentDegExtruder(active_extruder);
            SERIAL_PROTOCOL("T:");
            SERIAL_PROTOCOL(tt);
            SERIAL_PROTOCOL(" E:");
            SERIAL_PROTOCOL((int)active_extruder);
            SERIAL_PROTOCOL(" B:");
            SERIAL_PROTOCOL_F((float)Jawsboard::getBoard().getCurrentDegBed(),1);
            SERIAL_PROTOCOLLN("");
            codenum = millis();
        }
        Jawsboard::getBoard().runboardSlice();
        manage_inactivity();
    }
    previous_millis_cmd = millis();
}

static void readCurrentTemperature()
{
    if(setTargetedHotend(105)){
        return;
    }

    SERIAL_PROTOCOL("ok T:");
    SERIAL_PROTOCOL_F(Jawsboard::getBoard().getCurrentDegExtruder(tmp_extruder),1);
    SERIAL_PROTOCOL(" /");
    SERIAL_PROTOCOL_F(Jawsboard::getBoard().getTargetDegExtruder(tmp_extruder),1);
    if (Jawsboard::getBoard().using_platform){
        SERIAL_PROTOCOL(" B:");
        SERIAL_PROTOCOL_F(Jawsboard::getBoard().getCurrentDegBed(),1);
        SERIAL_PROTOCOL(" /");
        SERIAL_PROTOCOL_F(Jawsboard::getBoard().getTargetDegBed(),1);
    }

    for (int8_t cur_extruder = 0; cur_extruder < Jawsboard::getBoard().tool_count; ++cur_extruder) {
          SERIAL_PROTOCOL(" T");
          SERIAL_PROTOCOL(cur_extruder);
          SERIAL_PROTOCOL(":");
          SERIAL_PROTOCOL_F(Jawsboard::getBoard().getCurrentDegExtruder(cur_extruder),1);
          SERIAL_PROTOCOL(" /");
          SERIAL_PROTOCOL_F(Jawsboard::getBoard().getTargetDegExtruder(cur_extruder),1);
    }

    SERIAL_PROTOCOL(" @:");
    SERIAL_PROTOCOL(Jawsboard::getBoard().getHeaterPower(tmp_extruder));
    SERIAL_PROTOCOL(" B@:");
    SERIAL_PROTOCOL(Jawsboard::getBoard().getHeaterPower(-1));
    SERIAL_PROTOCOLLN("");
}

static void readCurrentPosition()
{
    SERIAL_PROTOCOL("X:");
    SERIAL_PROTOCOL(current_position[X_AXIS]);
    SERIAL_PROTOCOL(" Y:");
    SERIAL_PROTOCOL(current_position[Y_AXIS]);
    SERIAL_PROTOCOL(" Z:");
    SERIAL_PROTOCOL(current_position[Z_AXIS]);
    SERIAL_PROTOCOL(" E:");
    SERIAL_PROTOCOL(current_position[E_AXIS]);

    SERIAL_PROTOCOL(MSG_COUNT_X);
    SERIAL_PROTOCOL(float(st_get_position(X_AXIS))/axis_steps_per_unit[X_AXIS]);
    SERIAL_PROTOCOL(" Y:");
    SERIAL_PROTOCOL(float(st_get_position(Y_AXIS))/axis_steps_per_unit[Y_AXIS]);
    SERIAL_PROTOCOL(" Z:");
    SERIAL_PROTOCOL(float(st_get_position(Z_AXIS))/axis_steps_per_unit[Z_AXIS]);

    SERIAL_PROTOCOLLN("");
}

static void setSpeedFactor()
{
    if(code_seen('S')){
        feedmultiply = code_value() ;
    }
}

static void setExtrudeFactor()
{
    if(code_seen('S')){
        extrudemultiply = code_value() ;
    }
}

static void takeTimeSinceLast()
{
    stoptime=millis();
    char time[30];
    unsigned long t=(stoptime-starttime)/1000;
    int secs,mins;
    mins=t/60;
    secs=t%60;
    sprintf(time,"%i min, %i sec", mins, secs);
    SERIAL_ECHO_START;
    SERIAL_ECHOLN(time);
    //autotempShutdown();
}

static void readEndstopStatus()
{
    SERIAL_PROTOCOLLN(MSG_M119_REPORT);
    #if defined(X_MIN_PIN) && X_MIN_PIN > -1
        SERIAL_PROTOCOL(MSG_X_MIN);
        SERIAL_PROTOCOLLN(((READ(X_MIN_PIN)^X_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
    #if defined(X_MAX_PIN) && X_MAX_PIN > -1
        SERIAL_PROTOCOL(MSG_X_MAX);
        SERIAL_PROTOCOLLN(((READ(X_MAX_PIN)^X_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
    #if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
        SERIAL_PROTOCOL(MSG_Y_MIN);
        SERIAL_PROTOCOLLN(((READ(Y_MIN_PIN)^Y_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
    #if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
        SERIAL_PROTOCOL(MSG_Y_MAX);
        SERIAL_PROTOCOLLN(((READ(Y_MAX_PIN)^Y_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
    #if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
        SERIAL_PROTOCOL(MSG_Z_MIN);
        SERIAL_PROTOCOLLN(((READ(Z_MIN_PIN)^Z_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
    #if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
        SERIAL_PROTOCOL(MSG_Z_MAX);
        SERIAL_PROTOCOLLN(((READ(Z_MAX_PIN)^Z_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
    #endif
}

static void setHomingoffset()
{
    for(int8_t i=0; i < 3; i++){
        if(code_seen(axis_codes[i])) add_homing[i] = code_value();
    }
    eeprom::setCustom_addHoming(add_homing);
}

static void filamentChange()
{
    float target[4];
    float lastpos[4];

    target[X_AXIS] = current_position[X_AXIS];
    target[Y_AXIS] = current_position[Y_AXIS];
    target[Z_AXIS] = current_position[Z_AXIS];
    target[E_AXIS] = current_position[E_AXIS];

    lastpos[X_AXIS] = current_position[X_AXIS];
    lastpos[Y_AXIS] = current_position[Y_AXIS];
    lastpos[Z_AXIS] = current_position[Z_AXIS];
    lastpos[E_AXIS] = current_position[E_AXIS];

    //retract by E
    if(code_seen('E')){
        target[E_AXIS]+= code_value();
    }
    else{
        #ifdef FILAMENTCHANGE_FIRSTRETRACT
        target[E_AXIS]+= FILAMENTCHANGE_FIRSTRETRACT ;
        #endif
    }
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

    //lift Z
    if(code_seen('Z')){
        target[Z_AXIS]+= code_value();
    }
    else{
    #ifdef FILAMENTCHANGE_ZADD
        target[Z_AXIS]+= FILAMENTCHANGE_ZADD;
    #endif
    }
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

    //move xy
    if(code_seen('X')){
        target[X_AXIS] = code_value();
    }
    else{
    #ifdef FILAMENTCHANGE_XPOS
        target[X_AXIS] = FILAMENTCHANGE_XPOS ;
    #endif
    }

    if(code_seen('Y')){
        target[Y_AXIS]= code_value();
    }
    else{
    #ifdef FILAMENTCHANGE_YPOS
        target[Y_AXIS]= FILAMENTCHANGE_YPOS ;
    #endif
    }
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

    if(code_seen('L')){
        target[E_AXIS]+= code_value();
    }
    else
    {
    #ifdef FILAMENTCHANGE_FINALRETRACT
        target[E_AXIS]+= FILAMENTCHANGE_FINALRETRACT ;
    #endif
    }
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

    //finish moves
    st_synchronize();
    //disable extruder steppers so filament can be removed
    disable_e0();
    disable_e1();
    disable_e2();
    HAL_Delay(100);

    uint32_t start = HAL_GetTick();
    uint8_t cnt=0;
    while( HAL_GetTick() - start < 60000 ){
        cnt++;
        Jawsboard::getBoard().runboardSlice();
        manage_inactivity();
        if(cnt==0){
        }
    }

    //return to normal
    if(code_seen('L')){
        target[E_AXIS]+= -code_value();
    }
    else{
    #ifdef FILAMENTCHANGE_FINALRETRACT
        target[E_AXIS]+=(-1)*FILAMENTCHANGE_FINALRETRACT ;
    #endif
    }
    current_position[E_AXIS]=target[E_AXIS]; //the long retract of L is compensated by manual filament feeding
    plan_set_e_position(current_position[E_AXIS]);
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //should do nothing
    plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move xy back
    plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move z back
    plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], lastpos[E_AXIS], feedrate/60, active_extruder); //final untretract
}

static void setMotorCurrent()
{
    if(code_seen('X')) {
        digitalPotWrite(0, code_value());
    }
    if(code_seen('Y')) {
        digitalPotWrite(1, code_value());
    }
    if(code_seen('Z')) {
        digitalPotWrite(2, code_value());
    }
    if(code_seen('E')) {
        digitalPotWrite(3, code_value());
    }
}

static void restartAfterStopped()
{
    Stopped = false;

    gcode_LastN = Stopped_gcode_LastN;
    FlushSerialRequestResend();
}

static void selectSDFile()
{
    char* starpos = (strchr(strchr_pointer + 4,'*'));
    if(starpos != NULL)
        *(starpos) = '\0';
    sd_card.openFile(strchr_pointer + 4,true);
}

static void deleteSDFile()
{
    if (sd_card.isCardOK()){
        sd_card.closeFile();
        char *starpos = (strchr(strchr_pointer + 4,'*'));
        if(starpos != NULL){
            char* npos = strchr(cmdbuffer[bufindr], 'N');
            strchr_pointer = strchr(npos,' ') + 1;
            *(starpos) = '\0';
        }
        sd_card.removeFile(strchr_pointer + 4);
    }
}

static void startSDPrint()
{
    sd_card.startPrint();
    starttime = millis();
}

static void pauseSDPrint()
{
    sd_card.pausePrint();
}

static void cancelSDPrint()
{
    sd_card.cancelPrint();
}

static void startSDWrite()
{
    char* starpos = (strchr(strchr_pointer + 4,'*'));
    if(starpos != NULL){
        char* npos = strchr(cmdbuffer[bufindr], 'N');
        strchr_pointer = strchr(npos,' ') + 1;
        *(starpos) = '\0';
    }
    sd_card.eraseForSpace();
    if(sd_card.openFile(strchr_pointer+4,false)){
        SERIAL_PROTOCOLLN("SD Create File Success!");
        sd_card.startFileTransfer();
    }else{
        SERIAL_PROTOCOLLN("SD Create File False!");
    }
}

static void stopSDWrite()
{
    sd_card.stopFileTransfer();
    sd_card.closeFile();
}

static void get_coordinates()
{
    for(int8_t i=0; i < NUM_AXIS; i++) {
        if(code_seen(axis_codes[i])){
            destination[i] = (float)code_value() + (axis_relative_modes[i] || relative_mode)*current_position[i];
        }else
            destination[i] = current_position[i]; //Are these else lines really needed?
    }
    if(code_seen('F')) {
        next_feedrate = code_value();
        if(next_feedrate > 0.0) feedrate = next_feedrate;
    }
}

static void prepare_move(){
    clamp_to_software_endstops(destination);
    previous_millis_cmd = millis();

    // Do not use feedmultiply for E or Z only moves
    if( (current_position[X_AXIS] == destination [X_AXIS]) && (current_position[Y_AXIS] == destination [Y_AXIS])) {
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
    }else {
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate*feedmultiply/60/100.0, active_extruder);
    }

    for(int8_t i=0; i < NUM_AXIS; i++) {
        current_position[i] = destination[i];
    }
}

static void get_arc_coordinates()
{
    get_coordinates();

    if(code_seen('I')) {
        offset[0] = code_value();
    }
    else {
        offset[0] = 0.0;
    }
    if(code_seen('J')) {
        offset[1] = code_value();
    }
    else {
        offset[1] = 0.0;
    }
}

static void prepare_arc_move(char isclockwise)
{
    float r = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc

    // Trace the arc
    mc_arc(current_position, destination, offset, X_AXIS, Y_AXIS, Z_AXIS, feedrate*feedmultiply/60/100.0, r, isclockwise, active_extruder);

    // As far as the parser is concerned, the position is now == target.
    //In reality the motion control system might still be processing the action and the real tool position in any intermediate location.
    for(int8_t i=0; i < NUM_AXIS; i++) {
        current_position[i] = destination[i];
    }
    previous_millis_cmd = millis();
}


static void axis_is_at_home(int axis) {
  current_position[axis] = base_home_pos(axis) + add_homing[axis];
  min_pos[axis] =          base_min_pos(axis) + add_homing[axis];
  max_pos[axis] =          base_max_pos(axis) + add_homing[axis];
}

static bool HOMEAXIS_DO(int axis)
{
    bool dohome = false;
    if (axis == X_AXIS){
        #if (X_MIN_PIN > -1)
        dohome = dohome || (X_HOME_DIR == -1);
        #endif
        #if (X_MAX_PIN > -1)
        dohome = dohome || (X_HOME_DIR == 1);
        #endif
    }else if (axis == Y_AXIS){
        #if (Y_MIN_PIN > -1)
        dohome = dohome ||  (Y_HOME_DIR == -1);
        #endif
        #if (Y_MAX_PIN > -1)
        dohome = dohome ||  (Y_HOME_DIR == 1);
        #endif
    }else if (axis == Z_AXIS){
        #if (Z_MIN_PIN > -1)
        dohome = dohome ||  (Z_HOME_DIR == -1);
        #endif
        #if (Z_MAX_PIN > -1)
        dohome = dohome ||  (Z_HOME_DIR == 1);
        #endif
    }

    return dohome;
}

static void homeaxis(int axis) {

    if( axis==X_AXIS ? HOMEAXIS_DO(X_AXIS ) : (axis==Y_AXIS ? HOMEAXIS_DO(Y_AXIS) : (axis==Z_AXIS ? HOMEAXIS_DO(Z_AXIS) : 0))){
        int axis_home_dir = home_dir(axis);

        current_position[axis] = 0;
        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

        destination[axis] = 1.5 * max_length(axis) * axis_home_dir;
        feedrate = homing_feedrate[axis];
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
        st_synchronize();

        current_position[axis] = 0;
        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
        destination[axis] = -home_retract_mm(axis) * axis_home_dir;
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
        st_synchronize();

        destination[axis] = 2*home_retract_mm(axis) * axis_home_dir;

        feedrate = homing_feedrate[axis]/2;

        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
        st_synchronize();

        axis_is_at_home(axis);
        destination[axis] = current_position[axis];
        feedrate = 0.0;
        endstops_hit_on_purpose();
        axis_known_position[axis] = true;
    }
}

static void engage_z_probe()
{

}

static void retract_z_probe()
{

}

static void run_z_probe()
{
    plan_bed_level_matrix.set_to_identity();
    feedrate = homing_feedrate[Z_AXIS];

    // move down until you find the bed
    float zPosition = -10;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

        // we have to let the planner know where we are right now as it is not where we said to go.
    zPosition = st_get_position_mm(Z_AXIS);
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS]);

    // move up the retract distance
    zPosition += home_retract_mm(Z_AXIS);
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    // move back down slowly to find bed
    feedrate = homing_feedrate[Z_AXIS]/4;
    zPosition -= home_retract_mm(Z_AXIS) * 2;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    current_position[Z_AXIS] = st_get_position_mm(Z_AXIS);
    // make sure the planner knows where we are as it may be a bit different than we last said to move to
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

static void do_blocking_move_to(float x, float y, float z)
{
    float oldFeedRate = feedrate;

    feedrate = homing_feedrate[Z_AXIS];

    current_position[Z_AXIS] = z;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    feedrate = XY_TRAVEL_SPEED;

    current_position[X_AXIS] = x;
    current_position[Y_AXIS] = y;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    feedrate = oldFeedRate;
}

static void do_blocking_move_relative(float offset_x, float offset_y, float offset_z)
{
    do_blocking_move_to(current_position[X_AXIS] + offset_x, current_position[Y_AXIS] + offset_y, current_position[Z_AXIS] + offset_z);
}

static void setup_for_endstop_move()
{
    saved_feedrate = feedrate;
    saved_feedmultiply = feedmultiply;
    feedmultiply = 100;
    previous_millis_cmd = millis();

    enable_endstops(true);
}

/// Probe bed height at position (x,y), returns the measured z value
static float probe_pt(float x, float y, float z_before)
{
    // move to right place
    do_blocking_move_to(current_position[X_AXIS], current_position[Y_AXIS], z_before);
    do_blocking_move_to(x - X_PROBE_OFFSET_FROM_EXTRUDER, y - Y_PROBE_OFFSET_FROM_EXTRUDER, current_position[Z_AXIS]);

    //engage probe
    engage_z_probe();

    //run probe
    run_z_probe();
    float measured_z = current_position[Z_AXIS];

    //retract probe
    retract_z_probe();

    SERIAL_PROTOCOL(MSG_BED);
    SERIAL_PROTOCOL(" x: ");
    SERIAL_PROTOCOL(x);
    SERIAL_PROTOCOL(" y: ");
    SERIAL_PROTOCOL(y);
    SERIAL_PROTOCOL(" z: ");
    SERIAL_PROTOCOL(measured_z);
    SERIAL_PROTOCOL("\n");

    return measured_z;
}

static void clean_up_after_endstop_move()
{
#ifdef ENDSTOPS_ONLY_FOR_HOMING
    enable_endstops(false);
#endif

    feedrate = saved_feedrate;
    feedmultiply = saved_feedmultiply;
    previous_millis_cmd = millis();
}

static void set_bed_level_equation_3pts(float z_at_pt_1, float z_at_pt_2, float z_at_pt_3)
{

    plan_bed_level_matrix.set_to_identity();

    vector_3 pt1 = vector_3(ABL_PROBE_PT_1_X, ABL_PROBE_PT_1_Y, z_at_pt_1);
    vector_3 pt2 = vector_3(ABL_PROBE_PT_2_X, ABL_PROBE_PT_2_Y, z_at_pt_2);
    vector_3 pt3 = vector_3(ABL_PROBE_PT_3_X, ABL_PROBE_PT_3_Y, z_at_pt_3);

    vector_3 from_2_to_1 = (pt1 - pt2).get_normal();
    vector_3 from_2_to_3 = (pt3 - pt2).get_normal();
    vector_3 planeNormal = vector_3::cross(from_2_to_1, from_2_to_3).get_normal();
    planeNormal = vector_3(planeNormal.x, planeNormal.y, abs(planeNormal.z));

    plan_bed_level_matrix = matrix_3x3::create_look_at(planeNormal);

    vector_3 corrected_position = plan_get_position();
    current_position[X_AXIS] = corrected_position.x;
    current_position[Y_AXIS] = corrected_position.y;
    current_position[Z_AXIS] = corrected_position.z;

    // put the bed at 0 so we don't go below it.
    current_position[Z_AXIS] = zprobe_zoffset;

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

static bool setTargetedHotend(int code)
{
    tmp_extruder = active_extruder;
    if(code_seen('T')) {
        tmp_extruder = code_value();
        if(tmp_extruder >= Jawsboard::getBoard().tool_count) {
            SERIAL_ECHO_START;
            switch(code){
            case 104:
                SERIAL_ECHO(MSG_M104_INVALID_EXTRUDER);
                break;
            case 105:
                SERIAL_ECHO(MSG_M105_INVALID_EXTRUDER);
                break;
            case 109:
                SERIAL_ECHO(MSG_M109_INVALID_EXTRUDER);
                break;
            case 218:
                SERIAL_ECHO(MSG_M218_INVALID_EXTRUDER);
                break;
            case 221:
                SERIAL_ECHO(MSG_M221_INVALID_EXTRUDER);
                break;
            }
            SERIAL_ECHOLN(tmp_extruder);

            return true;
        }
    }
    return false;
}

void init()
{
    eeprom::eeprom_command_init();
    filamentDetect_init();

    for(int8_t i = 0; i < BUFSIZE; i++){
        fromsd[i] = false;
    }
    sd_card.init();
}

void kill()
{
    __disable_irq(); // Stop interrupts

    Jawsboard::getBoard().heatersOff(true);

    disable_x();
    disable_y();
    disable_z();
    disable_e0();
    disable_e1();
    disable_e2();

    //power manage

    //
    SERIAL_ERROR_START;
    SERIAL_ERRORLN(MSG_ERR_KILLED);

    while(1) { /* Intentionally left empty */ } // Wait for reset
}

void Stop()
{
    Jawsboard::getBoard().heatersOff(true);
    if(Stopped == false) {
        Stopped = true;
        Stopped_gcode_LastN = gcode_LastN;
        SERIAL_ERROR_START;
        SERIAL_ERRORLN(MSG_ERR_STOPPED);
    }
}

void manage_inactivity()
{
    if(buflen < (BUFSIZE-1))
        get_command();

    if( (millis() - previous_millis_cmd) >  max_inactive_time )
        if(max_inactive_time)
            kill();
    if(stepper_inactive_time)  {
        if( (millis() - previous_millis_cmd) >  stepper_inactive_time ){
            if(blocks_queued() == false) {
                if(DISABLE_X) disable_x();
                if(DISABLE_Y) disable_y();
                if(DISABLE_Z) disable_z();
                if(DISABLE_E) {
                    disable_e0();
                    disable_e1();
                    disable_e2();
                }
            }
        }
    }

    #if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1
    controllerFan(); //Check if fan should be turned on to cool stepper drivers down
    #endif

    check_axes_activity();
}

void clamp_to_software_endstops(float target[3])
{
    if (min_software_endstops) {
        if (target[X_AXIS] < min_pos[X_AXIS]) target[X_AXIS] = min_pos[X_AXIS];
        if (target[Y_AXIS] < min_pos[Y_AXIS]) target[Y_AXIS] = min_pos[Y_AXIS];

        float negative_z_offset = 0;

        #ifdef ENABLE_AUTO_BED_LEVELING
        if (Z_PROBE_OFFSET_FROM_EXTRUDER < 0) negative_z_offset = negative_z_offset + Z_PROBE_OFFSET_FROM_EXTRUDER;
        if (add_homing[Z_AXIS] < 0) negative_z_offset = negative_z_offset + add_homing[Z_AXIS];
        #endif

        if (target[Z_AXIS] < min_pos[Z_AXIS]+negative_z_offset) target[Z_AXIS] = min_pos[Z_AXIS]+negative_z_offset;
    }

    if (max_software_endstops) {
        if (target[X_AXIS] > max_pos[X_AXIS]) target[X_AXIS] = max_pos[X_AXIS];
        if (target[Y_AXIS] > max_pos[Y_AXIS]) target[Y_AXIS] = max_pos[Y_AXIS];
        if (target[Z_AXIS] > max_pos[Z_AXIS]) target[Z_AXIS] = max_pos[Z_AXIS];
    }
}

bool IsStopped()
{
    return Stopped;
}

}
