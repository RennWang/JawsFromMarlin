/**
  ******************************************************************************
  * @file    DigitalIn.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    3-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef DIGITALIN_H
#define DIGITALIN_H

#include "stm32f4xx_hal.h"

/** A digital input, used for reading the state of a pin
 *
 * Example:
 * @code
 * // Flash an LED while a DigitalIn is true
 *
 *
 * DigitalIn enable(p5);
 * DigitalOut led(LED1);
 *
 * int main() {
 *     while(1) {
 *         if(enable) {
 *             led = !led;
 *         }
 *         wait(0.25);
 *     }
 * }
 * @endcode
 */
class DigitalIn {

public:
    /** Create a DigitalIn connected to the specified pin
     *
     *  @param pin DigitalIn pin to connect to
     */
    DigitalIn(PinName pin) : gpio() {
        gpio_init_in(&gpio, pin);
    }

    /** Create a DigitalIn connected to the specified pin
     *
     *  @param pin DigitalIn pin to connect to
     *  @param mode the initial mode of the pin
     */
    DigitalIn(PinName pin, PinMode mode) : gpio() {
        gpio_init_in_ex(&gpio, pin, mode);
    }
    /** Read the input, represented as 0 or 1 (int)
     *
     *  @returns
     *    An integer representing the state of the input pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read() {
        return gpio_read(&gpio);
    }

    /** Set the input pin mode
     *
     *  @param mode PullUp, PullDown, PullNone, OpenDrain
     */
    void mode(PinMode pull) {
        gpio_mode(&gpio, pull);
    }

protected:
    gpio_t gpio;
};



#endif
