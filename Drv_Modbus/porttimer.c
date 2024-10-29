/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "stm32f10x.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{

  NVIC_InitTypeDef NVIC_InitStructure;
 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	 
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  /* 使能定时器时钟；*/
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5  , ENABLE);   
  /* 复位定时器；*/
	TIM_DeInit(TIM5);
  /* 设置自动重装载值；*/ 
	TIM_TimeBaseStructure.TIM_Period = usTim1Timerout50us - 1;
  /* 设置预分频器值；*/ 
  // 频率72MHZ / 3600 = 20khz,对应周期50us
	TIM_TimeBaseStructure.TIM_Prescaler = 3600 - 1; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM5,TIM_FLAG_Update);
  /* 使能定时器的计数更新中断；*/
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);	

  /* 使能定时器中断和优先级；*/
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;            
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           
	NVIC_Init(&NVIC_InitStructure);              
  /* 使能定时器；*/ 
  //	TIM_Cmd(TIM5, ENABLE);	
  return TRUE ;
}


inline void
vMBPortTimersEnable(  )
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
  /* 清零定时器 */  
  TIM_SetCounter(TIM5 , 0);
  /* 清除定时器更新中断 */
  TIM_ClearFlag(TIM5,TIM_FLAG_Update);
  /* 使能定时器更新中断 */
  TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);	
  /* 使能定时器；*/ 
 	TIM_Cmd(TIM5, ENABLE);
  
}

inline void
vMBPortTimersDisable(  )
{
    /* Disable any pending timers. */
  /* 清零定时器 */  
  TIM_SetCounter(TIM5 , 0);
  /* 清除定时器更新中断 */
  TIM_ClearFlag(TIM5,TIM_FLAG_Update);
  /* 禁用定时器更新中断 */
  TIM_ITConfig(TIM5,TIM_IT_Update,DISABLE);	
  /* 失能定时器*/ 
 	TIM_Cmd(TIM5, DISABLE);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}

void TIM5_IRQHandler(void)
{	
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)!=RESET)
   {
		 TIM_ClearITPendingBit(TIM5 , TIM_IT_Update); 
		 TIM_SetCounter(TIM5 , 0);
     prvvTIMERExpiredISR();
	 } 		 
}
