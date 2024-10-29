#ifndef _STORE_APP_H_
#define _STORE_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include "systeminf.h"


bool WriteSysParam(SysParam_t *sysParam);
bool ReadSysParam(SysParam_t *sysParam);

bool SetModbusAddrParam(uint8_t addr);
bool SetModbusBaudParam(uint32_t buad);

void InitSysParam(void);
void StoreTask(void );

/**
***********************************************************
* @brief 获取雷击次数
* @param
* @return 雷击次数
***********************************************************
*/

uint16_t  GetLightNumData(void);

/**
***********************************************************
* @brief 获取modbus地址
* @param
* @return 雷击次数
***********************************************************
*/

uint8_t  GetModbusAddrData(void);

/**
***********************************************************
* @brief 获取modbus波特率
* @param
* @return 
***********************************************************
*/

uint32_t  GetModbusBaudData(void);

/**
***********************************************************
* @brief 获取eeprom里面存储的浪涌次数
* @param
* @return 
***********************************************************
*/

uint32_t  GetsysNum(void);


#endif
