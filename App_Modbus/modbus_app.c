#include <stdint.h>
#include <stdlib.h>
#include "modbus_slave.h"
#include "mb.h"
#include "mbcrc.h"
#include "sensor_drv.h"
#include "led_drv.h"
#include "switch_drv.h"
#include "norflash_drv.h"
#include "store_app.h"
#include "stm32f10x.h"

/**
***********************************************************
* @brief ���ּĴ�����ؾ��
* @param
* @return 
***********************************************************
*/
#define R   (1 << 0)
#define W   (1 << 1)

typedef struct {
    uint8_t  property;				// ��д������
    const uint16_t address;		// ��ַ
    uint16_t minValue;				// ��Сֵ
    uint16_t maxValue;				// ���ֵ
    void (*ReadCb)(uint16_t *value);
    void (*WriteCb)(uint16_t value); 
} MbRegisterInstance_t;

/**
***********************************************************
* @brief ��ɢ������ؾ��
* @param
* @return 
***********************************************************
*/
typedef struct {    
    const uint16_t address;		// ��ַ    
    void (*ReadDiscreteInputsCb)(uint16_t *value , USHORT  , USHORT );  

} MbRegDiscreteInstance_t;


static void ModbusGetTemp(uint16_t *value);
static void ModbusGetHumi(uint16_t *value);
static void ModbusGetPhaseVol(uint16_t *value);
static void ModbusGetLightNum(uint16_t *value);

static void ModbusSetLed1(uint16_t value);
static void ModbusSetLed2(uint16_t value);

static void ModbusSetRtcTime_Pre(uint16_t value);
static void ModbusSetRtcTime(uint16_t value);

static void ModbusGetDispersed(uint16_t *value , USHORT  , USHORT);


static MbRegisterInstance_t g_regInstanceTab[] = {
    {
        .property = R,
        .address = 0x0000,          // �¶� 01 03 00 00 00 02
		.ReadCb = ModbusGetTemp,
    },
    {
        .property = R,
        .address = 0x0001,         
    .ReadCb = ModbusGetLightNum ,   // ��ӿ����
    },
    {
        .property = R,
        .address = 0x0002,          // ʪ��
		.ReadCb = ModbusGetHumi,
    },
    {
        .property = R,
        .address = 0x0003,          // ���ѹ
		.ReadCb = ModbusGetPhaseVol,
    },

    
    {
        .property = R | W,
        .address = 0x000A,          // LED1���� 01 06 00 02 00 01  ,LED1 LED2 01 10 00 02 00 02 04 00 01 00 01
        .minValue = 0,
        .maxValue = 1,
		.WriteCb = ModbusSetLed1,
    },
    {
        .property = R | W,
        .address = 0x000B,          // LED2���� 01 06 00 03 00 01
        .minValue = 0,
        .maxValue = 1,
		.WriteCb = ModbusSetLed2,
    },
    
    {
        .property = W,
        .address = 0x002C,          // 300
        .minValue = 0,
        .maxValue = 65535,
		.WriteCb = ModbusSetRtcTime_Pre,
    },
    {
        .property = W,
        .address = 0x002D,          // 301
        .minValue = 0,
        .maxValue = 65535,
		.WriteCb = ModbusSetRtcTime,
    },
   
};
#define REG_TABLE_SIZE         (sizeof(g_regInstanceTab) / sizeof(g_regInstanceTab[0]))


static MbRegDiscreteInstance_t g_regdiscreteInstanceTab[] = {
    {        
      .address = 0x0100,        
      .ReadDiscreteInputsCb = ModbusGetDispersed
    },  
};
#define REGDISCRETE_TABLE_SIZE         (sizeof(g_regdiscreteInstanceTab) / sizeof(g_regdiscreteInstanceTab[0]))


