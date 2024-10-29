
#include "stm32f10x.h"
#include <time.h>
#include "rtc_drv.h"
#include "delay.h"
#include <stdint.h>
#include <string.h>


#define MAGIC_CODE    0x5A5A

/**
***********************************************************
* @brief RTC硬件初始化
* @param
* @return 
***********************************************************
*/
void RtcDrvInit(void)
{
	u8 temp=0;
  
  /*从指定的后备寄存器中读出数据:读出了与写入的指定数据不相同*/
	if ( BKP_ReadBackupRegister(BKP_DR1) != MAGIC_CODE )		
  {	    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
    PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
    BKP_DeInit();	//复位备份区域 	
    RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
    {
    temp++;
    DelayNms(10);
    }
    if(temp>=250) return;                     //初始化时钟失败,晶振有问题	    
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
    RCC_RTCCLKCmd(ENABLE);	                  //使能RTC时钟  
    RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
    RTC_WaitForSynchro();		                  //等待RTC寄存器同步  
    RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
    RTC_EnterConfigMode();                    // 允许配置	
    RTC_SetPrescaler(32767);                  //设置RTC预分频的值
    RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
    /* 2024年5月24日 15:30:53 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
    PWR_BackupAccessCmd(ENABLE);	            //使能RTC和后备寄存器访问 
    RTC_SetCounter(1716535853);	              //设置RTC计数器的值
    RTC_WaitForLastTask();	                 //等待最近一次对RTC寄存器的写操作完成  	
    /* 退出配置模式 */
    RTC_ExitConfigMode();  
    /* 向指定的后备寄存器中写入用户程序数据  */    
    BKP_WriteBackupRegister(BKP_DR1, MAGIC_CODE);	     
    return;    
  }
  /* 使能对RTC的访问，使能PMU和BKP时钟 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
  /* 使能对后备寄存器和RTC的写权限 */
  PWR_BackupAccessCmd(ENABLE);	
  /* 等待APB1接口时钟和RTC时钟同步 */
  RTC_WaitForSynchro();	
  /* 等待上次对 RTC 寄存器写操作完成 */
  RTC_WaitForLastTask();   
}		


/**
***********************************************************
* @brief 设置时间
* @param time,输入，日历时间
* @return 
***********************************************************
*/
void SetRtcTime(RtcTime_t *time)
{
  time_t timeStamp;
  struct tm tmInfo;
  memset(&tmInfo, 0, sizeof(tmInfo));

  tmInfo.tm_year = time->year - 1900;
  tmInfo.tm_mon = time->month - 1;
  tmInfo.tm_mday = time->day;
  tmInfo.tm_hour = time->hour;
  tmInfo.tm_min = time->minute;
//  tmInfo.tm_sec = time->second;

  timeStamp = mktime(&tmInfo) - 8 * 60 * 60;

  RTC_WaitForLastTask();	 //等待最近一次对RTC寄存器的写操作完成
  RTC_SetCounter(timeStamp);
}

/**
***********************************************************
* @brief 获取时间
* @param time,输出，日历时间
* @return 
***********************************************************
*/
void GetRtcTime(RtcTime_t *time)
{
	time_t timeStamp;
	struct tm *tmInfo;
	
	timeStamp = RTC_GetCounter() + 8 * 60 * 60;
	tmInfo = localtime(&timeStamp);
	time->year = tmInfo->tm_year + 1900 ;
	time->month = tmInfo->tm_mon + 1;
	time->day = tmInfo->tm_mday;
	time->hour = tmInfo->tm_hour;
	time->minute = tmInfo->tm_min;
	time->second = tmInfo->tm_sec;
}
		  


/************************END OF FILE***************************************/
