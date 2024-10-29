
#include "stm32f10x.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "delay.h"
#include "eeprom_drv.h"

#define GET_I2C_SDA()             GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11)	/* 读SDA口线状态 */
#define SET_I2C_SCL()             GPIO_SetBits(GPIOC, GPIO_Pin_10)          // 时钟线SCL输出高电平
#define CLR_I2C_SCL()             GPIO_ResetBits(GPIOC, GPIO_Pin_10)        // 时钟线SCL输出低电平
#define SET_I2C_SDA()             GPIO_SetBits(GPIOC, GPIO_Pin_11)          // 数据线SDA输出高电平
#define CLR_I2C_SDA()             GPIO_ResetBits(GPIOC, GPIO_Pin_11)        // 数据线SDA输出低电平


static void Eeprom_GPIO_Config(void )
{
	GPIO_InitTypeDef GPIO_InitStructure; 	
	/*开启外设时钟*/
  RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );	
	/*选择引脚*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;	
	/*设置引脚模式为通用开漏输出*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;   
	/*设置引脚速率 */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	/*调用库函数，初始化GPIO_PORT*/
  GPIO_Init ( GPIOC, &GPIO_InitStructure );	
}


/**
*******************************************************************
* @function 产生IIC起始时序，准备发送或接收数据前必须由起始序列开始 
* @param
* @return 
* @brief    SCL为高电平时，SDA由高电平向低电平跳变，开始传输数据 
*           生成下图所示的波形图，即为起始时序 
*                1 2    3     4   
*                    __________     
*           SCL : __/          \_____ 
*                 ________          
*           SDA :         \___________ 
*******************************************************************
*/
static void I2CStart(void)
{
	SET_I2C_SDA();          // 1#数据线SDA输出高电平
	SET_I2C_SCL();          // 2#时钟线SCL输出高电平   
	DelayNus(4);            // 延时4us
	CLR_I2C_SDA();          // 3#数据线SDA输出低电平 
	DelayNus(4);            // 延时4us
	CLR_I2C_SCL();          // 4#时钟线SCL输出低电平，保持I2C的时钟线SCL为低电平，准备发送或接收数据 
	DelayNus(4);            // 延时4us
}

/**
*******************************************************************
* @function 产生IIC停止时序  
* @param
* @return 
* @brief    SCL为高电平时，SDA由低电平向高电平跳变，结束传输数据 
*          生成下图所示的波形图，即为停止时序 
*                1 2   3  4   
*                       _______________     
*          SCL : ______/          
*                __        ____________  
*          SDA:    \______/
*******************************************************************
*/
static void I2CStop(void)
{
	CLR_I2C_SDA();          //2#数据线SDA输出低电平
	DelayNus(4);            //延时4us
	SET_I2C_SCL();          //3#时钟线SCL输出高电平
	DelayNus(4);  
	SET_I2C_SDA();          //4#数据线SDA输出高电平，发送I2C总线结束信号
}

/**
*******************************************************************
* @function 发送一字节，数据从高位开始发送出去
* @param    byte
* @return 
* @brief    下面是具体的时序图 
*                1 2     3      4
*                         ______
*           SCL: ________/      \______    
*                ______________________    
*           SDA: \\\___________________
*******************************************************************
*/
static void I2CSendByte(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)   // 循环8次，从高到低取出字节的8个位
	{     
		if ((byte & 0x80))            // 2#取出字节最高位，并判断为‘0’还是‘1’，从而做出相应的操作
		{
			SET_I2C_SDA();            // 数据线SDA输出高电平，数据位为‘1’
		}
		else
		{  
			CLR_I2C_SDA();      	  // 数据线SDA输出低电平，数据位为‘0’
		}
		
		byte <<= 1;            		  // 左移一位，次高位移到最高位
		
		DelayNus(4);          		  // 延时4us
		SET_I2C_SCL();                // 3#时钟线SCL输出高电平
		DelayNus(4);          		  // 延时4us
		CLR_I2C_SCL();        		  // 4#时钟线SCL输出低电平
		DelayNus(4);                  // 延时4us  
	} 
} 

