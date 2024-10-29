#include <stdint.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "sensor_drv.h"
#include "ntc_drv.h"
#include "surge_history.h"
#include "surgemdl_drv.h"


/**
***********************************************************
* @brief 传感器驱动初始化
* @param
* @return 
***********************************************************
*/
void SensorDrvInit(void)
{
  /***传感器初始化****/
  TempDrvInit (); 
  SurgeMdlDrvInit ();
}

/**
***********************************************************
* @brief 触发驱动转换传感器数据
* @param
* @return 
***********************************************************
*/
void SensorDrvProc(void)
{
	TempSensorProc(); 
  SurgemdlDataPro ();
  SurgeHistoryProc();
}

/**
***********************************************************
* @brief 获取传感器数据
* @param sensorData,输出，传感器数据回写地址
* @return 
***********************************************************
*/
bool  GetSensorData(SensorData_t *sensorData)
{
  float Temp;
  SurgemdlParam_t SurgemdlPara;
  
  GetTempData(&Temp);
  
  sensorData->temp = Temp;
  
  if( GetSurgemdlData(&SurgemdlPara)) 
  {
    sensorData->SurgemdlParam .Num  = SurgemdlPara.Num ;
    sensorData->SurgemdlParam.PeakCurrent   = SurgemdlPara.PeakCurrent  ;
    return true ;
  }
  else 
  {
    return false ;
  }  
}
