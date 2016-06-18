/**
  ******************************************************************************
  * @file    DigitalOut.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    3-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef MBED_DIGITALOUT_H
#define MBED_DIGITALOUT_H

#include "gpio_api.h"

/** A digital output, used for setting the state of a pin
 *
 * Example:
 * @code
 * // Toggle a LED
 * #include "mbed.h"
 *
 * DigitalOut led(LED1);
 *
 * int main() {
 *     while(1) {
 *         led = !led;
 *         wait(0.2);
 *     }
 * }
 * @endcode
 */
class DigitalOut {

public:
    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     */
    DigitalOut(PinName pin) : gpio() {
        gpio_init_out(&gpio, pin);
    }

    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     *  @param value the initial pin value
     */
    DigitalOut(PinName pin, int value) : gpio() {
        gpio_init_out_ex(&gpio, pin, PullNone,value);
    }

    /** Set the output, specified as 0 or 1 (int)
     *
     *  @param value An integer specifying the pin output value,
     *      0 for logical 0, 1 (or any other non-zero value) for logical 1
     */
    void write(int value) {
        gpio_write(&gpio, value);
    }

    /** Return the output setting, represented as 0 or 1 (int)
     *
     *  @returns
     *    an integer representing the output setting of the pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read() {
        return gpio_read(&gpio);
    }

protected:
    gpio_t gpio;
};

#endif
