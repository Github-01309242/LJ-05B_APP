
#include "stm32f10x.h"
#include <stdint.h>
#include <stdlib.h>

static volatile uint64_t g_sysRunTime = 0;

static void (*g_pTaskScheduleFunc)(void);          // 函数指针，保存任务调度的函数地址

/**
***********************************************************
* @brief 注册任务调度回调函数
* @param pFunc, 传入回调函数地址
* @return 
***********************************************************
*/
void TaskScheduleCbReg(void (*pFunc)(void))
{
	g_pTaskScheduleFunc = pFunc;
}

/**
***********************************************************
* @brief systick初始化
* @param
* @return 
***********************************************************
*/
void SysTick_Init(void)
{
	/* SystemFrequency / 1000    1ms中断一次
	 * SystemFrequency / 100000	 10us中断一次
	 * SystemFrequency / 1000000 1us中断一次
	 */
	if (SysTick_Config(SystemCoreClock / 1000))	// ST3.5.0库版本
	{ 
		/* Capture error */ 
		while (1);
	}
}

/**
***********************************************************
* @brief 定时中断服务函数，1ms产生一次中断
* @param
* @return 
***********************************************************
*/
void SysTick_Handler(void)
{	
	g_sysRunTime++;
	if (g_pTaskScheduleFunc == NULL)
	{
		return;
	}
	g_pTaskScheduleFunc();

}

/**
***********************************************************
* @brief 获取系统运行时间
* @param
* @return 以1ms为单位
***********************************************************
*/
uint64_t GetSysRunTime(void)
{
	return g_sysRunTime;
}

/*********************************************END OF FILE**********************/
