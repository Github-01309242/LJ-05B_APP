
#ifndef __SURGEMDL_DRV_H
#define	__SURGEMDL_DRV_H

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    uint16_t    Num;	
    uint16_t    PeakCurrent;	
} SurgemdlParam_t;

/**
***********************************************************
* @brief 长沙雷击传感器模块转串口硬件初始化
* @param
* @return 
***********************************************************
*/
void SurgeMdlDrvInit(void);
void SurgemdlDataPro(void);
bool GetRefreshflag(void );
void PresetNum_Surge(uint16_t data);
bool GetSurgemdlData(SurgemdlParam_t*  SurgePara);
void SurgemdlFuncTest(void );

#endif
