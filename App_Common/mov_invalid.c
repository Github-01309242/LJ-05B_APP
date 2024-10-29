/*
 * 压敏电阻失效比例计算应用程序
 */
#include <stdbool.h>
#include <stdint.h>
#include "sensor_drv.h"
#include "systick_drv.h"
#include "switch_drv.h"
#include "mov_invalid.h"
#include "RTT_Debug.h"

#define Over_Temp_threshold  50.0f
#define First_Period_Time   5
#define Second_Period_Time  10
#define Three_Period_Time   15

static uint32_t GetMOVFail_PCT( void )
{
  static  uint64_t  StartTime = 0;
  static  uint64_t  EndTime = 0;
  static  uint64_t  SpaceTime = 0;
  static bool  g_movIsOk = true ;  
  
  uint8_t* STA = GetSwitchinputStatus();
  //获取到IO输入状态已经变化，直接返回0%
  if( STA[0] == 0x01 )     
  {
  return BROKEN ;
  }
  else if( STA[0] == 0x00 )
  {
    SensorData_t sensorData; 
    GetSensorData(&sensorData);
    if( sensorData.temp   < Over_Temp_threshold )
    {
      g_movIsOk = true ;
      return EXCELLENT; 
    }
    if( sensorData.temp  > Over_Temp_threshold  && g_movIsOk)  
    {
      g_movIsOk = false ;
      StartTime = GetSysRunTime ();
    }      
    EndTime = GetSysRunTime ();    
    SpaceTime = (EndTime - StartTime) / 1000 / 60 ;     //ms累计转换成min    
    
    if(SpaceTime < First_Period_Time )
    {
      return EXCELLENT;
    }
    else if ( SpaceTime > First_Period_Time  && SpaceTime < Second_Period_Time )
    {
      return MILD;
    }
    else if ( SpaceTime > Second_Period_Time && SpaceTime < Three_Period_Time )
    {
      return MODERATE;
    }
    else if ( SpaceTime > Three_Period_Time )
    {    
      return SEVERE;
    }
  }
  return OTHERINDEX;
}

static MovLevel_t  g_MovLevel = OTHERLEVEL;

static void CalMovLevel(uint8_t percentage)
{ 
  if( percentage == EXCELLENT )    
  {
   g_MovLevel = MOV_LEVEL_GRID_0;
  }
  else if ( percentage == MILD )  
  {
   g_MovLevel = MOV_LEVEL_GRID_1;
  }
  else if ( percentage == MODERATE )  
  {
   g_MovLevel = MOV_LEVEL_GRID_2;
  }
  else if ( percentage == SEVERE  )  
  {
   g_MovLevel = MOV_LEVEL_GRID_3;
  }  
  else if ( percentage == BROKEN  )  
  {
   g_MovLevel = MOV_LEVEL_GRID_4;     //已经失效
  }
}

void MovInvalidProc(void )
{ 
  uint8_t g_PCT ;  
  g_PCT = GetMOVFail_PCT() ;
  CalMovLevel(g_PCT);
}

MovLevel_t GetMovLevel(void)
{
	return g_MovLevel;
}








  



