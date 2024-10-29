/*
 * 失效管理应用程序
 */
#include "switch_drv.h"
#include <stdbool.h>
#include "led_drv.h"
#include "rtc_drv.h"
#include "norflash_drv.h"
#include "switch_drv.h"
#include "mov_invalid.h"
#include "invalid_app.h"

#define DiscreteInputsNum   2     //  一共2个离散信号(遥信)输入 

static RtcTime_t RtcTime[2]={0};

#define   BUFFER_SIZE              4096
#define   FLASH_ADDRESS            4096

static void SwitchinputProc(void )
{
  static uint8_t  flag[2] = {0x00 ,0x00} ;
  uint8_t* STA = GetSwitchinputStatus();
  for (uint8_t  i = 0; i < DiscreteInputsNum; i++) 
  {
      uint8_t  mask = 0x01;  
      bool bit = (bool )(STA[i] & mask);  // 使用 & 操作符来检查该位是0还是1

      if( bit  && !flag[i])
      {
        flag[i] = 0x01 ;
        TurnOnLed (i+1);
        GetRtcTime ( &RtcTime[i] );   //获取发生时间
        
        EraseNorflashForWrite(FLASH_ADDRESS*(Invalid_START_Sector+i), sizeof(RtcTime[0]));
        WriteNorflashData(FLASH_ADDRESS*(Invalid_START_Sector+i), (uint8_t *)&RtcTime[i], sizeof(RtcTime[0]));
      }   
      else if( bit == false )
      {   
        flag[i] = 0x00 ;          
        TurnOffLed (i+1);
      }  
      
  }
}

/**
***********************************************************
* @brief 返回失效时间
* @param pageindex  指针buffer
* @return 
***********************************************************
*/
void GetInvalidTime(uint16_t pageindex ,uint8_t * buffer)
{
  ReadNorflashData ((uint32_t )(pageindex*4096),buffer,sizeof( RtcTime_t));
}

void Invalid_Task(void )
{
  SwitchinputProc();
  MovInvalidProc();
}

