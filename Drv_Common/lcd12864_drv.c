
/* 针对液晶模块型号：JLX12864G-1353-PC
   串行接口，
   带中文字库IC：JLX-GB2312，竖置横排
   驱动IC是:UC1701X
   晶联讯电子：网址  http://www.jlxlcd.cn;
*/

#include "stm32f10x.h"
#include "delay.h"
#include "lcd12864_drv.h"
#include "picture.h"
#include <stdint.h>

//-----------------背光接口定义---------------- 
#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_1)  //BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_1)

//-----------------LCD驱动IC接口定义---------------- 
#define LCD_CS_Clr()   GPIO_ResetBits(GPIOA,GPIO_Pin_4)  //CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOA,GPIO_Pin_4)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_5)  //RET
#define LCD_RES_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_5)

#define LCD_RS_Clr()   GPIO_ResetBits(GPIOA,GPIO_Pin_6)  //RS
#define LCD_RS_Set()   GPIO_SetBits(GPIOA,GPIO_Pin_6)

#define LCD_SDA_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_7)  //SDA
#define LCD_SDA_Set() GPIO_SetBits(GPIOA,GPIO_Pin_7)

#define LCD_SCK_Clr() GPIO_ResetBits(GPIOC,GPIO_Pin_4)  //SCK
#define LCD_SCK_Set() GPIO_SetBits(GPIOC,GPIO_Pin_4)

//-----------------字库IC接口定义---------------- 
#define ROM_CS_Clr() GPIO_ResetBits(GPIOC,GPIO_Pin_5)  //ROM_CS
#define ROM_CS_Set() GPIO_SetBits(GPIOC,GPIO_Pin_5)

#define ROM_SCK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_0)  //ROM_SCK
#define ROM_SCK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_0)
 		     
#define ROM_IN_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_10)  //ROM_IN
#define ROM_IN_Set() GPIO_SetBits(GPIOB,GPIO_Pin_10)

#define ROM_OUT()	   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) 

