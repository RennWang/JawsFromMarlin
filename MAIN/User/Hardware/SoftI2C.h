#ifndef SOFTI2C_H
#define SOFTI2C_H

#include "hw_gpio.h"

class SoftI2C
{
public:
    SoftI2C(PinName sda, PinName scl);
    void init();
    void Start(void);
    void Stop(void);
    uint8_t Wait_Ack(void);
    void Ack(void);
    void NAck(void);

    void Send_Byte(uint8_t txd);
    uint8_t Read_Byte(uint8_t ack);
private:
    PinName _sda;
    PinName _scl;
};

extern SoftI2C i2c_one;
extern SoftI2C i2c_two;

#endif /* SOFTI2C_H */
