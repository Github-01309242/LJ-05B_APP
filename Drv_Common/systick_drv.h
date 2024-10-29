#ifndef __SYSTICK_H
#define __SYSTICK_H


#include "stm32f10x.h"

void SysTick_Init(void);

void ZDYdelaynus(uint16_t  time);        

/**
***********************************************************
* @brief 注册任务调度回调函数
* @param pFunc, 传入回调函数地址
* @return 
***********************************************************
*/
void TaskScheduleCbReg(void (*pFunc)(void));
/**
***********************************************************
* @brief 获取系统运行时间
* @param
* @return 以1ms为单位
***********************************************************
*/
uint64_t GetSysRunTime(void);


#endif /* __SYSTICK_H */
