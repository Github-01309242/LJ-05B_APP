#ifndef _SURGE_HISTORY_H_
#define _SURGE_HISTORY_H_

#include <stdint.h>
#include "rtc_drv.h"

/*保存浪涌相关信息的代码*/
typedef struct {
    uint16_t    Num;
	
	/* 添加配置参数开始 */
    uint16_t    PeakCurrent;
    RtcTime_t   RtcTime;       //定义结构体变量
	
	/* 添加配置参数结束 */
} SurgeInfo_t;

void SurgeHistoryProc(void );
//uint8_t * RecvStr(void );
void GetSurgeHistoryInfo(uint16_t pageindex ,uint8_t * buffer);
void EraseSurgeHistoryInfo(void );

#endif
