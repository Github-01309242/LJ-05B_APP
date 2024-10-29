#include <stdint.h>
#include "stm32f10x.h"
#include "led_drv.h"
#include "norflash_drv.h"

static void TimerInit(uint32_t periodUs)
{
  
	/*((1+TIM_Prescaler )/72M)*(1+TIM_Period )=((1+7200-1)/72M)*(1+1000-1)=100ms */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 
  RCC_APB1PeriphClockCmd(  RCC_APB1Periph_TIM2   , ENABLE);  
		/*********TIME2************/
	TIM_DeInit(TIM2);

  TIM_TimeBaseStructure.TIM_Prescaler= 7200 - 1;  
  
	TIM_TimeBaseStructure.TIM_Period= periodUs - 1;    

	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM2,TIM_FLAG_Update);

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

	TIM_Cmd(TIM2,ENABLE); 
    
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			     	//设置TIM2中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	     	//抢占优先级 1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;			 	//子优先级为0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					  //使能
  NVIC_Init(&NVIC_InitStructure);  
}

uint8_t g_timer = 0;

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
	{		
    g_timer++;
    if( g_timer > 10 ) g_timer=0;
      ToggleLed (0);  

//      uint8_t  bufferRead[20] = {0};
//      ReadNorflashData ((uint32_t )(2*4096),bufferRead,20);
    
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}


void TimingDrvInit(void)
{
	TimerInit(1000);   
}

uint8_t Getgtimer(void)  
{
  return g_timer;
}


