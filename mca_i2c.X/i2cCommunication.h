#ifndef I2CCOMMUNICATION_H
#define I2CCOMMUNICATION_H

#include <stdint.h>

void initI2C2();
void processI2C2();
void i2c2InitSlave(uint8_t address);
void i2c2ReadData();
void i2c2WriteData();
void i2c2InterruptHandler();
void clearI2C2Interrupt();

#endif
