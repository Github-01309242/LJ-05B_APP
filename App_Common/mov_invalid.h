#ifndef _MOV_INVALID_H_
#define _MOV_INVALID_H_

#include <stdint.h>

/*显示档位*/
typedef enum
{
	MOV_LEVEL_GRID_0,
	MOV_LEVEL_GRID_1,
	MOV_LEVEL_GRID_2,
	MOV_LEVEL_GRID_3,
	MOV_LEVEL_GRID_4,
	OTHERLEVEL
} MovLevel_t;

typedef enum 
{
	EXCELLENT,
	MILD,          //轻微
	MODERATE,      //中度
	SEVERE,        //严重的
	BROKEN,        //完全损坏
	OTHERINDEX
} MovQualityIndex;

MovLevel_t GetMovLevel(void);
void MovInvalidProc(void );

#endif