static eMBErrorCode ReadRegsCb(uint8_t startAddr, uint8_t regNum, uint8_t *buf)
{
    if (buf == NULL)
    {
          return MB_EINVAL;
    }
    
    for (uint32_t i = 0; i < regNum; i++)
    {
      MbRegisterInstance_t *instance = NULL;
      for (uint32_t j = 0; j < REG_TABLE_SIZE; j++)
      {
        if (g_regInstanceTab[j].address != startAddr + i)
        {
          continue;
        }
        instance = &g_regInstanceTab[j];
        if((instance->property & R) == 0)  //��д����
        {
          return MB_EINVAL;
        }

        if (instance->ReadCb != NULL)
        {
          instance->ReadCb((uint16_t *)&buf[2 * i]);
        }
      }
      if (instance == NULL)
      {
        return MB_ENOREG;
      }		
    }
   return MB_ENOERR;
}
  
static eMBErrorCode WriteRegsCb(uint8_t startAddr, uint8_t regNum, uint8_t *buf)
{
  if (buf == NULL)
	{
        return MB_EINVAL;
	}
	
  for (uint32_t i = 0; i < regNum; i++)
	{
		
		MbRegisterInstance_t *instance = NULL;
		for (uint32_t j = 0; j < REG_TABLE_SIZE; j++)
		{
			if (g_regInstanceTab[j].address != startAddr + i)
			{
				continue;
			}
			instance = &g_regInstanceTab[j];
			
		    if((instance->property & W) == 0)  //��д����
			{
				return MB_EINVAL;
			}
			
			uint16_t setValue = ((buf[2 * i] << 8) & 0xFF00) | (buf[2 * i + 1] & 0xFF);
			if ((setValue < instance->minValue) || (setValue > instance->maxValue))
			{
				return MB_EINVAL;
			}
			
			if (instance->WriteCb != NULL)
			{
				instance->WriteCb(setValue);
			}
		}
		
		if (instance == NULL)
		{
			return MB_ENOREG;
		}
	}
  return MB_ENOERR;
}


#define REG_DISCRETE_START    0x0100    //���ؼĴ�����ʵ��ַ
#define REG_DISCRETE_SIZE     2         //���ؼĴ�������

static eMBErrorCode ReadDiscreteInputsCb( uint8_t *buf ,USHORT usAddress, USHORT usNDiscrete)
{
    if (buf == NULL)
    {
          return MB_EINVAL;
    }
    
    MbRegDiscreteInstance_t *instance = NULL;
    instance = &g_regdiscreteInstanceTab[REGDISCRETE_TABLE_SIZE-1];
    
    int  iRegIndex;      
    iRegIndex = (int)(usAddress + usNDiscrete - REG_DISCRETE_START);
    
    if((usAddress >= REG_DISCRETE_START)&&\
    ((usAddress+usNDiscrete) <= (REG_DISCRETE_START + REG_DISCRETE_SIZE)))
    {
      if (instance->ReadDiscreteInputsCb  != NULL)
			{       
       instance->ReadDiscreteInputsCb((uint16_t *)buf , usNDiscrete , iRegIndex);
      }
      if (instance == NULL)
      {
        return MB_ENOREG;
      }
    }
    else
    {
        return MB_ENOREG;
    }
   return MB_ENOERR;

}


static void ModbusGetTemp(uint16_t *value)
{
	SensorData_t sensorData;
	GetSensorData(&sensorData);
	*value = (uint16_t)(sensorData.temp *10 );
}

static void ModbusGetHumi(uint16_t *value)
{
	SensorData_t sensorData;
	GetSensorData(&sensorData);
	*value = (uint16_t)(sensorData.humi * 10);
}

static void ModbusGetPhaseVol(uint16_t *value)
{
	SensorData_t sensorData;
	GetSensorData(&sensorData);
	*value = (uint16_t)(sensorData.Voltage_A * 10);
}

static void ModbusGetLightNum(uint16_t *value)
{
	SensorData_t sensorData;
	GetSensorData(&sensorData);
	*value = sensorData.SurgemdlParam .Num   ;  
}


static void ModbusSetLed1(uint16_t value)
{
	value == 0 ? TurnOffLed(LED1) : TurnOnLed(LED1);
}

static void ModbusSetLed2(uint16_t value)
{
	value == 0 ? TurnOffLed(LED2) : TurnOnLed(LED2);
}

