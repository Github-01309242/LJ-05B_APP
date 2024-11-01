
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
	uint8_t run;                // 调度标志，1：调度，0：挂起
	uint16_t timCount;          // 时间片计数值
	uint16_t timRload;          // 时间片重载值
	void (*pTaskFuncCb)(void);  // 函数指针变量，用来保存业务功能模块函数地址
} TaskComps_t;

static TaskComps_t g_taskComps[] = 
{
  {0, 5,    5,     HmiTask  },
  {0, 10,   10,    ModbusTask  },
  {0, 500,  500,   Sensor_Task  },
  {0, 500,  500,   Invalid_Task   },
	/* 添加业务功能模块 */
};

#define TASK_NUM_MAX   (sizeof(g_taskComps) / sizeof(g_taskComps[0]))

static void TaskHandler(void)
{
	for (uint8_t i = 0; i < TASK_NUM_MAX; i++)
	{
		if (g_taskComps[i].run)                // 判断时间片标志
		{
			g_taskComps[i].run = 0;              // 标志清零
			g_taskComps[i].pTaskFuncCb();        // 执行调度业务功能模块
		}
	}
}

/**
***********************************************************
* @brief 在定时器中断服务函数中被间接调用，设置时间片标记，
         需要定时器1ms产生1次中断
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
  DelayNms (2000);   //等待浪涌传感器上电稳定
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
