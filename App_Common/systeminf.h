#ifndef _SYSTEMINF_H_
#define _SYSTEMINF_H_

#include <stdint.h>

typedef struct {
    uint16_t  magicCode;
	
	/* ������ò�����ʼ */
    uint8_t modbusAddr;
    uint32_t modbusBaud;
    uint8_t LightningNum;
	
	/* ������ò������� */
	
	uint8_t crcVal;
} SysParam_t;



#endif