static uint32_t Time_Stamp = 0 ;
static void ModbusSetRtcTime_Pre(uint16_t value)
{
	Time_Stamp = (value << 16 ) & 0xFFFF0000 ;
}

static void ModbusSetRtcTime(uint16_t value)
{
	Time_Stamp  |= (value & 0x0000FFFF );
  
  RTC_EnterConfigMode();                    // ��������	
  RTC_SetPrescaler(32767);                  //����RTCԤ��Ƶ��ֵ
  RTC_WaitForLastTask();	                  //�ȴ����һ�ζ�RTC�Ĵ�����д�������
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
  PWR_BackupAccessCmd(ENABLE);	            //ʹ��RTC�ͺ󱸼Ĵ������� 
  RTC_SetCounter(Time_Stamp);	              //����RTC��������ֵ
  RTC_WaitForLastTask();	                 //�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
  /* �˳�����ģʽ */
  RTC_ExitConfigMode();  
}

//ģ����ɢ�Ĵ�������
//uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE] = {0x01,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x01}; 

static void ModbusGetDispersed(uint16_t *value ,USHORT usNDiscrete ,USHORT  Index)
{
	USHORT coilValue=0x0000;
  USHORT readNumber = usNDiscrete;
  
  uint8_t* STA = GetSwitchinputStatus();   
 
  for(uint8_t i = 0; i < usNDiscrete; i++)
  {
      readNumber--;
      Index--;
      coilValue|= STA[Index]<<readNumber;
  }

  * value = coilValue;
}


#define PacketNum_HEAD   2
#define PacketNum_MAX   100
#define NUM_MAX_SURGE   11
#define NUM_MAX_SEND    ( 4 + NUM_MAX_SURGE - 2 + 2 )
static uint8_t bufferRead[NUM_MAX_SURGE];
static uint8_t bufferSend[20];

static void CustomFuncHandlerCb(UCHAR *pucRegBuffer )
{
  uint16_t PacketNum;
  
  PacketNum = pucRegBuffer[PacketNum_HEAD]*256 + pucRegBuffer[PacketNum_HEAD+1];
  if( PacketNum == 0 || PacketNum > PacketNum_MAX)
  {
   return ;
  }
  ReadNorflashData ((uint32_t )((PacketNum-1)*4096),bufferRead,NUM_MAX_SURGE);   //��norflash�ж�ȡ�ڼ����׵���ص���Ϣ����
  
  bufferSend[0] = pucRegBuffer[0];
  bufferSend[1] = pucRegBuffer[1];
  bufferSend[2] = PacketNum;
  bufferSend[3] = NUM_MAX_SURGE -2 ;
  for( uint8_t i = 0;i < (NUM_MAX_SURGE-2) ; i++)
  {
  bufferSend[4+i] = bufferRead[i+2];
  }

  uint16_t CRCHL = usMBCRC16( ( UCHAR * ) bufferSend, NUM_MAX_SEND-2 );
  bufferSend[NUM_MAX_SEND-2] = CRCHL | 0X00;                      //���ֽ���ǰ
  bufferSend[NUM_MAX_SEND-1] = ( CRCHL >> 8 ) | 0X00;             //���ֽ��ں�
  
  for( uint8_t i=0; i < NUM_MAX_SEND; i++)
  {
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_SendData( USART1 , *( bufferSend + i ) );
    /*�ȴ��������*/
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ;
  }
  
}


void ModbusAppInit(void)
{
	ModbusSlaveInstance_t mbInstace = {
        .baudRate = GetModbusBaudData(),
        .cb.ReadRegs = ReadRegsCb,
        .cb.WriteRegs = WriteRegsCb,
        .cb.ReadDiscreteInputs = ReadDiscreteInputsCb,
        .slaveAddr = GetModbusAddrData(),
    };
	ModbusSlaveInit(&mbInstace);
  CustomFuncHandlerCbReg (CustomFuncHandlerCb );
}

void ModbusTask(void)
{
	(void)eMBPoll();
}


