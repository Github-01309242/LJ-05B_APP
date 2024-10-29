#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "stm32f10x.h"
#include "delay.h"

static float g_tempData;
#define MAX_BUF_SIZE         10
static uint16_t g_temp10MplBuf[MAX_BUF_SIZE];
	
static const uint16_t g_ntcAdcTable[] = {
  3448, 3420, 3390, 3360, 3329, 3319, 3285, 3250, 3215, 3178,     //-10   ~   -1℃
	3141, 3103, 3064, 3024, 2984, 2943, 2901, 2859, 2816, 2773,     //0   ~   9℃
	2729, 2685, 2640, 2596, 2550, 2505, 2459, 2413, 2368, 2322,     //10  ~  19℃
	2276, 2230, 2184, 2138, 2093, 2048, 2003, 1958, 1913, 1870,     //20  ~  29℃
	1826, 1783, 1740, 1698, 1657, 1616, 1576, 1536, 1497, 1458,     //30  ~  39℃
	1420, 1383, 1347, 1311, 1276, 1242, 1208, 1175, 1143, 1112,     //40  ~  49℃
	1081, 1051, 1022, 993,  965,  938,  912,  886,  861,  836,      //50  ~  59℃
	813,  789,  767,  745,  724,  703,  683,  663,  644,  626,      //60  ~  69℃
	608,  590,  573,  557,  541,  525,  510,  496,  481,  468,      //70  ~  79℃
	454,  441,  429,  417,  405,  393,  382,  372,  361,  351,      //80  ~  89℃
	341,  332,  323,  313,  305,  297,  289,  280,  273,  265,      //90  ~  99℃
  258,  251,  245,  238,  231,  225,  219,  213,  208,  202,      //100  ~  109℃
  197,  192,  187,  182,  177,  173,  168,  164,  159,  156,      //110  ~  119℃
  151,  148,  144,  140,  137,  134,  130,  127,  124,  120,      //120  ~  129℃
  118,  115,  112,  109,  106,  104,  101,  99,   97,   94,       //130  ~  139℃
  92,   90,   88,   86,   84,   82,   80,   78,   76,   75,       //140  ~  149℃
  73,   71,   70,   68,   66,   65,   64,   62,   61,   60,       //150  ~  159℃
  58,                                                             //160  ~  169℃ 
};
#define NTC_TABLE_SIZE         (sizeof(g_ntcAdcTable) / sizeof(g_ntcAdcTable[0]))
#define INDEX_TO_TEMP(index)   ((int32_t)(index - 10))

static void GpioInit(void)
{
  // 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
  
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;               
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
static void AdcInit(void)
{
  /* 使能ADC1通道时钟 */
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1	, ENABLE );	  
  /* 设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M  */
  RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
  /* 复位ADC1 */  
  ADC_DeInit(ADC1);  
  ADC_InitTypeDef ADC_InitStructure;

  /* ADC1配置 */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                 //工作模式：独立模式
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                      //循环模式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE  ;               //ADC工作在连续转化模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//软件触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;             //ADC数据右对齐
  ADC_InitStructure.ADC_NbrOfChannel = 1;                            //1个通道
  ADC_Init(ADC1, &ADC_InitStructure);

  /******************************************************************
     设置指定ADC的规则组通道，设置通道对应的转化顺序和采样时间
  *******************************************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_55Cycles5);
  ADC_Cmd(ADC1, ENABLE);                                             //使能ADC
  ADC_ResetCalibration(ADC1);                                        //校验复位
  while(ADC_GetResetCalibrationStatus(ADC1));                        //等待复位完成
  ADC_StartCalibration(ADC1);                                        //开始ADC1校准
  while(ADC_GetCalibrationStatus(ADC1));                             //等待校验完成
  /* 启动转换 */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);    
}

static uint16_t GetAdcVal(void)
{
  /* 等待转换完成标志 */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

  /* 读取数据寄存器 */
  return (ADC_GetConversionValue(ADC1));
}

/**
***********************************************************
* @brief NTC驱动初始化
* @param
* @return 
***********************************************************
*/
void TempDrvInit(void)
{
	GpioInit();
	AdcInit();
}


/**
***********************************************************
* @brief 二分查找法
* @param 数据从大到小排列，此时可以查找出第一个 小于等于 目标值的元素
* @return 
***********************************************************
*/
static int32_t DescBinarySearch(const uint16_t *arr, int32_t size, uint16_t key) 
{
	int32_t left = 0;              			
	int32_t right = size - 1;       		
	int32_t mid;
	int32_t index = size - 1;
	while (left <= right)             		
	{
		mid = left + (right - left) / 2; 
		if (key >= arr[mid])
		{
			right = mid - 1;
			index = mid;
		}
		else
		{
			left = mid + 1;  
		}
	}
    	return  index  ;               				
}




