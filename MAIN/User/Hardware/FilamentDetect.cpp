#include "FilamentDetect.h"
#include "hw_gpio.h"
#include "Timeout.h"
#include "Command.h"


#define FILAMENT_DETECT_TIME_OUT    500
//static bool is_detected;
//static Timeout filamentdect_timeout;

void filamentDetect_init()
{
    SET_INPUT(PE_0);
    //is_detected = false;
    //filamentdect_timeout.clear();
}

bool filamentDetect_check()
{
    return READ(PE_0);
//    if (!is_detected){
//        if (!READ(PD_1)){
//            is_detected = true;
//            filamentdect_timeout.start(FILAMENT_DETECT_TIME_OUT);
//        }
//    }
//    else{
//        if (filamentdect_timeout.isActive()){
//            if (filamentdect_timeout.hasElapsed()){
//                if (!READ(PD_1)){
//                    command::requestPause(MSG_PAUSE_FILAMENT);
//                    filamentdect_timeout.abort();
//                    filamentdect_timeout.clear();
//                }
//            }
//        }
//        else{
//            if (READ(PD_1)){
//                is_detected = false;
//            }
//        }
//    }
}
