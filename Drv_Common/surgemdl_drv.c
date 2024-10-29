
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x.h"
#include "surgemdl_drv.h"
#include "RTT_Debug.h"
#include "sensor_drv.h"

#define MAX_BUF_SIZE           30
static uint8_t  g_recvDataBuf[MAX_BUF_SIZE];
static uint16_t g_recvDataLen;

typedef struct
{
	USART_TypeDef * uartNo;
	uint32_t rcuUart;
	uint32_t rcuGpio;
	GPIO_TypeDef* gpio;
	uint32_t txPin;
	uint32_t rxPin;
	uint8_t irq;
  uint32_t rcuDma;
  DMA_Channel_TypeDef* dmaCh;
  
} UartHwInfo_t;

static UartHwInfo_t g_uartHwInfo = {USART2, RCC_APB1Periph_USART2, RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_2, GPIO_Pin_3, 
                                    USART2_IRQn, RCC_AHBPeriph_DMA1, DMA1_Channel6};


static void SurgeGpioInit(void)
{
  RCC_APB2PeriphClockCmd(g_uartHwInfo.rcuGpio , ENABLE);
   
  GPIO_InitTypeDef GPIO_InitStructure;	
  
  /*将USART Tx配置为复用功能推拉 */
  GPIO_InitStructure.GPIO_Pin = g_uartHwInfo.txPin  ;				
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  
  GPIO_Init(g_uartHwInfo.gpio  ,&GPIO_InitStructure);
  /*Configure USART Rx as input floating*/
  GPIO_InitStructure.GPIO_Pin = g_uartHwInfo.rxPin   ;				
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	  
  GPIO_Init(g_uartHwInfo.gpio  ,&GPIO_InitStructure);
}

static void SurgeUartInit(uint32_t baudRate)
{
    /* 使能时钟 */
  RCC_APB1PeriphClockCmd( g_uartHwInfo.rcuUart  , ENABLE);
  
	USART_InitTypeDef USART_InitStructure;	
  
  /* 复位UART */
  USART_DeInit(g_uartHwInfo.uartNo );
	/* 配置串口的工作参数 */
	/* 配置波特率  */
	USART_InitStructure.USART_BaudRate = baudRate;
	/* 配置 针数据字长 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	/* 配置停止位  */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	/* 配置校验位  */
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	/* 配置硬件流控制  */
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* 配置工作模式，只收  */
	USART_InitStructure.USART_Mode = USART_Mode_Rx;
	/* 完成串口的初始化配置  */
	USART_Init(g_uartHwInfo.uartNo , &USART_InitStructure);
  
  /* 启用或禁用指定的USART中断   空闲中断 */
  USART_ITConfig(g_uartHwInfo.uartNo ,USART_IT_IDLE ,ENABLE );

  /*  使能串口中断 */
  NVIC_InitTypeDef NVIC_InitStructure; 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	 
	NVIC_InitStructure.NVIC_IRQChannel = g_uartHwInfo.irq ;			   	 //设置串口2中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	       //抢占优先级 0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				       //子优先级为0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					         //使能
  NVIC_Init(&NVIC_InitStructure);    
  /* 使能串口 */
	USART_Cmd(g_uartHwInfo.uartNo , ENABLE);	
}


	//开始一次 DMA 传输！ 
static void MX_DMA_Enable(DMA_Channel_TypeDef* DMA_CHx)
{
	DMA_Cmd(DMA_CHx, DISABLE ); //关闭 DMA1 所指示的通道 
  
	DMA_SetCurrDataCounter(DMA_CHx,MAX_BUF_SIZE);//设置 DMA 缓存的大小
  
	DMA_Cmd(DMA_CHx, ENABLE); //使能 DMA1 所指示的通道
}

