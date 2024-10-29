#include "hmi_common.h"
#include "lcd12864_drv.h"
#include "invalid_app.h"
#include "hmi_app2.h"

#define KEY_LEFT_POS_X          1
#define KEY_LEFT_POS_Y          7
#define KEY_RIGHT_POS_X         5 
#define KEY_RIGHT_POS_Y         7
static void DisplayKeyBar()
{
  LcdDrawChinese (KEY_LEFT_POS_X, KEY_LEFT_POS_Y, (unsigned  char *)"<");
  LcdDrawChinese (KEY_RIGHT_POS_X, KEY_RIGHT_POS_Y, (unsigned  char *)">");
  
}

#define TIME_YEAR_POS_X          1
#define TIME_YEAR_POS_Y          3
#define TIME_HOUR_POS_X          1
#define TIME_HOUR_POS_Y          5
#define YEAR_INDEX               0
static void DisplayInvalidTimeInfo(void )
{
  char strBuf[30];
  char g_Buf[30];
  int8_t Offset = *(GetleftrightIndex ()+2);
  
  LcdDrawChinese(TIME_YEAR_POS_X,TIME_YEAR_POS_Y-2,(unsigned char *)"第  路断开时刻");
  sprintf(strBuf, "%02d", Offset+1 );
  LcdDrawString(TIME_YEAR_POS_X+2,TIME_YEAR_POS_Y-2,(unsigned char *)strBuf) ; 
  
  memset(strBuf, 0, sizeof(strBuf));
  GetInvalidTime( Invalid_START_Sector + Offset ,(uint8_t *)g_Buf);     
  sprintf(strBuf, "%02d-%02d-%02d", g_Buf[YEAR_INDEX+1]*256+g_Buf[YEAR_INDEX] , g_Buf[YEAR_INDEX+2] , g_Buf[YEAR_INDEX+3] );
  LcdDrawString(TIME_YEAR_POS_X,TIME_YEAR_POS_Y,(unsigned char *)strBuf) ; 
  memset(strBuf, 0, sizeof(strBuf));
  sprintf(strBuf, "%02d:%02d:%02d", g_Buf[YEAR_INDEX+4] , g_Buf[YEAR_INDEX+5] , g_Buf[YEAR_INDEX+6] );
  LcdDrawString(TIME_HOUR_POS_X,TIME_HOUR_POS_Y,(unsigned char *)strBuf) ; 
  
}

void DisplayInvalidTimePage(void)
{
  DisplayMenuKey();
	DisplayKeyBar();
	DisplayInvalidTimeInfo();
}
