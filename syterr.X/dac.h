
#ifndef DAC_H    /* Guard against multiple inclusion */
#define DAC_H

#include <stdint.h> 

void initI2C1();
void i2c1Start();
void i2c1Stop();
void i2c1Write(uint8_t data);
uint8_t i2c1ReadAck();
uint8_t i2c1ReadNack();
void mcp4725WriteDAC(uint16_t value);
uint16_t mcp4725ReadDAC();
void writeDACValue(uint16_t value);
uint16_t readDACValue();
uint16_t readVoltage();

#endif