#include <stdbool.h>
#include "key_drv.h"
#include "lcd12864_drv.h"
#include "hmi_mainpage.h"
#include "hmi_surge_history.h"
#include "hmi_invalid_history.h"
#include "hmi_serialwork.h"
#include "surge_history.h"

static uint8_t g_currentPageIndex; 
static bool g_isScreenOn = true;
volatile  bool g_RepeatSurgeHistoryPage = false ;

#define MAIN_PAGE_INDEX                0
#define SURGE_HISTORY_PAGE_INDEX       1
#define INVALID_TIME_PAGE_INDEX        2
#define RS485_STATE_PAGE_INDEX         3
#define RS485_CONFIG_PAGE_INDEX        4   // 二级页面
#define PRIMARY_PAGE_NUM_MAX           4
#define SCREEN_OFF_PAGE_INDEX          5   

volatile  int8_t g_ChildKeyIndex[10] = {0};

static void ProcKeyEvent(void)
{
	uint8_t keyVal;
	keyVal = GetKeyVal();
	static uint8_t s_beforeScrOffIndex;
	if (g_currentPageIndex == SURGE_HISTORY_PAGE_INDEX)
	{
		if (keyVal == KEY3_SHORT_PRESS)
		{
			g_currentPageIndex++;
			g_currentPageIndex %= PRIMARY_PAGE_NUM_MAX;
		}
		if (keyVal == KEY1_SHORT_PRESS)
		{
      g_ChildKeyIndex[SURGE_HISTORY_PAGE_INDEX] --;     
		}
    if (keyVal == KEY2_SHORT_PRESS)
		{
      if(g_ChildKeyIndex[SURGE_HISTORY_PAGE_INDEX] != 0)
      {
       g_ChildKeyIndex[SURGE_HISTORY_PAGE_INDEX] ++;      
	    } 
    }
		if (keyVal == KEY3_LONG_PRESS)
		{
			g_isScreenOn = false;
			s_beforeScrOffIndex = g_currentPageIndex;
			g_currentPageIndex = SCREEN_OFF_PAGE_INDEX;
		}
    if (keyVal == KEY4_LONG_PRESS)
		{
			EraseSurgeHistoryInfo();
      clear_screen ();
		}
	}
  else if (g_currentPageIndex == INVALID_TIME_PAGE_INDEX)
	{
    g_RepeatSurgeHistoryPage = true ;
		if (keyVal == KEY3_SHORT_PRESS)
		{
			g_currentPageIndex++;
			g_currentPageIndex %= PRIMARY_PAGE_NUM_MAX;
		}
		if (keyVal == KEY1_SHORT_PRESS)
		{
      if(g_ChildKeyIndex[INVALID_TIME_PAGE_INDEX] != 0)
      {
      g_ChildKeyIndex[INVALID_TIME_PAGE_INDEX] --;    
      }        
		}
    if (keyVal == KEY2_SHORT_PRESS)
		{
      if(g_ChildKeyIndex[INVALID_TIME_PAGE_INDEX] != 1)
      {
      g_ChildKeyIndex[INVALID_TIME_PAGE_INDEX] ++; 
      }  
    }
		if (keyVal == KEY3_LONG_PRESS)
		{
			g_isScreenOn = false;
			s_beforeScrOffIndex = g_currentPageIndex;
			g_currentPageIndex = SCREEN_OFF_PAGE_INDEX;
		}
	}
  else if (g_currentPageIndex == RS485_STATE_PAGE_INDEX)
	{
		if (keyVal == KEY3_SHORT_PRESS)
		{
			g_currentPageIndex++;
			g_currentPageIndex %= PRIMARY_PAGE_NUM_MAX;
		}
		if (keyVal == KEY3_DOUBLE_PRESS)
		{
			g_currentPageIndex = RS485_CONFIG_PAGE_INDEX;
		}
		if (keyVal == KEY3_LONG_PRESS)
		{
			g_isScreenOn = false;
			s_beforeScrOffIndex = g_currentPageIndex;
			g_currentPageIndex = SCREEN_OFF_PAGE_INDEX;
		}
	}
	else if (g_currentPageIndex == RS485_CONFIG_PAGE_INDEX)
	{
		//KEY4长按切换到RS485_STATE_PAGE_INDEX界面,否则不响应所有按键事件 
		if (keyVal == KEY4_LONG_PRESS)
		{
			g_currentPageIndex = RS485_STATE_PAGE_INDEX;
		}
    if (keyVal == KEY1_SHORT_PRESS)
		{
      if(g_ChildKeyIndex[RS485_CONFIG_PAGE_INDEX] != 0)
      {
      g_ChildKeyIndex[RS485_CONFIG_PAGE_INDEX] --;    
      }        
		}
    if (keyVal == KEY2_SHORT_PRESS)
		{
      if(g_ChildKeyIndex[RS485_CONFIG_PAGE_INDEX] != 20)  //设置地址最大不能超过20
      {
      g_ChildKeyIndex[RS485_CONFIG_PAGE_INDEX] ++; 
      }  
    }
	}
	else if (g_currentPageIndex == SCREEN_OFF_PAGE_INDEX)
	{
		if (keyVal != 0)
		{
			g_isScreenOn = true;
			g_currentPageIndex = s_beforeScrOffIndex;
		}
	}
	else
	{
		if (keyVal == KEY3_SHORT_PRESS)
		{
			g_currentPageIndex++;
			g_currentPageIndex %= PRIMARY_PAGE_NUM_MAX;
		}
		if (keyVal == KEY3_LONG_PRESS)
		{
			g_isScreenOn = false;
			s_beforeScrOffIndex = g_currentPageIndex;
			g_currentPageIndex = SCREEN_OFF_PAGE_INDEX;
		}		
	}
}

int8_t * GetleftrightIndex(void )
{
  
  return (int8_t *)g_ChildKeyIndex;
}

bool IsScreenOn(void)
{
	return g_isScreenOn;
}
void HmiTask(void)
{
	static uint8_t s_lastPageIndex; 
	ProcKeyEvent();
  //类似清屏  全部填充黑色
	if (s_lastPageIndex != g_currentPageIndex)
	{
    clear_screen ();
		s_lastPageIndex = g_currentPageIndex;
	}
	switch (g_currentPageIndex)
	{
		case MAIN_PAGE_INDEX:
			TurnOnScreen();
			DisplayMainpage();
			break;
		case SURGE_HISTORY_PAGE_INDEX:
			TurnOnScreen();
			DisplaySurgeHistoryPage() ;
			break;
		case INVALID_TIME_PAGE_INDEX:
			TurnOnScreen();
			DisplayInvalidTimePage();
			break;
		case RS485_STATE_PAGE_INDEX:
			TurnOnScreen();
			DisplayRS485StatePage();
			break;		
		case RS485_CONFIG_PAGE_INDEX:
			TurnOnScreen();
			DisplayRS485ConfigPage();
			break;		
		case SCREEN_OFF_PAGE_INDEX:
			TurnOffScreen();
		break;
	}
}

