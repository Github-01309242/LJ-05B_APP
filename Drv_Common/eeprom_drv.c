
#include "stm32f10x.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "delay.h"
#include "eeprom_drv.h"

#define GET_I2C_SDA()             GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11)	/* ��SDA����״̬ */
#define SET_I2C_SCL()             GPIO_SetBits(GPIOC, GPIO_Pin_10)          // ʱ����SCL����ߵ�ƽ
#define CLR_I2C_SCL()             GPIO_ResetBits(GPIOC, GPIO_Pin_10)        // ʱ����SCL����͵�ƽ
#define SET_I2C_SDA()             GPIO_SetBits(GPIOC, GPIO_Pin_11)          // ������SDA����ߵ�ƽ
#define CLR_I2C_SDA()             GPIO_ResetBits(GPIOC, GPIO_Pin_11)        // ������SDA����͵�ƽ


static void Eeprom_GPIO_Config(void )
{
	GPIO_InitTypeDef GPIO_InitStructure; 	
	/*��������ʱ��*/
  RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );	
	/*ѡ������*/															   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;	
	/*��������ģʽΪͨ�ÿ�©���*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;   
	/*������������ */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	/*���ÿ⺯������ʼ��GPIO_PORT*/
  GPIO_Init ( GPIOC, &GPIO_InitStructure );	
}


/**
*******************************************************************
* @function ����IIC��ʼʱ��׼�����ͻ��������ǰ��������ʼ���п�ʼ 
* @param
* @return 
* @brief    SCLΪ�ߵ�ƽʱ��SDA�ɸߵ�ƽ��͵�ƽ���䣬��ʼ�������� 
*           ������ͼ��ʾ�Ĳ���ͼ����Ϊ��ʼʱ�� 
*                1 2    3     4   
*                    __________     
*           SCL : __/          \_____ 
*                 ________          
*           SDA :         \___________ 
*******************************************************************
*/
static void I2CStart(void)
{
	SET_I2C_SDA();          // 1#������SDA����ߵ�ƽ
	SET_I2C_SCL();          // 2#ʱ����SCL����ߵ�ƽ   
	DelayNus(4);            // ��ʱ4us
	CLR_I2C_SDA();          // 3#������SDA����͵�ƽ 
	DelayNus(4);            // ��ʱ4us
	CLR_I2C_SCL();          // 4#ʱ����SCL����͵�ƽ������I2C��ʱ����SCLΪ�͵�ƽ��׼�����ͻ�������� 
	DelayNus(4);            // ��ʱ4us
}

/**
*******************************************************************
* @function ����IICֹͣʱ��  
* @param
* @return 
* @brief    SCLΪ�ߵ�ƽʱ��SDA�ɵ͵�ƽ��ߵ�ƽ���䣬������������ 
*          ������ͼ��ʾ�Ĳ���ͼ����Ϊֹͣʱ�� 
*                1 2   3  4   
*                       _______________     
*          SCL : ______/          
*                __        ____________  
*          SDA:    \______/
*******************************************************************
*/
static void I2CStop(void)
{
	CLR_I2C_SDA();          //2#������SDA����͵�ƽ
	DelayNus(4);            //��ʱ4us
	SET_I2C_SCL();          //3#ʱ����SCL����ߵ�ƽ
	DelayNus(4);  
	SET_I2C_SDA();          //4#������SDA����ߵ�ƽ������I2C���߽����ź�
}

/**
*******************************************************************
* @function ����һ�ֽڣ����ݴӸ�λ��ʼ���ͳ�ȥ
* @param    byte
* @return 
* @brief    �����Ǿ����ʱ��ͼ 
*                1 2     3      4
*                         ______
*           SCL: ________/      \______    
*                ______________________    
*           SDA: \\\___________________
*******************************************************************
*/
static void I2CSendByte(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)   // ѭ��8�Σ��Ӹߵ���ȡ���ֽڵ�8��λ
	{     
		if ((byte & 0x80))            // 2#ȡ���ֽ����λ�����ж�Ϊ��0�����ǡ�1�����Ӷ�������Ӧ�Ĳ���
		{
			SET_I2C_SDA();            // ������SDA����ߵ�ƽ������λΪ��1��
		}
		else
		{  
			CLR_I2C_SDA();      	  // ������SDA����͵�ƽ������λΪ��0��
		}
		
		byte <<= 1;            		  // ����һλ���θ�λ�Ƶ����λ
		
		DelayNus(4);          		  // ��ʱ4us
		SET_I2C_SCL();                // 3#ʱ����SCL����ߵ�ƽ
		DelayNus(4);          		  // ��ʱ4us
		CLR_I2C_SCL();        		  // 4#ʱ����SCL����͵�ƽ
		DelayNus(4);                  // ��ʱ4us  
	} 
} 

