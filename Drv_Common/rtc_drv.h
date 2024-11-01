#ifndef _RTC_DRV_H_
#define _RTC_DRV_H_

#include <stdint.h>

//此处定义了一个结构体类型，此时未分配内存单元
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RtcTime_t;

/**
***********************************************************
* @brief RTC硬件初始化
* @param
* @return 
***********************************************************
*/
void RtcDrvInit(void);

/**
***********************************************************
* @brief 设置时间
* @param time,输入，日历时间
* @return 
***********************************************************
*/
void SetRtcTime(RtcTime_t *time);

/**
***********************************************************
* @brief 获取时间
* @param time,输出，日历时间
* @return 
***********************************************************
*/
void GetRtcTime(RtcTime_t *time);

#endif
