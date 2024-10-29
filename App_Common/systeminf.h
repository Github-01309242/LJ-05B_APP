#ifndef _SYSTEMINF_H_
#define _SYSTEMINF_H_

#include <stdint.h>

typedef struct {
    uint16_t  magicCode;
	
	/* 添加配置参数开始 */
    uint8_t modbusAddr;
    uint32_t modbusBaud;
    uint8_t LightningNum;
	
	/* 添加配置参数结束 */
	
	uint8_t crcVal;
} SysParam_t;



#endif