/**
*******************************************************************
* @function ��ȡһ�ֽ�����
* @param    
* @return   ��ȡ���ֽ�
* @brief    �����Ǿ����ʱ��ͼ
*                       ______
*           SCL: ______/      \___        
*                ____________________    
*           SDA: \\\\______________\\\
*******************************************************************
*/
static uint8_t I2CReadByte(void)
{
	uint8_t byte = 0;           		// byte������Ž��յ�����
	SET_I2C_SDA();                      // �ͷ�SDA
	for (uint8_t i = 0; i < 8; i++)     // ѭ��8�Σ��Ӹߵ��Ͷ�ȡ�ֽڵ�8��λ
	{
		SET_I2C_SCL();          		// ʱ����SCL����ߵ�ƽ
		DelayNus(4);            		// ��ʱ4us
		byte <<= 1;          			// ����һλ���ճ��µ����λ

		if (GET_I2C_SDA())       		// ��ȡ������SDA������λ
		{
			byte++;            			// ��SCL�������غ������Ѿ��ȶ�����˿���ȡ�����ݣ��������λ
		}
		CLR_I2C_SCL();          		// ʱ����SCL����͵�ƽ
		DelayNus(4);            		// ��ʱ4us
	} 

	return byte;           				// ���ض�ȡ��������
}


/**
*******************************************************************
* @function �ȴ����ն˵�Ӧ���ź�
* @param    
* @return   0������Ӧ��ʧ�ܣ�1������Ӧ��ɹ�
* @brief    ��SDA���ͺ󣬱�ʾ���յ�ACK�źţ�Ȼ������SCL��
*           �˴���ʾ���Ͷ��յ����ն˵�ACK
*                _______|____     
*           SCL:        |    \_________    
*                _______|     
*           SDA:         \_____________ 
*******************************************************************
*/
static bool I2CWaitAck(void)
{
	uint8_t errTimes = 0;
	
	SET_I2C_SDA();             // �ͷ�SDA����,����Ҫ
	DelayNus(4);               // ��ʱ4us
	
	SET_I2C_SCL();             // ʱ����SCL����ߵ�ƽ
	DelayNus(4);               // ��ʱ4us

	while (GET_I2C_SDA())      // ����������������Ǹߵ�ƽ�������ն�û��Ӧ��
	{
		errTimes++;            // ��������1

		if (errTimes > 250)    // �������250�Σ����ж�Ϊ���ն˳��ֹ��ϣ���˷��ͽ����ź�
		{
			I2CStop();         // ����һ��ֹͣ�ź�
			return false;      // ����ֵΪ1����ʾû���յ�Ӧ���ź�
		}
	}

	CLR_I2C_SCL();             // ��ʾ���յ�Ӧ���źţ�ʱ����SCL����͵�ƽ
	DelayNus(4);               // ��ʱ4us
	
	return true;               // ����ֵΪ0����ʾ����Ӧ��ɹ�  
}

/**
*******************************************************************
* @function ����Ӧ���ź�
* @param    
* @return   
* @brief    �����Ǿ����ʱ��ͼ 
*                 1 2     3      4      5     
*                         ______
*           SCL: ________/      \____________    
*                __                     ______
*           SDA:   \___________________/        
*******************************************************************
*/
void I2CSendAck(void)
{
  CLR_I2C_SDA();          // 2#������SDA����͵�ƽ
	DelayNus(4);            // ��ʱ4us
	SET_I2C_SCL();          // 3#ʱ����SCL����ߵ�ƽ,��SCL������ǰ��Ҫ��SDA���ͣ�ΪӦ���ź�
	DelayNus(4);            // ��ʱ4us
	CLR_I2C_SCL();          // 4#ʱ����SCL����͵�ƽ
	DelayNus(4);            // ��ʱ4us
	SET_I2C_SDA();          // 5#������SDA����ߵ�ƽ���ͷ�SDA����,����Ҫ
}  

