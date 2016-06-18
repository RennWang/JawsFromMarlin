#include "Estop.h"
#include "hw_gpio.h"
#include "Command.h"

void estop_init()
{
    SET_INPUT(PD_3);
}

void estop_check()
{
    if (!READ(PD_3))
        command::kill();
}