static void SurgeDmaInit(void )
{
  DMA_InitTypeDef  DMA_InitStructure;

  RCC_AHBPeriphClockCmd(g_uartHwInfo.rcuDma , ENABLE);	//使能DMA传输

  DMA_DeInit(g_uartHwInfo.dmaCh );  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;//DMA外设地址  
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_recvDataBuf;//DMA 存储器0地址  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC ;//外设到存储器模式  
  DMA_InitStructure.DMA_BufferSize = MAX_BUF_SIZE;//数据传输量   
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式  
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式  
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位  
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位  
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式   
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级  
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
  DMA_Init(g_uartHwInfo.dmaCh , &DMA_InitStructure);//初始化DMA Stream  

  /* 使能UART接收数据使用DMA */ 
	USART_DMACmd(g_uartHwInfo.uartNo, USART_DMAReq_Rx, ENABLE);
  /* 使能DMA通道；*/ 
  MX_DMA_Enable(g_uartHwInfo.dmaCh );
}  

/**
***********************************************************
* @brief 长沙雷击传感器模块转串口硬件初始化
* @param
* @return 
***********************************************************
*/
void SurgeMdlDrvInit(void)
{
  SurgeGpioInit();
  SurgeUartInit(115200);
  SurgeDmaInit();
}

/**
***********************************************************
* @brief 串口2中断服务函数
* @param
* @return 
***********************************************************
*/
void USART2_IRQHandler(void)
{
    if (RESET != USART_GetITStatus(g_uartHwInfo.uartNo , USART_IT_IDLE))
    {
      USART_ClearITPendingBit(g_uartHwInfo.uartNo, USART_IT_IDLE);  //第一步，读取stat0寄存器，清除IDLE标志位
      
      USART_ReceiveData(g_uartHwInfo.uartNo );                      //第二步，读取数据寄存器，清除IDLE标志位
      
      g_recvDataLen = MAX_BUF_SIZE - DMA_GetCurrDataCounter( g_uartHwInfo.dmaCh);
      
      memset(&g_recvDataBuf[g_recvDataLen], 0, MAX_BUF_SIZE - g_recvDataLen);
       /* 使能DMA通道；*/ 
      MX_DMA_Enable(g_uartHwInfo.dmaCh );
    }
}

/**
********************************************************************************
    第1字节  第2~3字节       第4~5字节         第6~9字节     第10~13字节  第14字节
    帧头     正极性H 正极性L 负极性H  负极性L  正极性累加值  负极性累加值 帧尾
    0xAA     0x01    0x26    0x01     0x26     0xFFFFFFFF    0xFFFFFFFF   0xBB
********************************************************************************
*/
#define FRAME_HEAD_0     0xAA  
#define FRAME_TAIL_0     0xBB  

static SurgemdlParam_t g_SurgeParam;
static bool g_refresh = false ;

void  SurgemdlDataPro(void)
{
  static uint32_t Positive_Peak ,Negative_Peak;
  
	if( *g_recvDataBuf != FRAME_HEAD_0)
  {
   g_refresh = false ; 
   return ;
  }
  else if (*g_recvDataBuf == FRAME_HEAD_0)
  {
    g_refresh = true ;
    
    g_SurgeParam.Num ++;
    
    if( g_SurgeParam.Num > Save_MAX)
    {
      g_SurgeParam.Num = 1 ;
    }    
    
    Positive_Peak = g_recvDataBuf[1]*256+g_recvDataBuf[2];
    
    Negative_Peak = g_recvDataBuf[3]*256+g_recvDataBuf[4];    
    
    g_SurgeParam.PeakCurrent =( Positive_Peak > Negative_Peak )? Positive_Peak:Negative_Peak;      
    
    *g_recvDataBuf = 0;
  }
}

bool GetRefreshflag(void )
{
 return g_refresh;
}  

/***********************************************************
* @brief 预设浪涌次数
* @param
* @return 
***********************************************************
*/
void PresetNum_Surge(uint16_t data)
{
   g_SurgeParam.Num  =  data ; 
}

bool GetSurgemdlData(SurgemdlParam_t*  SurgemdlPara)
{
  SurgemdlPara->Num = g_SurgeParam.Num;
  
  SurgemdlPara->PeakCurrent =  g_SurgeParam.PeakCurrent ;
  
  if(  SurgemdlPara->Num > Save_MAX)
  { 
   return false;
  }
  else 
  {
   return true;  
  }
}

void SurgemdlFuncTest(void )
{
  SurgemdlParam_t data;
  
  GetSurgemdlData (&data);
  
  DBG_log("surgenum is %d\n", data.Num );
  
  DBG_log("surge original peakcurrent is %d\n", data.PeakCurrent  );
  
}



