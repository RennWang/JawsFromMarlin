#include "RGB_LED.h"
#include "SoftI2C.h"
#include "delay.h"

#define _delay_us(x) HAL_Delay_us(x)

static void TWI_write_data(uint8_t addr, uint8_t* data, uint8_t size)
{
    i2c_one.Start();
    i2c_one.Send_Byte(addr);
    i2c_one.Wait_Ack();
    for (int i = 0; i< size; i++){
        i2c_one.Send_Byte(data[i]);
        i2c_one.Wait_Ack();
    }
    i2c_one.Stop();
}

//7 bit addr , add 0 after addr
const static int LEDAddress = 0B11000100;

RGB_LED::RGB_LED()
{

}

void RGB_LED::init()
{
     setDefaultColor();
}

// channel : 1,2 select PWM channels, 3 is a pure on / off channel
// level : duty cycle (brightness) for channels 1,2,
//			for Channel 3, level is on if not zero
// LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Red:0b1100, Green:0b11}
//  		ones indicate on, zeros indicate off
void RGB_LED::setBrightness(uint8_t Channel, uint8_t level, uint8_t LEDs)
{
     uint8_t data1[2] = {LED_REG_SELECT, 0};
     uint8_t data2[2] = {0, level};

     // set pwm for select channel
     if (Channel == LED_CHANNEL1){
        data2[0] = LED_REG_PWM0;
        // clear past select data and apply PWM0
        data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
     }
     else if (Channel == LED_CHANNEL2){
        data2[0] = LED_REG_PWM1;
        // clear past select data and apply PWM1
        data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
     }
     else {
        toggleLEDNoPWM((level != 0), LEDs);
        return;
     }

     TWI_write_data(LEDAddress, data1, 2); //uint8_t error
     _delay_us(1);
     TWI_write_data(LEDAddress, data2, 2); //uint8_t error
     _delay_us(1);

     LEDSelect = data1[1];
}

// channel : 1,2 select PWM channels, channel 3 does nothing
// level : blink rate for channels 1,2,  channel 3 ignores this
// LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11}
//  		ones indicate on, zeros indicate off
void RGB_LED::setBlinkOff(uint8_t Channel, uint8_t LEDs)
{
     setBlinkRate(Channel, LEDs, 0);
}

void RGB_LED::setBlinkRate(uint8_t Channel, uint8_t LEDs, uint8_t rate)
{
     uint8_t data1[2] = {LED_REG_SELECT, 0};
     uint8_t data2[2] = {0 , rate};

     // set pwm for select channel
     if (Channel == LED_CHANNEL1){
        data2[0] = LED_REG_PSC0;
        // clear past select data and apply PWM0
        data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
     }
     else if (Channel == LED_CHANNEL2){
        data2[0] = LED_REG_PSC1;
        // clear past select data and apply PWM1
        data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
     }
     else
        return;

     TWI_write_data(LEDAddress, data1, 2); //uint8_t error
     _delay_us(1);
     TWI_write_data(LEDAddress, data2, 2); //uint8_t error
     _delay_us(1);

     LEDSelect = data1[1];
}

// channel 3 sets LEDs on or off
// LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11}
//  		ones indicate on, zeros indicate off
void RGB_LED::toggleLEDNoPWM(bool enable, uint8_t LEDs)
{
     uint8_t data[2] = {LED_REG_SELECT, 0};

     if ( enable )
        // clear past select data and turn LEDs full on
        data[1] = (LEDSelect & ~LEDs) | (LED_ON & LEDs);
     else
        // clear past select data and turn LEDs full off
        data[1] = (LEDSelect & ~LEDs) | (LED_OFF & LEDs);

     TWI_write_data(LEDAddress, data, 2); //uint8_t error

     LEDSelect = data[1];
}

void RGB_LED::startupSequence()
{
}

void RGB_LED::clear()
{
     // clear LEDs
     setBrightness(3, 0, LED_RED | LED_GREEN | LED_BLUE);
}

void RGB_LED::errorSequence()
{
     clear();
     // set blinking red lights
     setBrightness(1, 200, LED_RED);
     setBlinkRate(1, LED_RED, 130);
}

