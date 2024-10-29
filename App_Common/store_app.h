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
* @brief ��ȡ�׻�����
* @param
* @return �׻�����
***********************************************************
*/

uint16_t  GetLightNumData(void);

/**
***********************************************************
* @brief ��ȡmodbus��ַ
* @param
* @return �׻�����
***********************************************************
*/

uint8_t  GetModbusAddrData(void);

/**
***********************************************************
* @brief ��ȡmodbus������
* @param
* @return 
***********************************************************
*/

uint32_t  GetModbusBaudData(void);

/**
***********************************************************
* @brief ��ȡeeprom����洢����ӿ����
* @param
* @return 
***********************************************************
*/

uint32_t  GetsysNum(void);


#endif
