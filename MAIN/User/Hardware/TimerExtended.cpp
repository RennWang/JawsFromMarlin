#include "TimerExtended.h"

typedef struct
{
    __IO uint32_t isUsed;
    __IO uint32_t starttick;
    __IO uint32_t timebase_ms;
    __IO TimerIrq TimerExtendedInterrupt;
}TimerExtendedStruct;

static TimerExtendedStruct ext_timers[MAX_TIMER_EXTENDED_COUNT] = {0};

bool TimerExtended::isOccupied(uint8_t id)
{
    return (ext_timers[id].isUsed != 0);
}

TimerExtended::TimerExtended(uint8_t id)
{
    extend_id = id;
}

void TimerExtended::start(uint32_t timebase, TimerIrq interruptcallback)
{
    ext_timers[extend_id].isUsed = 0;

        timebase_ms = timebase;
        TimerIrqCallback = interruptcallback;
        ext_timers[extend_id].starttick = HAL_GetTick();
        ext_timers[extend_id].TimerExtendedInterrupt = interruptcallback;
        ext_timers[extend_id].timebase_ms = timebase;

    ext_timers[extend_id].isUsed = 1;
}

void TimerExtended::stop()
{
    ext_timers[extend_id].isUsed = 0;

    timebase_ms = 0;
    TimerIrqCallback = 0;
    ext_timers[extend_id].TimerExtendedInterrupt = 0;
    ext_timers[extend_id].timebase_ms = 0;
}

void TimerExtendedIrqHandler(void)
{
    uint32_t i;
    for (i = 0; i<MAX_TIMER_EXTENDED_COUNT; i++){
        if (ext_timers[i].isUsed){
            if (ext_timers[i].timebase_ms != 0 && ext_timers[i].TimerExtendedInterrupt != NULL){
                if (HAL_GetTick() - ext_timers[i].starttick >= ext_timers[i].timebase_ms){
                    ext_timers[i].starttick = HAL_GetTick();
                    ext_timers[i].TimerExtendedInterrupt();
                }
            }
        }
    }
}
