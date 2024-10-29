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
* @brief ������������ʼ��
* @param
* @return 
***********************************************************
*/
void SensorDrvInit(void);

/**
***********************************************************
* @brief ��ȡ����������
* @param sensorData,��������������ݻ�д��ַ
* @return bool����
***********************************************************
*/
bool GetSensorData(SensorData_t *sensorData);

/**
***********************************************************
* @brief ��������ת������������
* @param
* @return 
***********************************************************
*/
void SensorDrvProc(void);

#endif
