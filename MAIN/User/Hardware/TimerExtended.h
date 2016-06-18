#ifndef TIMEREXTENDED_H
#define TIMEREXTENDED_H

#include "Jaws_main.h"

#define MAX_TIMER_EXTENDED_COUNT    10

typedef void (*TimerIrq)(void);

class TimerExtended
{
public:
    uint32_t timebase_ms;
    TimerIrq TimerIrqCallback;

public:
    TimerExtended(uint8_t id);

	void start(uint32_t timebase, TimerIrq interruptcallback);

    void stop();

    static bool isOccupied(uint8_t id);

private:
    uint8_t extend_id;
};

void TimerExtendedIrqHandler(void);

#endif /* TIMEREXTENDED_H */