/**
*******************************************************************
* @function ���ͷ�Ӧ���ź�
* @param    
* @return   
* @brief    �����Ǿ����ʱ��ͼ 
*               1 2     3      4
*                        ______
*          SCL: ________/      \______    
*               __ ___________________    
*          SDA: __/
*******************************************************************
*/
void I2CSendNack(void)
{
	SET_I2C_SDA();          // 2#������SDA����ߵ�ƽ
	DelayNus(4);            // ��ʱ4us
	SET_I2C_SCL();          // 3#ʱ����SCL����ߵ�ƽ����SCL������ǰ��Ҫ��SDA���ߣ�Ϊ��Ӧ���ź�
	DelayNus(4);            // ��ʱ4us
	CLR_I2C_SCL();          // 4#ʱ����SCL����͵�ƽ
	DelayNus(4);            // ��ʱ4us
}

#define EEPROM_DEV_ADDR			0xA0		// 24xx02���豸��ַ
#define EEPROM_PAGE_SIZE	  8			  // 24xx02��ҳ���С
#define EEPROM_SIZE				  256	    // 24xx02������
#define EEPROM_I2C_WR			  0		    // д����bit
#define EEPROM_I2C_RD	      1		    // ������bit

/**
*******************************************************************
* @function ָ����ַ��ʼ����ָ������������
* @param    readAddr,��ȡ��ַ��0~255
* @param    pBuffer,�����׵�ַ
* @param    numToRead,Ҫ���������ݸ���,������256
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
* @function ָ����ַ��ʼд��ָ������������
* @param    writeAddr,д���ַ��0~255
* @param    pBuffer,�����׵�ַ
* @param    numToWrite,Ҫд������ݸ���,������256
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
		д����EEPROM�������������������ȡ�ܶ��ֽڣ�ÿ��д����ֻ����ͬһ��page��
		����24c02��page size = 8
		�򵥵Ĵ�������Ϊ��Ϊ���������д��Ч�ʣ����������ð�ҳд������������ҳ���·��͵�ַ��
	*/
  for(i=0; i<numToWrite; i++)
  {
    if( i==0 || (dataAddr & EEPROM_PAGE_SIZE-1) ==0)
    {
      I2CStop();
			/* ͨ���������Ӧ��ķ�ʽ���ж��ڲ�д�����Ƿ���� */ 			
			for ( j = 0; j < 100; j++)
			{				
				/* ��1��������I2C���������ź� */
				I2CStart();
				
				/* ��2���������豸��ַ�Ϳ���λ */
				I2CSendByte(EEPROM_DEV_ADDR | EEPROM_I2C_WR);	// �˴���дָ��
				
				/* ��3��������һ��ʱ�ӣ��ж������Ƿ���ȷӦ�� */
				if (I2CWaitAck())
				{
					break;
				}
			}
			if (j  == 100)
			{
				goto i2c_err;	// EEPROM����д��ʱ
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
  * @brief  eeprom ��ʼ������
  * @param  ��
  * @retval ��
  */
void EepromDrvInit ( void )
{
	Eeprom_GPIO_Config ();	
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_CheckDevice
*	����˵��: ���I2C�����豸��CPU�����豸��ַ��Ȼ���ȡ�豸Ӧ�����жϸ��豸�Ƿ����
*	��    �Σ�_Address���豸��I2C���ߵ�ַ
*	�� �� ֵ: ����ֵ 1 ��ʾ��ȷ�� ����0��ʾδ̽�⵽
*********************************************************************************************************
*/
uint8_t I2c_CheckDevice(void )
{
	uint8_t ucAck;
	
	I2CStart();  	/* ���������ź� */

	/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
	I2CSendByte(EEPROM_DEV_ADDR | EEPROM_I2C_WR);
	ucAck = I2CWaitAck();	/* ����豸��ACKӦ�� */

	I2CStop ();			/* ����ֹͣ�ź� */

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




