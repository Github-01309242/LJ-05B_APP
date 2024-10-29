#include "stm32f10x.h"

void InitIrqAfterBoot(void)
{
  NVIC_SetVectorTable(FLASH_BASE, 0x3000);
	__enable_irq();
}

void ResetToBoot(void)
{
	__disable_irq();    //关闭所有中断
	NVIC_SystemReset(); //复位函数，需要一些执行的时间
}
