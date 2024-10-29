
#include <stdbool.h>
#include "surge_history.h"
#include "surgemdl_drv.h"
#include "sensor_drv.h"
#include "norflash_drv.h"
#include "systeminf.h"
#include "store_app.h"
#include "led_drv.h"

static SurgeInfo_t g_SurgeInfoCurrent;

/**
***********************************************************
* @brief 保存浪涌次数到系统上电初始化结构体中
* @param
* @return 
***********************************************************
*/

static bool StoreSurgeNum(uint8_t Num)
{
//	if (Num == 0 )
//	{
//		return true;
//	}	
	SysParam_t Param ;  
  ReadSysParam(&Param);
	Param.LightningNum  = Num;	
	if (!WriteSysParam(&Param))
	{
		return false;
	}
	return true;
}


/**
***********************************************************
* @brief 保存浪涌具体信息到外部norflash中
* @param
* @return 
***********************************************************
*/
static void  RecordSurgeParam(SurgeInfo_t * Param)
{
  static uint8_t  Num = 0;
  
  Num = Param->Num - 1;
  //按页写数据，1页对应一次的所有信息记录
  EraseNorflashForWrite( (uint32_t )(Num*4096), 1);
	WriteNorflashData( (uint32_t )(Num*4096), (uint8_t *)Param, sizeof(SurgeInfo_t) );
  
//uint8_t  bufferRead[NUM_MAX] = {0};
//ReadNorflashData ((uint32_t )(Num*4096),bufferRead,NUM_MAX);
///**触发威纶通事件记录用报文***/
//for( uint8_t i=0; i < NUM_MAX; i++)
//{
//  USART_ClearFlag(USART1, USART_FLAG_TC);
//  USART_SendData( USART1 , *( bufferRead + i ) );
//  /*等待发送完成*/
//  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ;
//}
  
}

/**
***********************************************************
* @brief 记录雷击信息数据
* @param
* @return 
***********************************************************
*/
#define Change_factor    0x14      // 1kA 对应 0x14=20  这个值需要特别标定
static SensorData_t sensorData;
void SurgeHistoryProc( void  )
{
  
//  static uint8_t LastNum = 0; 
  GetSensorData(&sensorData);
//  if(g_SurgemdlParam.Num == LastNum )
//  {
//   return ;
//  }
//   LastNum = g_SurgemdlParam.Num; 
  if( GetRefreshflag ())
  {
   g_SurgeInfoCurrent.Num =  sensorData.SurgemdlParam .Num  ;
    
   g_SurgeInfoCurrent.PeakCurrent = sensorData.SurgemdlParam .PeakCurrent   * 1.0f / Change_factor * 10;   
    
   //单独存储次数到eeprom中  
   StoreSurgeNum( g_SurgeInfoCurrent.Num );  
   //获取发生时间    
   GetRtcTime ( &g_SurgeInfoCurrent.RtcTime );   
    //保存浪涌具体信息到外部norflash中  
   RecordSurgeParam( &g_SurgeInfoCurrent) ;    
  }
}

//uint8_t * RecvStr(void )
//{
//  return (uint8_t *)&g_SurgeInfoCurrent;
//}
/**
***********************************************************
* @brief 擦除浪涌历史数据
* @param
* @return 
***********************************************************
*/
#define   BUFFER_SIZE              4096
#define   FLASH_ADDRESS            4096
void EraseSurgeHistoryInfo(void )
{
  while (sensorData.SurgemdlParam .Num--)
  {    
    EraseNorflashForWrite( FLASH_ADDRESS* sensorData.SurgemdlParam .Num, BUFFER_SIZE);
    ToggleLed(0);   
  }
  StoreSurgeNum(0);      //在eeprom中改变浪涌次数
  PresetNum_Surge(0);    //不需要复位
}  
/**
***********************************************************
* @brief 返回具体哪次浪涌的相关信息
* @param pageindex  指针buffer
* @return 
***********************************************************
*/
void GetSurgeHistoryInfo(uint16_t pageindex ,uint8_t * buffer)
{
  ReadNorflashData ((uint32_t )(pageindex*4096),buffer,sizeof (SurgeInfo_t));
}