void RGB_LED::setDefaultColor()
{

     clear();

     // set frequency to slowest and duty cyle to zero (off)
     uint8_t LEDColor = LED_DEFAULT_RED;
     uint32_t CustomColor = 0xC800C800;

     // blink rate has to be set first in order for color to register,
     // so set blink before each color
     LEDEnabled = true;

     uint8_t color;
     uint8_t intensity = 100;

     switch(LEDColor){
     default :
     case LED_DEFAULT_WHITE:
        color =  LED_RED | LED_GREEN | LED_BLUE;
        break;
     case LED_DEFAULT_BLUE:
        color = LED_BLUE;
        break;
     case LED_DEFAULT_RED:
        color = LED_RED;
        break;
     case LED_DEFAULT_GREEN:
        color = LED_GREEN;
        break;
     case LED_DEFAULT_ORANGE:
        setBlinkOff(1, LED_GREEN);
        setBrightness(1, 50, LED_GREEN);
        color = LED_RED;
        intensity = 200;
        break;
     case LED_DEFAULT_PINK:
        color = LED_BLUE | LED_RED;
        intensity = 70;
        break;
     case LED_DEFAULT_PURPLE:
        color = LED_BLUE | LED_RED;
        intensity = 200;
        break;
     case LED_DEFAULT_CUSTOM:
        setColor(CustomColor >> 24, CustomColor >> 16, CustomColor >> 8, true);
        return;
     case LED_DEFAULT_OFF:
        LEDEnabled = false;
        return;
     }
     setBrightness(1, intensity, color);
}


// set LED color and store to EEPROM "custom" color area
void RGB_LED::setCustomColor(uint8_t red, uint8_t green, uint8_t blue)
{
     LEDEnabled = true;
     setColor(red, green, blue, true);
}

// wiggly: set a three value color using a 2 value driver (+ ON/OFF channel)
void RGB_LED::setColor(uint8_t red, uint8_t green, uint8_t blue, bool clearOld)
{

     if ( !LEDEnabled )
        return;

     if ( clearOld )
        clear();

     int count;
     count = 0;
     uint8_t leds_on = 0;

     // if any color is all on, set it to ON
     if ( red == 255 )
        leds_on |= LED_RED;
     if ( green == 255 )
        leds_on |= LED_GREEN;
     if ( blue == 255 )
        leds_on |= LED_BLUE;

     setBrightness(3, 1, leds_on);

     // find number of distict color values
     if ( !((red == 0) || (red == 255)) )
        count++;

     if ( !((green == 0) || (green == 255)) )
        count++;

     if ( !((blue == 0) || (blue == 255)) )
        count++;

     // we have two channels for brightness, if we have two settings
     // or less, just set the channels to the requested values
     int channel = 0;
     if ( count < 3 ) {
        if ( (red > 0) && (red < 255) ) {
            setBlinkOff(channel, LED_RED);
            setBrightness(channel++, red, LED_RED);
        }
        if ( (green > 0) && (green < 255) ) {
            setBlinkOff(channel, LED_GREEN);
            setBrightness(channel++, green, LED_GREEN);
        }
        if ( (blue > 0) && (blue < 255) ) {
            setBlinkOff(channel, LED_BLUE);
            setBrightness(channel++, blue, LED_BLUE);
        }
     }
     // if three different values are requested, set the two closest
     // values to be equal and use the same channel
     else {
        int distRB = abs(red - blue);
        int distRG = abs(red - green);
        int distBG = abs(blue - green);

        if ( distRB < distRG ) {
            /// red and blue closest
            if ( distRB < distBG ) {
                setBlinkOff(0, LED_GREEN);
                setBrightness(0, green, LED_GREEN);
                setBlinkOff(1, LED_RED | LED_BLUE);
                setBrightness(1, red, LED_RED | LED_BLUE);
            }
            /// blue and green closest
            else {
                setBlinkOff(0, LED_RED);
                setBrightness(0, red, LED_RED);
                setBlinkOff(1, LED_GREEN |LED_BLUE);
                setBrightness(1, green, LED_GREEN | LED_BLUE);
            }
        }
        else{
	       /// red and green closest
            if ( distRG < distBG ) {
                setBlinkOff(0, LED_BLUE);
                setBrightness(0, blue, LED_BLUE);
                setBlinkOff(1, LED_GREEN | LED_RED);
                setBrightness(1, green, LED_GREEN | LED_RED);
            }
            /// blue and green closest
            else {
                setBlinkOff(0, LED_RED);
                setBrightness(0, red, LED_RED);
                setBlinkOff(1, LED_GREEN |LED_BLUE);
                setBrightness(1, green, LED_GREEN | LED_BLUE);
            }
        }
    }
}

RGB_LED rgb_led;
