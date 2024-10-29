/*
 * 系统储存应用程序
 */
#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "eeprom_drv.h"
#include "mb.h"
#include "rtc_drv.h"
#include "led_drv.h"

#include "modbus_slave.h"
#include "store_app.h"
#include "surgemdl_drv.h"
#include "systeminf.h"

#define MAGIC_CODE     0x5A5A
static const SysParam_t g_sysParamDefault =
{
  .magicCode = MAGIC_CODE,
	.modbusAddr = 1,
  .modbusBaud = 9600,
  .LightningNum = 0
};

static SysParam_t g_sysParamCurrent;


#define SYSPARAM_MAX_SIZE              128
#define SYSPARAM_START_ADDR            0
#define BACKUP_START_ADDR              128

static uint8_t CalcCrc8(uint8_t *buf, uint32_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t byte = 0; byte < len; byte++)
    {
      crc ^= (buf[byte]);
      for (uint8_t i = 8; i > 0; --i)
      {
        if (crc & 0x80)
        {
          crc = (crc << 1) ^ 0x31;
        }
        else 
        {	
          crc = (crc<<1);
        }
      }
    }
    return crc;
}

static bool ReadDataWithCheck(uint8_t readAddr, uint8_t *pBuffer, uint16_t numToRead)
{
	if (!ReadEepromData(readAddr, pBuffer, numToRead))
	{
		return false;
	}
	uint8_t crcVal = CalcCrc8(pBuffer, numToRead - 1);
	if (crcVal != pBuffer[numToRead - 1])
	{
		return false;
	}
	return true;
}

bool ReadSysParam(SysParam_t *sysParam)
{
	uint16_t sysParamLen = sizeof(SysParam_t);
	
	if (ReadDataWithCheck(SYSPARAM_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return true;
	}
	if (ReadDataWithCheck(BACKUP_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return true;
	}
	return false;
}

static bool WriteDataWithCheck(uint8_t writeAddr, uint8_t *pBuffer, uint16_t numToWrite)
{
	pBuffer[numToWrite - 1] = CalcCrc8(pBuffer, numToWrite - 1);
	if (!WriteEepromData(writeAddr, pBuffer, numToWrite))
	{
		return false;
	}
	return true;
}
	
bool WriteSysParam(SysParam_t *sysParam)
{
	uint16_t sysParamLen = sizeof(SysParam_t);
	if (sysParamLen > SYSPARAM_MAX_SIZE)
	{
		return false;
	}
	if (!WriteDataWithCheck(SYSPARAM_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return false;
	}
	
	WriteDataWithCheck(BACKUP_START_ADDR, (uint8_t *)sysParam, sysParamLen);
	
	return true;
}

void InitSysParam(void)
{
	SysParam_t sysParam;
	
	if (ReadSysParam(&sysParam) && sysParam.magicCode == MAGIC_CODE)
	{	
		g_sysParamCurrent = sysParam;
    /*把掉电前保存的次数作为重新计数初始值*/
    PresetNum_Surge(g_sysParamCurrent.LightningNum); 
		return;
	}
	g_sysParamCurrent = g_sysParamDefault;
  
  if (!WriteSysParam(&g_sysParamCurrent))
	{
		return;
	}
}

bool SetModbusAddrParam(uint8_t addr)
{
	if (addr == g_sysParamCurrent.modbusAddr  )
	{
		return true;
	}	 
	SysParam_t sysParam = g_sysParamCurrent;
	sysParam.modbusAddr = addr;	
	if (eMBSetSlaveAddr(addr) != MB_ENOERR)
	{
		return false;
	}
	if (!WriteSysParam(&sysParam))
	{
		eMBSetSlaveAddr(g_sysParamCurrent.modbusAddr);
		return false;
	}
	g_sysParamCurrent = sysParam;

	return true;
}

bool SetModbusBaudParam(uint32_t buad)
{
	if (buad == g_sysParamCurrent.modbusBaud   )
	{
		return true;
	}
	 
	SysParam_t sysParam = g_sysParamCurrent;
	sysParam.modbusBaud = buad;

	if (!WriteSysParam(&sysParam))
	{
		
		return false;
	}
	g_sysParamCurrent = sysParam;

	return true;
}


/**
***********************************************************
* @brief 获取modbus地址
* @param
* @return 雷击次数
***********************************************************
*/

uint8_t  GetModbusAddrData(void)
{ 
	return g_sysParamCurrent.modbusAddr  ;
}

/**
***********************************************************
* @brief 获取modbus波特率
* @param
* @return 
***********************************************************
*/

uint32_t  GetModbusBaudData(void)
{ 
	return g_sysParamCurrent.modbusBaud ;
}




 


