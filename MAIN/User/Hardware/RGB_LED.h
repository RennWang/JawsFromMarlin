#ifndef RGB_LED_H
#define RGB_LED_H

#include "Jaws_main.h"

 // LED control registers
 #define LED_REG_PSC0	    0b00000001
 #define LED_REG_PWM0	    0b00000010
 #define LED_REG_PSC1	    0b00000011
 #define LED_REG_PWM1	    0b00000100
 #define LED_REG_SELECT     0b00000101


 // LED output types
 #define LED_BLINK_PWM0     0b10101010
 #define LED_BLINK_PWM1     0b11111111
 #define LED_ON 	        0b01010101
 #define LED_OFF	        0b00000000

 // RBG IDs
 #define LED_GREEN          0b00001100
 #define LED_RED            0b00000011
 #define LED_BLUE           0b00110000

 // Channel IDs
 #define LED_CHANNEL1	0
 #define LED_CHANNEL2	1

enum LEDColors{
    LED_DEFAULT_WHITE = 0,
    LED_DEFAULT_RED,
    LED_DEFAULT_ORANGE,
    LED_DEFAULT_PINK,
    LED_DEFAULT_GREEN,
    LED_DEFAULT_BLUE,
    LED_DEFAULT_PURPLE,
    LED_DEFAULT_OFF,
    LED_DEFAULT_CUSTOM
};

class RGB_LED
{
public:
    RGB_LED();

    void init();
    void startupSequence();
    void errorSequence();
    void setColor(uint8_t red, uint8_t green, uint8_t blue, bool clearOld = true);
    void setDefaultColor();
    void setCustomColor(uint8_t red, uint8_t green, uint8_t blue);

    void clear();

    void setBrightness(uint8_t level, uint8_t LED1, uint8_t LED2);
    void setBlinkOff(uint8_t channel, uint8_t LEDs);
    void setBlinkRate(uint8_t channel, uint8_t LEDs, uint8_t rate);
    void toggleLEDNoPWM(bool enable, uint8_t LED);
private:
	uint8_t LEDSelect;
	uint8_t blinkRate;
	bool LEDEnabled;
};

extern RGB_LED rgb_led;
#endif /* RGB_LED_H */
