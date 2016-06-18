/**
  ******************************************************************************
  * @file    Command.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    8-May-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef COMMAND_H
#define COMMAND_H

#include "Jaws_main.h"
#include "Configuration.h"

#include "Protocol.h"
//#include "SerialPort.h"
#include "Usb.h"

#define commandSerial     usbSerial
//#define commandSerial     commdSerial

#define SERIAL_PROTOCOL(x)      (commandSerial.print(x))
#define SERIAL_PROTOCOL_F(x,y)  (commandSerial.print(x,y))
#define SERIAL_PROTOCOLLN(x)    (commandSerial.print(x),commandSerial.print('\n'),commandSerial.transmit())

#define SERIAL_ERROR_START      (commandSerial.print(errormagic))
#define SERIAL_ERROR(x)         SERIAL_PROTOCOL(x)
#define SERIAL_ERRORLN(x)       SERIAL_PROTOCOLLN(x)

#define SERIAL_ECHO_START       (commandSerial.print(echomagic))
#define SERIAL_ECHO(x)          SERIAL_PROTOCOL(x)
#define SERIAL_ECHOLN(x)        SERIAL_PROTOCOLLN(x)
#define SERIAL_ECHOPAIR(name,value)    (SERIAL_PROTOCOL(name),SERIAL_PROTOCOL(value))

const char errormagic[] = "Error:";
const char echomagic[] = "echo:";

namespace command{

extern float homing_feedrate[];
extern bool axis_relative_modes[];
extern int feedmultiply; //100->1 200->2
extern int extrudemultiply; //100->1 200->2
extern float current_position[NUM_AXIS];
extern float add_homing[3];
extern float min_pos[3];
extern float max_pos[3];

extern bool axis_known_position[3];
extern float zprobe_zoffset;

// Extruder offset, only in XY plane
#if EXTRUDER_COUNT > 1
extern float extruder_offset[2][EXTRUDER_COUNT];
#endif

extern uint8_t active_extruder;
extern uint8_t fanSpeed;
extern uint8_t fanSpeedPercent;


void enquecommand(const char *cmd);

float code_value();
long code_value_long();
bool code_seen(char code);

void get_command();
void process_commands();
void runCommandSlice();

void process_Gcommands(int codevalue);
void process_Mcommands(int codevalue);
void process_Tcommands(int codevalue);

void init();
void kill();
void Stop();

void manage_inactivity();
void clamp_to_software_endstops(float target[3]);
bool IsStopped();
}

#endif /* COMMAND_H */
