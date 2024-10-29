
#include "lcd12864_drv.h"
#include "hmi_app2.h"
#include "hmi_common.h"
#include "store_app.h"

extern PictureDesc_t g_Image_ConfigPicDesc;

#define ADDR_POS_X          1
#define ADDR_POS_Y          3
#define BAUD_POS_X          1
#define BAUD_POS_Y          3
#define OTHER_POS_X         1
#define OTHER_POS_Y         5

void DisplayRS485StatePage(void )
{
  DisplayMenuKey();
  
  char strBuf[10];

  LcdDrawChinese (1, 1, (unsigned  char *)"串口参数页面");
//  LcdDrawChinese (ADDR_POS_X, ADDR_POS_Y, (unsigned  char *)"地址= "); 
  memset(strBuf, 0, sizeof(strBuf));
  sprintf(strBuf, "%02d", GetModbusAddrData() );
  LcdDrawString (ADDR_POS_X+0, ADDR_POS_Y, (unsigned char *)strBuf) ;   
  
  LcdDrawChinese (3, 3, (unsigned  char *)",");
  
//  LcdDrawChinese (BAUD_POS_X, BAUD_POS_Y, (unsigned  char *)"波特率= "); 
  memset(strBuf, 0, sizeof(strBuf));
  sprintf(strBuf, "%02d", GetModbusBaudData() );
  LcdDrawString (BAUD_POS_X+4, BAUD_POS_Y, (unsigned char *)strBuf) ;  

  LcdDrawChinese (OTHER_POS_X, OTHER_POS_Y, (unsigned  char *)"双击K3,配置界面 ");    
  
}

#define KEY_LEFT_POS_X          1
#define KEY_LEFT_POS_Y          7
#define KEY_RIGHT_POS_X         5 
#define KEY_RIGHT_POS_Y         7
static void DisplayKeyBar(void )
{
  LcdDrawChinese (1, 1, (unsigned  char *)"参数配置页面");
  LcdDrawChinese (KEY_LEFT_POS_X, KEY_LEFT_POS_Y, (unsigned  char *)"<");
  LcdDrawChinese (KEY_RIGHT_POS_X, KEY_RIGHT_POS_Y, (unsigned  char *)">");  
  LcdDrawChinese (12, 5, (unsigned  char *)"长按");
  LcdDrawChinese (12, 7, (unsigned  char *)"确认"); 
}

static void DisplayAddrConfig(void )
{
  char strBuf[10];
  int8_t Offset = *(GetleftrightIndex() + 4 );
  LcdDrawChinese (ADDR_POS_X, ADDR_POS_Y, (unsigned  char *)"地址= "); 
  memset(strBuf, 0, sizeof(strBuf));
  sprintf(strBuf, "%02d", Offset );
  LcdDrawString(7,3,(unsigned char *)strBuf) ;   
  
  static int8_t lastOffset = 0;
  if( lastOffset != Offset)
  {  
    lastOffset = Offset;
    SetModbusAddrParam(Offset);
  }
}

void DisplayRS485ConfigPage(void )
{

 DisplayKeyBar();
 DisplayAddrConfig(); 
}
