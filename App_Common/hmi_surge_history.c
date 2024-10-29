
#include "hmi_common.h"
#include "lcd12864_drv.h"
#include "surgemdl_drv.h"
#include "sensor_drv.h"
#include "rtc_drv.h"
#include "hmi_app2.h"
#include "surge_history.h"

extern  bool g_RepeatSurgeHistoryPage;

#define KEY_LEFT_POS_X          1
#define KEY_LEFT_POS_Y          7
#define KEY_RIGHT_POS_X         4 
#define KEY_RIGHT_POS_Y         7
static void DisplayKeyBar()
{
  LcdDrawChinese (KEY_LEFT_POS_X, KEY_LEFT_POS_Y, (unsigned  char *)"<");
  LcdDrawChinese (KEY_RIGHT_POS_X, KEY_RIGHT_POS_Y, (unsigned  char *)">");
  LcdDrawChinese (12, 7, (unsigned  char *)"清除");
}
#define SURGE_NUM_POS_X          1
#define SURGE_NUM_POS_Y          1
#define TIME_YEAR_POS_X          1
#define TIME_YEAR_POS_Y          3
#define TIME_HOUR_POS_X          1
#define TIME_HOUR_POS_Y          5
#define SURGE_PEAK_POS_X         8
#define SURGE_PEAK_POS_Y         1
#define YEAR_INDEX               4
static void DisplaySurgehistoryInfo()
{
  char strBuf[30];
  char g_Buf[30];

  SensorData_t sensorData;

  LcdArea_t area = {SURGE_PEAK_POS_X*8, 1, (SURGE_PEAK_POS_X+4)*8, 3};
  
  LcdDrawChinese (SURGE_NUM_POS_X + 3, SURGE_NUM_POS_Y, (unsigned  char *)"次");
  LcdDrawChinese (SURGE_PEAK_POS_X + 4, SURGE_PEAK_POS_Y, (unsigned  char *)"kA");
	if (	GetSensorData(&sensorData))
	{
    sensorData.SurgemdlParam.Num = sensorData.SurgemdlParam.Num + (*(GetleftrightIndex ()+1));
    
    if(sensorData.SurgemdlParam.Num == 0 )
    {
      return ;   
    }  
    
    if(sensorData.SurgemdlParam.Num > Save_MAX &&  sensorData.SurgemdlParam.Num == 0xFFFF)
    {
      *(GetleftrightIndex ()+1 )= 0;
     return ;
    } 
    
    static uint16_t  LastNum =0  ;
    
    if( LastNum != sensorData.SurgemdlParam.Num  || g_RepeatSurgeHistoryPage == true)
    {
      g_RepeatSurgeHistoryPage = false ;
      LastNum = sensorData.SurgemdlParam.Num ;
      
      memset(strBuf, 0, sizeof(strBuf));
      sprintf(strBuf, "%02d", sensorData.SurgemdlParam.Num );
      LcdDrawString(SURGE_NUM_POS_X,SURGE_NUM_POS_Y,(unsigned char *)strBuf) ;   
      
      memset(strBuf, 0, sizeof(strBuf));
      /*从norflash对应扇区读取出某一次浪涌的信息*/
      GetSurgeHistoryInfo( sensorData.SurgemdlParam.Num -1 ,(uint8_t *)g_Buf);  

      if( (uint8_t )g_Buf[YEAR_INDEX] == 0xFF) return ;
      sprintf(strBuf, "%02d-%02d-%02d", g_Buf[YEAR_INDEX+1]*256+g_Buf[YEAR_INDEX] , g_Buf[YEAR_INDEX+2] , g_Buf[YEAR_INDEX+3] );
      LcdDrawString(TIME_YEAR_POS_X,TIME_YEAR_POS_Y,(unsigned char *)strBuf) ; 
      
      memset(strBuf, 0, sizeof(strBuf));
      sprintf(strBuf, "%02d:%02d:%02d", g_Buf[YEAR_INDEX+4] , g_Buf[YEAR_INDEX+5] , g_Buf[YEAR_INDEX+6] );
      LcdDrawString(TIME_HOUR_POS_X,TIME_HOUR_POS_Y,(unsigned char *)strBuf) ; 
      
      clear_area (&area);   //防止出现18.0之后1.5显示成1.50，区域刷新
      memset(strBuf, 0, sizeof(strBuf));
      sprintf(strBuf, "%02.1f", (g_Buf[3]*256+g_Buf[2])/10.0f  );
      LcdDrawString(SURGE_PEAK_POS_X,SURGE_PEAK_POS_Y,(unsigned char *)strBuf) ;            
    } 	
	}
	else
	{
    LcdDrawString(SURGE_NUM_POS_X,SURGE_NUM_POS_Y,(unsigned char *)"--") ;    
    LcdDrawString(SURGE_PEAK_POS_X,SURGE_PEAK_POS_Y,(unsigned char *)"--") ;     
    LcdDrawString(TIME_YEAR_POS_X,TIME_YEAR_POS_Y,(unsigned char *)"-----") ;  
    LcdDrawString(TIME_HOUR_POS_X,TIME_HOUR_POS_Y,(unsigned char *)"-----") ;     
	}	

}

void DisplaySurgeHistoryPage(void)
{
  DisplayMenuKey();
	DisplayKeyBar();
	DisplaySurgehistoryInfo();
}
