#include <stdint.h>
#include "stm32f10x.h"


/**
***********************************************************
* @brief 独立看门狗初始化，超时时间2S
* @param
* @return 
***********************************************************
*/
void IwdgDrvInit(void)
{
  /*使能IWDG_PR和IWDG_RLR两个寄存器*/
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /*设置预分频器*/
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  /*设置重装载寄存器*/
  IWDG_SetReload(299);    //1s ≈ (1/40000)*256*(299+1)    看门狗喂狗时间定时1.92s以内
  /* 开启开门狗 */
  IWDG_Enable();
}

/**
***********************************************************
* @brief 喂狗
* @param
* @return 
***********************************************************
*/

void FeedDog(void)
{
	IWDG_ReloadCounter();
}

