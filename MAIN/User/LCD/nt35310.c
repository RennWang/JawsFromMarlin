#include "LCDMiddle.h"

LCD_DEV lcd_dev;

static void nt35310_Init(void);
static uint16_t nt35310_ReadID(void);


void LCD_Init(void)
{
    lcd_dev.id = nt35310_ReadID();
    nt35310_Init();

    lcd_dev.dir    = 1;
    lcd_dev.width  = 480;
    lcd_dev.height = 320;

    LCD_SetScanDir();

    LCD_SetBackLight();

    LCD_Clear(RED);
}

uint16_t LCD_ReadID(void)
{
    return nt35310_ReadID();
}

void LCD_Clear(uint16_t color)
{
	uint32_t index = 0;
	uint32_t totalpoint = lcd_dev.width;
	totalpoint *= lcd_dev.height;
	LCD_SetCursor(0, 0);
	LCD_IO_WriteReg(0x2C);
	for(index = 0; index < totalpoint; index++){
	    LCD_IO_WriteData(color);
	}
}

void LCD_SetBackLight(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}

void LCD_SetScanDir(void)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	if(lcd_dev.dir==1)
	{
        regval|=(1<<7)|(0<<6)|(1<<5);
	}
	else
	{
	    regval|=(0<<7)|(0<<6)|(0<<5);
	}

    dirreg=0X36;
    LCD_IO_WriteReg(dirreg);
    LCD_IO_WriteData(regval);

    LCD_IO_WriteReg(0x2A);
    LCD_IO_WriteData(0);
    LCD_IO_WriteData(0);
    LCD_IO_WriteData((lcd_dev.width-1)>>8);
    LCD_IO_WriteData((lcd_dev.width-1)&0XFF);
    LCD_IO_WriteReg(0x2B);
    LCD_IO_WriteData(0);
    LCD_IO_WriteData(0);
    LCD_IO_WriteData((lcd_dev.height-1)>>8);
    LCD_IO_WriteData((lcd_dev.height-1)&0XFF);
}

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    LCD_IO_WriteReg(0x2A);
    LCD_IO_WriteData(Xpos>>8);
    LCD_IO_WriteData(Xpos&0xFF);

    LCD_IO_WriteReg(0x2B);
    LCD_IO_WriteData(Ypos>>8);
    LCD_IO_WriteData(Ypos&0xFF);
}

void LCD_DrawPoint(uint16_t Xpos, uint16_t Ypos, uint16_t Color)
{
    LCD_IO_WriteReg(0x2A);
    LCD_IO_WriteData(Xpos>>8);
    LCD_IO_WriteData(Xpos&0xFF);

    LCD_IO_WriteReg(0x2B);
    LCD_IO_WriteData(Ypos>>8);
    LCD_IO_WriteData(Ypos&0xFF);

    LCD_IO_WriteReg(0x2C);
    LCD_IO_WriteData(Color);
}

uint16_t LCD_ReadPoint(uint16_t Xpos, uint16_t Ypos)
{
 	uint16_t r = 0, g = 0, b = 0;

	if(Xpos >= lcd_dev.width || Ypos >= lcd_dev.height)
	    return 0;

	LCD_SetCursor(Xpos, Ypos);

    LCD_IO_WriteReg(0x2E);

	r = LCD_IO_ReadData();					//dummy Read

 	r = LCD_IO_ReadData();

    b = LCD_IO_ReadData();

    g = r & 0xFF;
    g <<= 8;

	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
}

void LCD_DisplayOn(void)
{
	LCD_IO_WriteReg(0x29);
}

void LCD_DisplayOff(void)
{
	LCD_IO_WriteReg(0x28);
}

