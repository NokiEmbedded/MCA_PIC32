#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "dacCommunication.h"

#define MCP4725_ADDRESS 0x60


void initI2C1() {
    I2C1CONbits.I2CEN = 1;   
    I2C1BRG = 0x27;         
}

void i2c1Start() {
    I2C1CONbits.SEN = 1;  
    while (I2C1CONbits.SEN);
}

void i2c1Stop() {
    I2C1CONbits.PEN = 1;  
    while (I2C1CONbits.PEN);
}

void i2c1Write(uint8_t data) {
    I2C1TRN = data;
    while (I2C1STATbits.TRSTAT);
}

uint8_t i2c1ReadAck() {
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    return I2C1RCV;
}

uint8_t i2c1ReadNack() {
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    return I2C1RCV;
}

void mcp4725WriteDAC(uint16_t value) {
    i2c1Start();
    i2c1Write(MCP4725_ADDRESS << 1);   
    i2c1Write((value >> 4) & 0xFF);   
    i2c1Write((value & 0x0F) << 4);  
    i2c1Stop();
}

uint16_t mcp4725ReadDAC() {
    uint16_t value = 0;
    i2c1Start();
    i2c1Write((MCP4725_ADDRESS << 1) | 1);  
    value = i2c1ReadAck() << 4;  
    value |= i2c1ReadNack() >> 4; 
    i2c1Stop();
    return value;
}

void writeDACValue(uint16_t value) {
    if (value > 4095) value = 4095; 
    mcp4725WriteDAC(value);
}

uint16_t readDACValue() {
    return mcp4725ReadDAC();
}