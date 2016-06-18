#ifndef LCDMIDDLE_H_INCLUDED
#define LCDMIDDLE_H_INCLUDED

#include "main.h"

typedef struct
{
	uint16_t width;
	uint16_t height;
	uint16_t id;
	uint16_t dir;
} LCD_DEV;

LCD_DEV lcd_dev;


#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430

#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458

#define LIGHTGREEN     	 0X841F
//#define LIGHTGRAY        0XEF5B
#define LGRAY 			 0XC618

#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12

void        LCD_Init(void);
uint16_t    LCD_ReadID(void);
void        LCD_Clear(uint16_t color);
void        LCD_SetBackLight(void);
void        LCD_SetScanDir(void);

void        LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void        LCD_DrawPoint(uint16_t Xpos, uint16_t Ypos, uint16_t Color);
uint16_t    LCD_ReadPoint(uint16_t Xpos, uint16_t Ypos);
void        LCD_DisplayOn(void);
void        LCD_DisplayOff(void);

void LCD_IO_Init(void);

void LCD_IO_WriteData(uint16_t RegValue);

void LCD_IO_WriteReg(uint8_t Reg);

uint16_t LCD_IO_ReadData(void);

#endif /* LCDMIDDLE_H_INCLUDED */
