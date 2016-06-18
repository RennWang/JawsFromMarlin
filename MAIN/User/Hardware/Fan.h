#ifndef FAN_H
#define FAN_H

#include "Jaws_main.h"

class Fan
{
public:
    Fan(uint8_t index);

    void init();
    void setSpeed(uint8_t fanspeed);
    uint8_t getSpeed(){return fan_speed;}

private:
    uint8_t fan_speed;
    uint8_t fan_index;
    uint8_t fan_init;
};

extern Fan pwmfan_one;
extern Fan pwmfan_two;

//extern void freqInit();
//extern void setFreqOut(uint32_t Freq);
//extern void stopFreqOut();
#endif /* FAN_H */
