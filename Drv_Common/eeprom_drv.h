#ifndef __EEPROM_DRV_H
#define	__EEPROM_DRV_H

#include <stdint.h>
#include <stdbool.h>

typedef enum 
{
  WRITE_ERR,
  READ_ERR,
  CRC_ERR
}I2C_ERR;

void I2CSendAck(void);
void I2CSendNack(void);
void EepromDrvInit ( void );
bool ReadEepromData(uint8_t readAddr, uint8_t *pBuffer, uint16_t numToRead);
bool WriteEepromData(uint8_t writeAddr, uint8_t *pBuffer, uint16_t numToWrite);
void EepromDrvTest(void);

void EepromErase(void );
uint8_t I2c_CheckDevice(void );


#endif