static void LCD12864_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 	
	/*开启外设时钟*/
  RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );	
	/*选择引脚*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;	
	/*设置引脚模式为通用推挽输出*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	/*设置引脚速率 */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO_PORT*/
  GPIO_Init (GPIOA, &GPIO_InitStructure );	
  GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
  
  /*开启外设时钟*/
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE );	
	/*选择引脚*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;	
	/*设置引脚模式为通用推挽输出*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	/*设置引脚速率 */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO_PORT*/
  GPIO_Init (GPIOC, &GPIO_InitStructure );	
  GPIO_SetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
  
	/*开启外设时钟*/
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE );	
	/*选择引脚*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_10;	
	/*设置引脚模式为通用推挽输出*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	/*设置引脚速率 */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO_PORT*/
  GPIO_Init ( GPIOB, &GPIO_InitStructure );	
  GPIO_SetBits(GPIOB,GPIO_Pin_0 | GPIO_Pin_10);
  
	/*开启外设时钟*/
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE );	
	/*选择引脚*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	
	/*设置引脚模式为通用浮空输入*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
	/*设置引脚速率 */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO_PORT*/
  GPIO_Init (GPIOB, &GPIO_InitStructure );	
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
static void LCD_WR_DATA(uint8_t dat)
{
  LCD_CS_Clr ();
  LCD_RS_Set ();
  for(uint8_t i=0;i<8;i++)
   {
    LCD_SCK_Clr ();
    if(dat&0x80) LCD_SDA_Set ();
    else LCD_SDA_Clr ();;
    LCD_SCK_Set ();
    dat<<=1;
   }
   LCD_CS_Set ();
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
static void LCD_WR_REG(uint8_t  dat)
{
	LCD_CS_Clr ();
	LCD_RS_Clr ();
  for(uint8_t i=0; i<8; i++)
  {
    LCD_SCK_Clr ();
    if(dat&0x80) LCD_SDA_Set ();
    else LCD_SDA_Clr ();;
    LCD_SCK_Set ();
    DelayNus (2);
    dat<<=1;
  }
  LCD_CS_Set ();
}

/**
***********************************************************
* @brief Lcd硬件初始化
* @param
* @return 
***********************************************************
*/
void Lcd12864DrvInit(void)
{
  LCD12864_GPIO_Config ();	
	ROM_CS_Set ();
	LCD_RES_Clr ();    /*低电平复位*/
	DelayNms (20);
	LCD_RES_Set ();		 /*复位完毕*/
	DelayNms (20);       
	LCD_WR_REG(0xe2);	 /*软复位*/
	DelayNms(5);
	LCD_WR_REG(0x2c);  /*升压步聚1*/
	DelayNms(5);	
	LCD_WR_REG(0x2e);  /*升压步聚2*/
	DelayNms(5);
	LCD_WR_REG(0x2f);  /*升压步聚3*/
	DelayNms(5);
	LCD_WR_REG(0x24);  /*粗调对比度，可设置范围0x20～0x27*/
	LCD_WR_REG(0x81);  /*微调对比度*/
	LCD_WR_REG(0x1d);  /*0x28,微调对比度的值，可设置范围0x00～0x3f*/
	LCD_WR_REG(0xa2);  /*1/9偏压比（bias）*/
	LCD_WR_REG(0xc8);  /*行扫描顺序：从上到下*/
	LCD_WR_REG(0xa0);  /*列扫描顺序：从左到右*/
	LCD_WR_REG(0x40);  /*起始行：第一行开始*/
	LCD_WR_REG(0xaf);  /*开显示*/
  
  
  clear_screen ();
  TurnOnScreen ();
}


static void lcd_address(uint8_t  page,uint8_t  column)
{
	column=column-0x01;
	LCD_WR_REG(0xb0+page-1);            /*设置页地址*/
	LCD_WR_REG(0x10+(column>>4&0x0f));	/*设置列地址的高4位*/
	LCD_WR_REG(column&0x0f);	          /*设置列地址的低4位*/	
}

/*全屏清屏*/
void clear_screen(void )
{
	uint8_t  i,j;
	ROM_CS_Set ();
	for(j=0;j<8;j++)
	{
		lcd_address(j+1,1);
		for(i=0;i<128;i++)
		{
		  	LCD_WR_DATA(0x00);
		}
	}
}


/*区域清屏*/
void clear_area(LcdArea_t *area )
{
	uint8_t  i,j;
	ROM_CS_Set ();
	for(j=area->y1  ;j<area->y2  ;j++)
	{
		lcd_address(j,area->x1);
		for(i=area->x1  ;i<area->x2  ;i++)
		{
		  	LCD_WR_DATA(0x00);
		}
	}
}


void LcdDrawPicture(uint16_t x, uint16_t y, PictureDesc_t *picDesc)
{
	uint8_t  i,j;
  uint8_t *picture;
  picture = (uint8_t *)picDesc->picture;
	for(j=y;j<y +( picDesc->height / 8) + 1;j++)
	{
		lcd_address(j,x*8);
		for (i=0;i<picDesc->width ;i++)
		{	
			LCD_WR_DATA( *picture );					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			picture++;
		}
	}

}

/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
static void display_graphic_16x16(uint8_t page,unsigned char column,unsigned char *dp)
{
	uint8_t i,j;
 	LCD_CS_Clr ();	
	ROM_CS_Set (); 	
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<16;i++)
		{	
			LCD_WR_DATA(*dp);					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			dp++;
		}
	}
	LCD_CS_Set ();
}

/*显示8x16点阵图像、ASCII, 或8x16点阵的自造字符、其他图标*/
static void display_graphic_8x16(uint8_t page,unsigned char column,unsigned char *dp)
{
	uint8_t i,j;
	LCD_CS_Clr ();	
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<8;i++)
		{	
			LCD_WR_DATA(*dp);					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			dp++;
		}
	}
	LCD_CS_Set ();
}

