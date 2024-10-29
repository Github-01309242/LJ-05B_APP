
#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "systick_drv.h"
#include "delay.h"
#include "led_drv.h"
#include "iwdg_drv.h"
#include "key_drv.h"
#include "rtc_drv.h"
#include "switch_drv.h"
#include "sensor_drv.h"
#include "eeprom_drv.h"
#include "norflash_drv.h"
#include "lcd12864_drv.h"
#include "surgemdl_drv.h"
#include "iap_drv.h"
#include "RTT_Debug.h"
#include "hmi_app2.h"
#include "sensor_app.h"
#include "store_app.h"
#include "modbus_app.h"
#include "invalid_app.h"


typedef struct
{
	uint8_t run;                // ���ȱ�־��1�����ȣ�0������
	uint16_t timCount;          // ʱ��Ƭ����ֵ
	uint16_t timRload;          // ʱ��Ƭ����ֵ
	void (*pTaskFuncCb)(void);  // ����ָ���������������ҵ����ģ�麯����ַ
} TaskComps_t;

static TaskComps_t g_taskComps[] = 
{
  {0, 5,    5,     HmiTask  },
  {0, 10,   10,    ModbusTask  },
  {0, 500,  500,   Sensor_Task  },
  {0, 500,  500,   Invalid_Task   },
	/* ���ҵ����ģ�� */
};

#define TASK_NUM_MAX   (sizeof(g_taskComps) / sizeof(g_taskComps[0]))

static void TaskHandler(void)
{
	for (uint8_t i = 0; i < TASK_NUM_MAX; i++)
	{
		if (g_taskComps[i].run)                // �ж�ʱ��Ƭ��־
		{
			g_taskComps[i].run = 0;              // ��־����
			g_taskComps[i].pTaskFuncCb();        // ִ�е���ҵ����ģ��
		}
	}
}

/**
***********************************************************
* @brief �ڶ�ʱ���жϷ������б���ӵ��ã�����ʱ��Ƭ��ǣ�
         ��Ҫ��ʱ��1ms����1���ж�
* @param
* @return 
***********************************************************
*/
static void TaskScheduleCb(void)
{
	for (uint8_t i = 0; i < TASK_NUM_MAX; i++)
	{
		if (g_taskComps[i].timCount)
		{
			g_taskComps[i].timCount--;
			if (g_taskComps[i].timCount == 0)
			{
				g_taskComps[i].run = 1;
				g_taskComps[i].timCount = g_taskComps[i].timRload;
			}
		}
	}
}


static void DrvInit(void)
{
  DelayInit ();
  
  LedDrvInit ();  
  KeyDrvInit ();
  Switchinput_DrvInit ();
  RtcDrvInit ();
  EepromDrvInit ();
  NorflashDrvInit ();
  DelayNms (2000);   //�ȴ���ӿ�������ϵ��ȶ�
  Lcd12864DrvInit ();
  
  IwdgDrvInit();
  SensorDrvInit ();
  
}

static void AppInit(void)
{
	TaskScheduleCbReg(TaskScheduleCb);  
  InitSysParam ();
  ModbusAppInit();
}


int main(void)
{
  InitIrqAfterBoot();
  SysTick_Init();      
  DrvInit();    
  AppInit(); 

  while(1)
  {		
    TaskHandler(); 
  }
}


/*********************************************END OF FILE**********************/
