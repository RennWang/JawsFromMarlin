#include "SoftI2C.h"
#include "delay.h"

#define SDA_IN()    SET_INPUT(_sda)
#define SDA_OUT()   SET_OUTPUT(_sda)

#define SDA_HIGH    WRITE(_sda, true)
#define SDA_LOW     WRITE(_sda, false)
#define SCL_HIGH    WRITE(_scl, true)
#define SCL_LOW     WRITE(_scl, false)

#define READ_SDA    READ(_sda)

SoftI2C::SoftI2C(PinName sda, PinName scl)
{
    _sda = sda;
    _scl = scl;
    init();
}

void SoftI2C::init()
{
    SET_OUTPUT(_sda);
    SET_OUTPUT(_scl);
    SDA_HIGH;
    SCL_HIGH;
}

void SoftI2C::Start(void)
{
    SDA_OUT();
	SDA_HIGH;
	SCL_HIGH;
	HAL_Delay_us(4);
 	SDA_LOW;
	HAL_Delay_us(4);
	SCL_LOW;
}

void SoftI2C::Stop(void)
{
    SDA_OUT();
	SCL_LOW;
	SDA_LOW;
 	HAL_Delay_us(4);
	SCL_HIGH;
	SDA_HIGH;
	HAL_Delay_us(4);
}

uint8_t SoftI2C::Wait_Ack(void)
{
    uint8_t ucErrTime=0;
	SDA_IN();
	SDA_HIGH;
	HAL_Delay_us(1);
	SCL_HIGH;
	HAL_Delay_us(1);

	while(READ_SDA){
		ucErrTime++;
		if(ucErrTime>250){
			Stop();
			return 1;
		}
	}
	SCL_LOW;
	return 0;
}

void SoftI2C::Ack(void)
{
	SCL_LOW;
	SDA_OUT();
	SDA_LOW;
	HAL_Delay_us(2);
	SCL_HIGH;
	HAL_Delay_us(2);
	SCL_LOW;
}

void SoftI2C::NAck(void)
{
	SCL_LOW;
	SDA_OUT();
	SDA_HIGH;
	HAL_Delay_us(2);
	SCL_HIGH;
	HAL_Delay_us(2);
	SCL_LOW;
}

void SoftI2C::Send_Byte(uint8_t txd)
{
    uint8_t t;
	SDA_OUT();
    SCL_LOW;
    for(t=0;t<8;t++)
    {
        (txd&0x80)>0?SDA_HIGH:SDA_LOW;
        txd<<=1;
		HAL_Delay_us(2);
		SCL_HIGH;
		HAL_Delay_us(2);
		SCL_LOW;
		HAL_Delay_us(2);
    }
}

uint8_t SoftI2C::Read_Byte(uint8_t ack)
{
    uint8_t i,receive=0;
	SDA_IN();
    for(i=0;i<8;i++ ){
        SCL_LOW;
        HAL_Delay_us(2);
		SCL_HIGH;
        receive<<=1;
        if(READ_SDA)receive++;
		HAL_Delay_us(1);
    }
    if (!ack)
        NAck();
    else
        Ack();
    return receive;
}

SoftI2C i2c_one(PB_11,PB_10);
SoftI2C i2c_two(PD_13,PD_12);
