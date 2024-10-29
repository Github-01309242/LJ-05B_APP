/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "stm32f10x.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

typedef struct
{
	USART_TypeDef* uartNo;
	GPIO_TypeDef* gpio;
	uint32_t txPin;
	uint32_t rxPin;
	uint8_t irq;
} UartHwInfo_t;

static UartHwInfo_t g_uartHwInfo = {USART1, GPIOA, GPIO_Pin_9, GPIO_Pin_10, USART1_IRQn};

static void GpioInit(void )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  // 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

 /**
  * @brief  RS485_USART 配置,工作参数配置
  * @param  无
  * @retval 无
  */
static void UartInit(uint32_t baudRate)
{
  USART_InitTypeDef USART_InitStructure;	
  /*  使能时钟 */
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1  , ENABLE);
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
	/* 配置工作模式，收发一起  */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* 完成串口的初始化配置  */
	USART_Init(USART1, &USART_InitStructure);

  /*  使能串口中断 */
  NVIC_InitTypeDef NVIC_InitStructure; 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			   	 //设置串口1中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	 //抢占优先级 0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				 //子优先级为0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					   //使能
  NVIC_Init(&NVIC_InitStructure);  
  /* 使能串口 */
	USART_Cmd(USART1, ENABLE);	
	
}

static void SwitchInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure; 
  // 打开串口GPIO的时钟
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
   /* 推挽输出 */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	  
   GPIO_Init(GPIOA ,&GPIO_InitStructure);                               
}

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	 if(TRUE == xRxEnable)
    {
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//使能接收中断
		GPIO_ResetBits(GPIOA, GPIO_Pin_8 ); 
    }
    else
    {
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//失能接收中断
		GPIO_SetBits(GPIOA, GPIO_Pin_8 );    
    }
    
   if(TRUE == xTxEnable)
    {
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);//使能发送中断
		GPIO_SetBits(GPIOA, GPIO_Pin_8 );   
    }
    else
    {
    USART_ITConfig(USART1, USART_IT_TC, DISABLE);//失能发送中断
		GPIO_ResetBits(GPIOA, GPIO_Pin_8 );   
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    (void)ucPORT;
    (void)ucDataBits;
    (void)eParity;
    SwitchInit();
    GpioInit();
    UartInit(ulBaudRate);
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    USART_SendData(g_uartHwInfo.uartNo , ucByte);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = USART_ReceiveData(g_uartHwInfo.uartNo );
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART1_IRQHandler(void)
{
    //判断接收还是发送中断
    if (RESET != USART_GetITStatus(g_uartHwInfo.uartNo , USART_IT_RXNE))
    {
        //接收完成中断
        prvvUARTRxISR();
        USART_ClearITPendingBit(g_uartHwInfo.uartNo, USART_IT_RXNE);
    }
    if(RESET != USART_GetITStatus(g_uartHwInfo.uartNo, USART_IT_TC))
    {
        //发送完成中断
        prvvUARTTxReadyISR();
        USART_ClearITPendingBit(g_uartHwInfo.uartNo, USART_IT_TC);
    }
}