/****送指令到晶联讯字库IC***/
static void send_command_to_ROM( uint8_t  dat )
{
	uint8_t  i;
	for(i=0;i<8;i++ )
	{
		if(dat&0x80)
			ROM_IN_Set ();
		else
			ROM_IN_Clr ();
			dat = dat<<1;
			ROM_SCK_Clr ();
			ROM_SCK_Set ();
	}
}

/****从晶联讯字库IC中取汉字或字符数据（1个字节）***/
static uint8_t  get_data_from_ROM( )
{
	
	uint8_t i;
	uint8_t ret_data=0;
	ROM_SCK_Set ();
	for(i=0;i<8;i++)
	{
		ROM_SCK_Clr ();
		ret_data=ret_data<<1;
		if( ROM_OUT () )
			ret_data=ret_data+1;
		else
			ret_data=ret_data+0;
		ROM_SCK_Set ();
	}
	return(ret_data);
}

/*从相关地址（addrHigh：地址高字节,addrMid：地址中字节,addrLow：地址低字节）中连续读出DataLen个字节的数据到 pBuff的地址*/
/*连续读取*/
static void get_n_bytes_data_from_ROM(unsigned char addrHigh,unsigned char addrMid,unsigned char addrLow,unsigned char *pBuff,unsigned char DataLen )
{
	unsigned char i;
	ROM_CS_Clr (); ;
	LCD_CS_Set ();	
	ROM_SCK_Clr ();
	send_command_to_ROM(0x03);
	send_command_to_ROM(addrHigh);
	send_command_to_ROM(addrMid);
	send_command_to_ROM(addrLow);
	for(i = 0; i < DataLen; i++ )
  {
	 *(pBuff+i) = get_data_from_ROM();
	}
  ROM_CS_Set ();
}

unsigned long fontaddr=0;
static void display_GB2312_string(unsigned char y,unsigned char x,unsigned char *text )
{
	unsigned char i= 0;
	unsigned char addrHigh,addrMid,addrLow ;
	unsigned char fontbuf[32];			
	while((text[i]>0x00))
	{
		if(((text[i]>=0xb0) &&(text[i]<=0xf7))&&(text[i+1]>=0xa1))
		{						
			/*GB2312汉字在晶联讯字库IC中的地址由以下公式来计算：*/
			/*Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0*/
			/*由于担心8位单片机有乘法溢出问题，所以分三部取地址*/
			fontaddr = (text[i]- 0xb0)*94; 
			fontaddr += (text[i+1]-0xa1)+846;
			fontaddr = (unsigned long)(fontaddr*32);
			
			addrHigh = (fontaddr&0xff0000)>>16;  /*地址的高8位,共24位*/
			addrMid = (fontaddr&0xff00)>>8;      /*地址的中8位,共24位*/
			addrLow = fontaddr&0xff;	           /*地址的低8位,共24位*/
			get_n_bytes_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,32 );/*取32个字节的数据，存到"fontbuf[32]"*/
			display_graphic_16x16(y,x,fontbuf);  /*显示到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=2;
			x+=16;
		}
		else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1))
		{						
			/*15x16点GB2312标准点阵字库在晶联讯字库IC中的地址由以下公式来计算：*/
			/*Address = ((MSB - 0xa1) * 94 + (LSB - 0xA1))*32+ BaseAdd;BaseAdd=0*/
			/*由于担心8位单片机有乘法溢出问题，所以分三部取地址*/
			fontaddr = (text[i]- 0xa1)*94; 
			fontaddr += (text[i+1]-0xa1);
			fontaddr = (unsigned long)(fontaddr*32);
			
			addrHigh = (fontaddr&0xff0000)>>16;  /*地址的高8位,共24位*/
			addrMid = (fontaddr&0xff00)>>8;      /*地址的中8位,共24位*/
			addrLow = fontaddr&0xff;	           /*地址的低8位,共24位*/
			get_n_bytes_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,32 );/*取32个字节的数据，存到"fontbuf[32]"*/
			display_graphic_16x16(y,x,fontbuf);  /*显示LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=2;
			x+=16;
		}
		else if((text[i]>=0x20) &&(text[i]<=0x7e))	
		{		
     /*8x16点ASCII粗体的字符在晶联讯字库IC中的地址由以下公式来计算：*/      
			unsigned char fontbuf[16];			
			fontaddr = (text[i]- 0x20);
			fontaddr = (unsigned long)(fontaddr*16);
			fontaddr = (unsigned long)(fontaddr+0x3cf80);			
			addrHigh = (fontaddr&0xff0000)>>16;
			addrMid = (fontaddr&0xff00)>>8;
			addrLow = fontaddr&0xff;
			get_n_bytes_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,16 ); /*取16个字节的数据，存到"fontbuf[32]"*/			
			display_graphic_8x16(y,x,fontbuf);       /*显示8x16的ASCII字到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=1;
			x+=8;
		}
		else
    {
			i++;
    }      
	}
}

///******************************************************************************
//函数说明：显示数字
//入口数据：m底数，n指数
//返回值：  无
//******************************************************************************/
//u32 mypow(u8 m,u8 n)
//{
//	u32 result=1;	 
//	while(n--)result*=m;
//	return result;
//}