/**
*******************************************************************
* @function 读取一字节数据
* @param    
* @return   读取的字节
* @brief    下面是具体的时序图
*                       ______
*           SCL: ______/      \___        
*                ____________________    
*           SDA: \\\\______________\\\
*******************************************************************
*/
static uint8_t I2CReadByte(void)
{
	uint8_t byte = 0;           		// byte用来存放接收的数据
	SET_I2C_SDA();                      // 释放SDA
	for (uint8_t i = 0; i < 8; i++)     // 循环8次，从高到低读取字节的8个位
	{
		SET_I2C_SCL();          		// 时钟线SCL输出高电平
		DelayNus(4);            		// 延时4us
		byte <<= 1;          			// 左移一位，空出新的最低位

		if (GET_I2C_SDA())       		// 读取数据线SDA的数据位
		{
			byte++;            			// 在SCL的上升沿后，数据已经稳定，因此可以取该数据，存入最低位
		}
		CLR_I2C_SCL();          		// 时钟线SCL输出低电平
		DelayNus(4);            		// 延时4us
	} 

	return byte;           				// 返回读取到的数据
}


/**
*******************************************************************
* @function 等待接收端的应答信号
* @param    
* @return   0，接收应答失败；1，接收应答成功
* @brief    当SDA拉低后，表示接收到ACK信号，然后，拉低SCL，
*           此处表示发送端收到接收端的ACK
*                _______|____     
*           SCL:        |    \_________    
*                _______|     
*           SDA:         \_____________ 
*******************************************************************
*/
static bool I2CWaitAck(void)
{
	uint8_t errTimes = 0;
	
	SET_I2C_SDA();             // 释放SDA总线,很重要
	DelayNus(4);               // 延时4us
	
	SET_I2C_SCL();             // 时钟线SCL输出高电平
	DelayNus(4);               // 延时4us

	while (GET_I2C_SDA())      // 读回来的数据如果是高电平，即接收端没有应答
	{
		errTimes++;            // 计数器加1

		if (errTimes > 250)    // 如果超过250次，则判断为接收端出现故障，因此发送结束信号
		{
			I2CStop();         // 产生一个停止信号
			return false;      // 返回值为1，表示没有收到应答信号
		}
	}

	CLR_I2C_SCL();             // 表示已收到应答信号，时钟线SCL输出低电平
	DelayNus(4);               // 延时4us
	
	return true;               // 返回值为0，表示接收应答成功  
}

/**
*******************************************************************
* @function 发送应答信号
* @param    
* @return   
* @brief    下面是具体的时序图 
*                 1 2     3      4      5     
*                         ______
*           SCL: ________/      \____________    
*                __                     ______
*           SDA:   \___________________/        
*******************************************************************
*/
void I2CSendAck(void)
{
  CLR_I2C_SDA();          // 2#数据线SDA输出低电平
	DelayNus(4);            // 延时4us
	SET_I2C_SCL();          // 3#时钟线SCL输出高电平,在SCL上升沿前就要把SDA拉低，为应答信号
	DelayNus(4);            // 延时4us
	CLR_I2C_SCL();          // 4#时钟线SCL输出低电平
	DelayNus(4);            // 延时4us
	SET_I2C_SDA();          // 5#数据线SDA输出高电平，释放SDA总线,很重要
}  

/**
*******************************************************************
* @function 发送非应答信号
* @param    
* @return   
* @brief    下面是具体的时序图 
*               1 2     3      4
*                        ______
*          SCL: ________/      \______    
*               __ ___________________    
*          SDA: __/
*******************************************************************
*/
void I2CSendNack(void)
{
	SET_I2C_SDA();          // 2#数据线SDA输出高电平
	DelayNus(4);            // 延时4us
	SET_I2C_SCL();          // 3#时钟线SCL输出高电平，在SCL上升沿前就要把SDA拉高，为非应答信号
	DelayNus(4);            // 延时4us
	CLR_I2C_SCL();          // 4#时钟线SCL输出低电平
	DelayNus(4);            // 延时4us
}

#define EEPROM_DEV_ADDR			0xA0		// 24xx02的设备地址
#define EEPROM_PAGE_SIZE	  8			  // 24xx02的页面大小
#define EEPROM_SIZE				  256	    // 24xx02总容量
#define EEPROM_I2C_WR			  0		    // 写控制bit
#define EEPROM_I2C_RD	      1		    // 读控制bit

