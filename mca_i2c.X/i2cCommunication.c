#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "i2cCommunication.h"  // Ensure this is included

#define I2C2_ADDRESS 0x20

char command[100];  // Declare command buffer

void processI2C2() {
    if (IFS1bits.I2C2BIF) {
        i2c2InterruptHandler();  // Handle I2C2 communication
        clearI2C2Interrupt();     // Clear the interrupt flag
    }
}

void i2c2InitSlave(uint8_t address) {
    I2C2BRG = 0x0076;         // Set baud rate for I2C2
    I2C2CONbits.I2CEN = 1;  // Enable I2C2
    I2C2CONbits.SCLREL = 1; // Release the clock
    I2C2ADD = address;      // Set slave address
    I2C2CONbits.DISSLW = 1; // Disable slew rate control
    IFS1bits.I2C2BIF = 0;   // Clear the interrupt flag
    IEC1bits.I2C2BIE = 1;   // Enable I2C2 Master Interrupt
}

void initI2C2() {
    I2C2CONbits.I2CEN = 1;   // Enable I2C2 module
    I2C2BRG = 0x27;          // Set the baud rate generator for I2C2
    i2c2InitSlave(I2C2_ADDRESS); 
}

void i2c2InterruptHandler() {
    if (I2C2STATbits.R_W == 0) {
        i2c2ReadData();  
    } else {
        i2c2WriteData();
    }
}

void i2c2ReadData() {
    char receivedData[100];
    uint8_t i = 0;

    while (I2C2STATbits.RBF) {
        receivedData[i++] = I2C2RCV;  // Store received data
    }
    receivedData[i] = '\0';  // Null-terminate the string
    strncpy(command, receivedData, sizeof(command) - 1); // Copy to command buffer
}

void i2c2WriteData() {
    uint8_t dataToSend = 0x55;
    I2C2TRN = dataToSend;
    while (I2C2STATbits.TRSTAT);
}

void clearI2C2Interrupt() {
    IFS1bits.I2C2BIF = 0;  
}
