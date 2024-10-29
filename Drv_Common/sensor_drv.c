#include <stdint.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "sensor_drv.h"
#include "ntc_drv.h"
#include "surge_history.h"
#include "surgemdl_drv.h"


/**
***********************************************************
* @brief ������������ʼ��
* @param
* @return 
***********************************************************
*/
void SensorDrvInit(void)
{
  /***��������ʼ��****/
  TempDrvInit (); 
  SurgeMdlDrvInit ();
}

/**
***********************************************************
* @brief ��������ת������������
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
* @brief ��ȡ����������
* @param sensorData,��������������ݻ�д��ַ
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
