#ifndef _LCD12864_DRV_H_
#define _LCD12864_DRV_H_

#include <stdint.h>
#include "picture.h"

/** Represents an area of the screen.*/
typedef struct
{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
} LcdArea_t;

/**
***********************************************************
* @brief Lcd硬件初始化
* @param
* @return 
***********************************************************
*/
void Lcd12864DrvInit(void);

/*全屏清屏*/
void clear_screen(void );
/*区域清屏*/
void clear_area(LcdArea_t *area );

//void display_GB2312_string(unsigned char y,unsigned char x,unsigned char *text);
//void LCD_ShowIntNum(unsigned char x,unsigned char y,uint16_t num,uint8_t len,uint8_t sizey);
//void LCD_ShowFloatNum(unsigned char x,unsigned char y,float num,uint8_t len,uint8_t sizey);

void LcdDrawPicture(uint16_t x, uint16_t y, PictureDesc_t *picDesc);
void LcdDrawChinese(unsigned char x,unsigned char y,unsigned  char * string);
void LcdDrawString(unsigned char x,unsigned char y,unsigned  char * string);

void TurnOnScreen(void );
void TurnOffScreen(void );

#endif