///******************************************************************************
//函数说明：显示整数
//入口数据：
//返回值：  无
//******************************************************************************/
//void LCD_ShowIntNum(unsigned char x,unsigned char y,u16 num,uint8_t len,uint8_t sizey)
//{ 
//  uint8_t  temp_Int;  
//	u8 t=0;
//	u8 enshow=0;
//	u8 sizex=sizey/2;
//	for(t=0;t<len;t++)
//	{
//		temp_Int = (num/mypow(10,len-t-1))%10 ;
//		if(enshow==0&&t<(len-1))
//		{
//			if(temp_Int==0)
//			{
//        display_GB2312_string(y,x*sizex+t*sizex,(unsigned char *)(&temp_Int) );
//				continue;
//			}else enshow=1; 
//		 	 
//		}
//    temp_Int += 0x30;
//	 	display_GB2312_string(y,x*sizex+t*sizex,(unsigned char *)(&temp_Int) );
//	}
//} 

/******************************************************************************
函数说明：显示一位浮点数
入口数据：
返回值：  无
******************************************************************************/
//void LCD_ShowFloatNum(unsigned char x,unsigned char y,float num,uint8_t len,uint8_t sizey)
//{         	
//  uint8_t  temp_Float;
//	u8 t=0,sizex=0;
//	u16 num1=0;
//	sizex = sizey/2;
//	num1=num*10;
//	for(t=0;t<len;t++)
//	{
//		temp_Float=(num1/mypow(10,len-t-1))%10;
//		if(t==(len-1))
//		{
//			display_GB2312_string(y,x*sizex+(len-1)*sizex,(unsigned char *)".");
//			t++;
//			len+=1;
//		}
//	 	temp_Float += 0x30;
//	 	display_GB2312_string(y,x*sizex+t*sizex,(unsigned char *)(&temp_Float) );
//	}
//}

/******************************************************************************
函数说明：显示汉字
入口数据：
返回值：  无
******************************************************************************/

void LcdDrawChinese(unsigned char x,unsigned char y,unsigned  char * string)
{
  u8 sizex;
  sizex = 16/2;
  display_GB2312_string(y,x*sizex,string ); 
}

/******************************************************************************
函数说明：显示字符串
入口数据：
返回值：  无
******************************************************************************/

void LcdDrawString(unsigned char x,unsigned char y,unsigned  char * string)
{
  u8 sizex;
  sizex = 16/2;
  display_GB2312_string(y,x*sizex,string ); 
}

void TurnOnScreen(void )
{
  LCD_BLK_Set();
} 

void TurnOffScreen(void )
{
  LCD_BLK_Clr();
}


