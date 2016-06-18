/**
  ******************************************************************************
  * @file    hw_I2CDigPot.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "delay.h"
#include "SoftI2C.h"

#define I2C_Start()       i2c_two.Start()
#define I2C_Send_Byte(x)  (i2c_two.Send_Byte(x))
#define I2C_Read_Byte(x)  (i2c_two.Read_Byte(x))
#define I2C_Wait_Ack()    i2c_two.Wait_Ack()
#define I2C_Stop()        i2c_two.Stop()

/** \brief
 *
 * \param channel int
 * \param value int     0 ~ 255
 * \return void
 */
void digipot_set(int channel, int value)
{
    uint8_t addresses[4] = { 0x00, 0x10, 0x60, 0x70};
    uint8_t addr= 0x58; // channel 0-3

    if(channel >= 4) {
    	addr= 0x5A; // channel 4-7
    	channel-= 4;
    }

    I2C_Start();
    I2C_Send_Byte(addr);
    I2C_Wait_Ack();
    I2C_Send_Byte(addresses[channel]);
	I2C_Wait_Ack();
	I2C_Send_Byte((uint8_t)value);
	I2C_Wait_Ack();
    I2C_Stop();
}
