#ifndef _SENSOR_DRV_H_
#define _SENSOR_DRV_H_

#include <stdint.h>
#include "rtc_drv.h"
#include "surgemdl_drv.h"


#define Save_MAX  200

typedef struct
{
	float temp;
	uint8_t humi;
  float Voltage_A;
  SurgemdlParam_t SurgemdlParam;
} SensorData_t;

/**
***********************************************************
* @brief 传感器驱动初始化
* @param
* @return 
***********************************************************
*/
void SensorDrvInit(void);

/**
***********************************************************
* @brief 获取传感器数据
* @param sensorData,输出，传感器数据回写地址
* @return bool类型
***********************************************************
*/
bool GetSensorData(SensorData_t *sensorData);

/**
***********************************************************
* @brief 触发驱动转换传感器数据
* @param
* @return 
***********************************************************
*/
void SensorDrvProc(void);

#endif
