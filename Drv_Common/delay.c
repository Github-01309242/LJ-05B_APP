#include <stdint.h>
#include "stm32f10x.h"

#define  DEM_CR      *(volatile u32 *)0xE000EDFC
 
#define  DWT_CR      *(volatile u32 *)0xE0001000
 
#define  DWT_CYCCNT  *(volatile u32 *)0xE0001004
 
#define  DEM_CR_TRCENA                   (1 << 24)
 
#define  DWT_CR_CYCCNTENA                (1 <<  0)


/**
***********************************************************
* @brief DWT初始化配置
* @param
* @return 
***********************************************************
*/
void DelayInit(void) 
{
    /* 先使能DWT外设，由内核调试寄存器DEM_CR的位24控制，写1使能 */
    DEM_CR  |=  (uint32_t )DEM_CR_TRCENA; 
    /* 使能CYCCNT寄存器之前，要先清0*/
    DWT_CYCCNT = 0;
    /* 使能CYCCNT寄存器，由DWT_CTRL的位0控制，写1使能 */
    DWT_CR  |=  (uint32_t )DWT_CR_CYCCNTENA;


}


/**
***********************************************************
* @brief 微秒级延时函数
* @param nUs，最大延时时间( 2^32 / 内核主频 ) * 10^6 us 
* @return 
***********************************************************
*/
void DelayNus(uint32_t uSec)
{

  int ticks_start, ticks_end, ticks_delay;
  
  ticks_start = DWT_CYCCNT;
   
  ticks_delay = ( uSec * ( 72000000 / (1000*1000) ) ); // 将微秒数换算成滴答数          
  
  ticks_end = ticks_start + ticks_delay;
  
  if ( ticks_end > ticks_start )
  {
    while( DWT_CYCCNT < ticks_end );
  }
  else // 计数溢出，翻转
  {
    while( DWT_CYCCNT >= ticks_end ); // 翻转后的值不会比ticks_end小
    while( DWT_CYCCNT < ticks_end );
  }
}

/**
***********************************************************
* @brief 毫秒级延时函数
* @param nMs，延时时间n毫秒
* @return 
***********************************************************
*/
void DelayNms(uint32_t nMs)
{
	for (uint32_t i = 0; i < nMs; i++)
	{
		DelayNus(1000);
	}
}