static void nt35310_Init(void)
{
    LCD_IO_Init();

    LCD_IO_WriteReg(0xED);
    LCD_IO_WriteData(0x01);
    LCD_IO_WriteData(0xFE);

    LCD_IO_WriteReg(0xEE);
    LCD_IO_WriteData(0xDE);
    LCD_IO_WriteData(0x21);

    LCD_IO_WriteReg(0xF1);
    LCD_IO_WriteData(0x01);

    LCD_IO_WriteReg(0xDF);
    LCD_IO_WriteData(0x10);

    LCD_IO_WriteReg(0xC4);
    LCD_IO_WriteData(0x8F);

    LCD_IO_WriteReg(0xC6);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xE2);
    LCD_IO_WriteData(0xE2);
    LCD_IO_WriteData(0xE2);

    LCD_IO_WriteReg(0xBF);
    LCD_IO_WriteData(0xAA);

    LCD_IO_WriteReg(0xB0);
    LCD_IO_WriteData(0x0D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x0D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x11);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x19);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x21);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x5D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x5D);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB1);
    LCD_IO_WriteData(0x80);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x8B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x96);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x02);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x03);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB4);
    LCD_IO_WriteData(0x8B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x96);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA1);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB5);
    LCD_IO_WriteData(0x02);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x03);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x04);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB6);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3F);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x5E);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x64);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x8C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xAC);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDC);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x70);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x90);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xEB);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDC);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xB8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xBA);
    LCD_IO_WriteData(0x24);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC1);
    LCD_IO_WriteData(0x20);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x54);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xFF);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC2);
    LCD_IO_WriteData(0x0A);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x04);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC3);
    LCD_IO_WriteData(0x3C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3A);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x39);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x37);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x36);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x32);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2F);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x29);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x26);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x24);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x24);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x23);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x36);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x32);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2F);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x29);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x26);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x24);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x24);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x23);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC4);
    LCD_IO_WriteData(0x62);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x05);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x84);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF0);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x18);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA4);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x18);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x50);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x0C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x17);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x95);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xE6);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC5);
    LCD_IO_WriteData(0x32);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x65);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x76);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC6);
    LCD_IO_WriteData(0x20);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x17);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x01);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xC9);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE0);
    LCD_IO_WriteData(0x16);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x1C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x21);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x36);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x46);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x52);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x64);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x7A);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x8B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB9);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC4);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xCA);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD9);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xE0);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE1);
    LCD_IO_WriteData(0x16);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x1C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x22);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x36);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x45);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x52);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x64);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x7A);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x8B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB9);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC4);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xCA);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xE0);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE2);
    LCD_IO_WriteData(0x05);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x0B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x1B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x34);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x4F);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x61);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x79);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x97);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA6);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD1);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD6);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDD);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE3);
    LCD_IO_WriteData(0x05);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x1C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x33);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x50);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x62);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x78);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x97);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA6);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC7);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD1);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD5);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDD);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE4);
    LCD_IO_WriteData(0x01);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x01);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x02);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x2A);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x4B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x5D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x74);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x84);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x93);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xBE);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC4);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xCD);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDD);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE5);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x02);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x29);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x3C);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x4B);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x5D);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x74);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x84);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x93);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xA2);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xB3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xBE);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xC4);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xCD);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xD3);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xDC);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE6);
    LCD_IO_WriteData(0x11);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x34);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x56);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x76);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x77);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x66);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xBB);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x66);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x45);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x43);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE7);
    LCD_IO_WriteData(0x32);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x76);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x66);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x67);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x67);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x87);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xBB);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x77);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x56);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x23);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x33);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x45);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE8);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x87);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x77);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x66);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x88);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xAA);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0xBB);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x99);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x66);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x44);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x55);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xE9);
    LCD_IO_WriteData(0xAA);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0x00);
    LCD_IO_WriteData(0xAA);

    LCD_IO_WriteReg(0xCF);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xF0);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x50);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xF3);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0xF9);
    LCD_IO_WriteData(0x06);
    LCD_IO_WriteData(0x10);
    LCD_IO_WriteData(0x29);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0x3A);
    LCD_IO_WriteData(0x55);	//66

    LCD_IO_WriteReg(0x11);
    HAL_Delay(100);
    LCD_IO_WriteReg(0x29);
    LCD_IO_WriteReg(0x35);
    LCD_IO_WriteData(0x00);

    LCD_IO_WriteReg(0x51);
    LCD_IO_WriteData(0xFF);
    LCD_IO_WriteReg(0x53);
    LCD_IO_WriteData(0x2C);
    LCD_IO_WriteReg(0x55);
    LCD_IO_WriteData(0x82);
    LCD_IO_WriteReg(0x2c);
}

static uint16_t nt35310_ReadID(void)
{
    uint16_t lcdid;
    LCD_IO_Init();

    LCD_IO_WriteReg(0xD4);

    LCD_IO_ReadData();	//dummy read
    LCD_IO_ReadData();
    lcdid = LCD_IO_ReadData();
    lcdid <<= 8;
    lcdid |= LCD_IO_ReadData();

    return lcdid;
}
