#ifndef __INVALID_APP_H
#define	__INVALID_APP_H

#include <stdint.h>

#define Invalid_START_Sector        257   //  开始存储离散开关量输入变化时刻的时间信息的扇区开始地址
/**
***********************************************************
* @brief 返回失效时间
* @param pageindex  指针buffer
* @return 
***********************************************************
*/
void GetInvalidTime(uint16_t pageindex ,uint8_t * buffer);

void Invalid_Task(void );


#endif