/*
  通常使用补码表示负数，因为补码更能反映出负数的实际对应关系。
  4.举例（十进制值 = 二进制值）
  原码：+5 = 0000 0101
  反码：-5 = 1111 1010
  补码：-5 = 反码+1 = 1111 1011
*/
//#define Table    //查表法

#ifdef Table
static uint16_t AdcToTemp10Mpl(uint16_t adcVal)
{
  /*数据从大到小排列，此时可以查找出第一个 小于等于 目标值的元素*/
	int32_t index = DescBinarySearch(g_ntcAdcTable, NTC_TABLE_SIZE, adcVal);
	
	if (index == 0)
	{
    g_isSensorOk = false ;
		return 0;
	}
  /*实现0.1℃的精度*/
  uint16_t temp10Mpl = INDEX_TO_TEMP(index - 1) * 10 + (g_ntcAdcTable[index - 1] - adcVal) * 10 / (g_ntcAdcTable[index - 1] - g_ntcAdcTable[index]);
  g_isSensorOk = true ;
  return temp10Mpl;
}

#else
static uint16_t AdcToTemp10Mpl(uint16_t adcVal)
{
  float vol;
  float res;
  float Tc;

  vol = (adcVal*3.3)/4096;

  res = (10000*vol)/(3.3-vol);

  Tc = (3950.0/(log(res/10000)+(3950.0/298.15)))-273.15;
  
  return (Tc*10);
}


#endif

/**
***********************************************************
* @brief 算术平均滤波
* @param arr，数组首地址
* @param len，元素个数
* @return 平均运算结果
***********************************************************
*/
static uint16_t ArithAvgFltr(uint16_t *arr, uint32_t len)
{
	uint32_t sum = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		sum += arr[i];
	}
	return (uint16_t)(sum / len);
}

/**
***********************************************************
* @brief qsort函数调用的回调函数，比较规则，降序排列
* @param *_a，对应数组元素
* @param *_b，对应数组元素
* @return 比较结果
***********************************************************
*/
static int32_t CmpCb(const void *_a, const void *_b)
{
	uint16_t *a = (uint16_t *)_a;
	uint16_t *b = (uint16_t *)_b;
	int32_t val = 0;
	 if (*a > *b)
	 {
		val = -1;
	 }
	 else if (*a < *b)
	 {
		val =  1;
	 }
	 else
	 {
		val = 0;
	 }
	 return val;
}

/**
***********************************************************
* @brief 中位值平均滤波
* @param arr，数组首地址
* @param len，元素个数，需要大于等于3个
* @return 平均运算结果
***********************************************************
*/
static uint16_t MedianAvgFltr(uint16_t *arr, uint32_t len)
{
	qsort(arr, len, sizeof(uint16_t), CmpCb);
	return ArithAvgFltr(&arr[1], len - 2);
}

void PushDataToBuf(uint16_t temp10Mpl)
{
	static uint16_t s_index = 0;
	g_temp10MplBuf[s_index] = temp10Mpl; 
	s_index++;
	s_index %= MAX_BUF_SIZE;
}


bool  g_isSensorOk = true ;

/**
***********************************************************
* @brief 触发驱动转换温度传感器数据
* @param
* @return 
***********************************************************
*/
void TempSensorProc(void)
{
	static uint16_t s_convertNum = 0;

	uint16_t adcVal = GetAdcVal();
  
  if( adcVal > 0xFC0)
  {
   g_tempData = 6553.5f;     //报文输出0xFFFF
    
   g_isSensorOk = false  ;
    
   return ; 
  }
  else  g_isSensorOk = true  ;
  
	uint16_t temp10Mpl = AdcToTemp10Mpl(adcVal);
	
	PushDataToBuf(temp10Mpl);
	s_convertNum++;
	if (s_convertNum < 3)
	{
		g_tempData = g_temp10MplBuf[0] / 10.0f ;
		return;
	}
	
	if (s_convertNum > MAX_BUF_SIZE)
	{
		s_convertNum = MAX_BUF_SIZE;
	}
	
	g_tempData = MedianAvgFltr(g_temp10MplBuf, s_convertNum) / 10.0f ;
}

/**
***********************************************************
* @brief 获取温度传感器数据
* @param
* @return bool类型
***********************************************************
*/

bool GetTempData(float *tempData)
{
  *tempData = g_tempData;
  
	if (g_isSensorOk)
	{		
		return true;
	}
  
	return false;
}


