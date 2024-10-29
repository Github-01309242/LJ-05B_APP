#include <stdint.h>
#include "stm32f10x.h"
#include "systick_drv.h"


typedef struct
{
	uint32_t rcu;
	GPIO_TypeDef * gpio;
	uint16_t pin;
} Key_GPIO_t;

static Key_GPIO_t g_gpioList[] = 
{
	{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_4},    // key1
	{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_3},    // key2
	{RCC_APB2Periph_GPIOD, GPIOD, GPIO_Pin_2},    // key3
  {RCC_APB2Periph_GPIOC, GPIOC, GPIO_Pin_12}    // key4
};

#define KEY_NUM_MAX (sizeof(g_gpioList) / sizeof(g_gpioList[0]))

typedef enum
{
	KEY_RELEASE = 0, // 释放松开
	KEY_CONFIRM,     // 消抖确认
	KEY_SHORTPRESS,  // 短按
	KEY_LONGPRESS    // 长按
} KEY_STATE;

#define CONFIRM_TIME                 10       // 按键消抖时间窗10ms
#define DOUBLE_CLICK_INTERVAL_TIME   300      // 两次抬起时间窗300ms，用来判断是否双击
#define LONGPRESS_TIME               1000     // 长按时间窗1000ms

typedef struct
{
	KEY_STATE keyState;
	uint8_t singleClickNum;
	uint64_t firstIoChangeSysTime;
	uint64_t firstReleaseSysTime;
} Key_Info_t;

static Key_Info_t g_keyInfo[KEY_NUM_MAX];
/**
***********************************************************
* @brief 按键硬件初始化
* @param
* @return 
***********************************************************
*/
#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled*/

void KeyDrvInit(void)
{  
    /**1、打开复用时钟：**/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  /**2、对PB3，PB4，PA15三个引脚进行重映射：**/
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*使能SWD 禁用JTAG*/
  
  GPIO_InitTypeDef GPIO_InitStructure;	
  
	for (uint8_t i = 0; i < KEY_NUM_MAX; i++)
	{
	 RCC_APB2PeriphClockCmd(g_gpioList[i].rcu , ENABLE);
    
	 GPIO_InitStructure.GPIO_Pin = g_gpioList[i].pin ;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	  
   GPIO_Init(g_gpioList[i].gpio ,&GPIO_InitStructure);
	}
}

static uint8_t KeyScan(uint8_t keyIndex)
{
	volatile uint64_t curSysTime;
	uint8_t keyPress;
	
	keyPress = GPIO_ReadInputDataBit(g_gpioList[keyIndex].gpio, g_gpioList[keyIndex].pin);

	switch (g_keyInfo[keyIndex].keyState)
	{
		case KEY_RELEASE:                                          //  释放状态：判断有无按键按下
			if (!keyPress)                                         //  有按键按下
			{ 
				g_keyInfo[keyIndex].keyState = KEY_CONFIRM;        //  获取系统运行时间
				g_keyInfo[keyIndex].firstIoChangeSysTime = GetSysRunTime();
				break;
			}

			if (g_keyInfo[keyIndex].singleClickNum != 0)
			{
				curSysTime = GetSysRunTime();
				if (curSysTime - g_keyInfo[keyIndex].firstReleaseSysTime > DOUBLE_CLICK_INTERVAL_TIME)
				{
					g_keyInfo[keyIndex].singleClickNum = 0;
					return (keyIndex + 1); //  返回单击按键码值，三个按键短按对应0x01, 0x02, 0x03
				}
			}

			break;
			
		case KEY_CONFIRM:
			if (!keyPress)
			{
				curSysTime = GetSysRunTime();
				if (curSysTime - g_keyInfo[keyIndex].firstIoChangeSysTime > CONFIRM_TIME) //  超过时间窗，切换到短按
				{
					g_keyInfo[keyIndex].keyState = KEY_SHORTPRESS;
				}
			}
			else
			{
				g_keyInfo[keyIndex].keyState = KEY_RELEASE;
			}
			break;
			
		case KEY_SHORTPRESS:           //  短按状态：继续判定是短按，还是双击，还是长按
			if (keyPress)
			{
				g_keyInfo[keyIndex].keyState = KEY_RELEASE;
				
				g_keyInfo[keyIndex].singleClickNum++;
				if (g_keyInfo[keyIndex].singleClickNum == 1)
				{
					g_keyInfo[keyIndex].firstReleaseSysTime = GetSysRunTime();
					break;
				}
				else
				{
					curSysTime = GetSysRunTime();
					if (curSysTime - g_keyInfo[keyIndex].firstReleaseSysTime <= DOUBLE_CLICK_INTERVAL_TIME) //  超过双击时间间隔窗，返回双击码值
					{
						g_keyInfo[keyIndex].singleClickNum = 0;
						return (keyIndex + 0x51); //  返回按键码值，三个按键双击对应0x51, 0x52, 0x53
					}
				}
			}
			else
			{
				curSysTime = GetSysRunTime();
				if (curSysTime - g_keyInfo[keyIndex].firstIoChangeSysTime > LONGPRESS_TIME)
				{	
					g_keyInfo[keyIndex].keyState = KEY_LONGPRESS;
				}
			}
			break;
		case KEY_LONGPRESS:
			if (keyPress)
			{
				g_keyInfo[keyIndex].keyState = KEY_RELEASE;
				return (keyIndex + 0x81); //返回按键码值，三个按键长按对应0x81, 0x82, 0x83
			}
			break;
		default:
			g_keyInfo[keyIndex].keyState = KEY_RELEASE;
			break;
	}
	return 0;
}
/**
***********************************************************
* @brief 获取按键码值
* @param
* @return 三个按键码值，短按0x01 0x02 0x03 0x04，
			                  长按0x81 0x82 0x83 0x84，
                        没有按下为0
***********************************************************
*/
uint8_t GetKeyVal(void)
{
	uint8_t res = 0;

	for (uint8_t i = 0; i < KEY_NUM_MAX; i++)
	{
		res = KeyScan(i);
		if (res != 0)
		{
			break;
		}
	}
	return res;
}

