
/*
 * ���������Ӧ�ó���
 */
#include "stm32f10x.h"
#include "sensor_drv.h"
#include "eeprom_drv.h"
#include "iwdg_drv.h"

void Sensor_Task(void )
{
  SensorDrvProc();
  FeedDog ();
}