/**
*******************************************************************
* @function 指定地址开始读出指定个数的数据
* @param    readAddr,读取地址，0~255
* @param    pBuffer,数组首地址
* @param    numToRead,要读出的数据个数,不大于256
* @return   
*******************************************************************
*/
bool ReadEepromData(uint8_t readAddr, uint8_t *pBuffer, uint16_t numToRead)
{
  if( (readAddr + numToRead ) >  EEPROM_SIZE || pBuffer == NULL)
  {
   return false ;
  }
  I2CStart();
  I2CSendByte ( EEPROM_DEV_ADDR | EEPROM_I2C_WR );
  if( !I2CWaitAck())
  {
    goto i2c_err;
  }
  
  I2CSendByte(readAddr);
  if( !I2CWaitAck())
  {
    goto i2c_err;
  }
  
  I2CStart();
  I2CSendByte ( EEPROM_DEV_ADDR | EEPROM_I2C_RD );
  if( !I2CWaitAck())
  {
    goto i2c_err;
  }
  
  numToRead--;
  while( numToRead-- )
  {
    *pBuffer++ = I2CReadByte();
    I2CSendAck();
  } 
  *pBuffer = I2CReadByte();
  I2CSendNack();
  
  I2CStop ();
  return true ;
 
  i2c_err:
  I2CStop ();
  return false ;
  
}

/**
*******************************************************************
* @function 指定地址开始写入指定个数的数据
* @param    writeAddr,写入地址，0~255
* @param    pBuffer,数组首地址
* @param    numToWrite,要写入的数据个数,不大于256
* @return                                                         
*******************************************************************
*/
bool WriteEepromData(uint8_t writeAddr, uint8_t *pBuffer, uint16_t numToWrite)
{
  if( (writeAddr + numToWrite ) >  EEPROM_SIZE || pBuffer == NULL)
  {
   return false ;
  }
  
  uint16_t i, j;
  uint8_t dataAddr  = writeAddr;
  /* 
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page；
		对于24c02，page size = 8
		简单的处理方法为：为了提高连续写的效率，本函数采用按页写操作，对于新页重新发送地址。
	*/
  for(i=0; i<numToWrite; i++)
  {
    if( i==0 || (dataAddr & EEPROM_PAGE_SIZE-1) ==0)
    {
      I2CStop();
			/* 通过检查器件应答的方式，判断内部写操作是否完成 */ 			
			for ( j = 0; j < 100; j++)
			{				
				/* 第1步：发起I2C总线启动信号 */
				I2CStart();
				
				/* 第2步：发送设备地址和控制位 */
				I2CSendByte(EEPROM_DEV_ADDR | EEPROM_I2C_WR);	// 此处是写指令
				
				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (I2CWaitAck())
				{
					break;
				}
			}
			if (j  == 100)
			{
				goto i2c_err;	// EEPROM器件写超时
			}
      
      I2CSendByte(dataAddr);
      if( !I2CWaitAck())
      {
        goto i2c_err;
      }   
    }

    I2CSendByte(pBuffer[i]);
    if( !I2CWaitAck())
    {
      goto i2c_err;
    }
    dataAddr++;
  }
  
  I2CStop ();
  return true  ;  

  i2c_err:
  I2CStop ();
  return false ;
}



 /**
  * @brief  eeprom 初始化函数
  * @param  无
  * @retval 无
  */
void EepromDrvInit ( void )
{
	Eeprom_GPIO_Config ();	
}

/*
*********************************************************************************************************
*	函 数 名: i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参：_Address：设备的I2C总线地址
*	返 回 值: 返回值 1 表示正确， 返回0表示未探测到
*********************************************************************************************************
*/
uint8_t I2c_CheckDevice(void )
{
	uint8_t ucAck;
	
	I2CStart();  	/* 发送启动信号 */

	/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
	I2CSendByte(EEPROM_DEV_ADDR | EEPROM_I2C_WR);
	ucAck = I2CWaitAck();	/* 检测设备的ACK应答 */

	I2CStop ();			/* 发送停止信号 */

	return ucAck;
}

#define BUFFER_SIZE              256

void EepromDrvTest(void)
{
    uint8_t bufferWrite[BUFFER_SIZE];
    uint8_t bufferRead[BUFFER_SIZE];
    
    for (uint16_t i = 0; i < BUFFER_SIZE; i++)
    { 
      bufferWrite[i]= i + 1;
    }
    if( !WriteEepromData(0,bufferWrite,BUFFER_SIZE))
    {
       return ;
    }
    DelayNms (10);
    if( !ReadEepromData(0,bufferRead,BUFFER_SIZE) )
    {
       return  ;
    }
    for (uint16_t i = 0; i < BUFFER_SIZE; i++)
    {
      if (bufferRead[i] != bufferWrite[i])
      {
       return;
      }
          
    }
}

void EepromErase(void )
{
  uint8_t Empty[256] = {0};
	if (!WriteEepromData(0 , Empty, 256))
	{
		return ;
	}
}





