/*
 * File:   newmainXC16.c
 * Author: Dell
 *
 * Created on October 6, 2024, 12:32 PM
 */


#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#pragma config FNOSC = FRCPLL     // Primary Oscillator: Fast RC with PLL
#pragma config POSCMOD = XT          // External Crystal Oscillator mode (XT, HS, or EC mode)
#pragma config IESO = ON          // Internal/External Oscillator Switchover (optional, for dynamic switching)
#pragma config OSCIOFNC = OFF     // Disable CLKO output, use the pin as GPIO instead of clock output
#pragma config FWDTEN = OFF       // Disable Watchdog Timer

// Function Prototypes
void initSystem();
void initI2C_PPS();
void initUART_PPS();
void initUSB();
void processCommand(char* command);
void sendToUSB(char* message);
void sendToUART(char* message);
void uartTransmit(uint8_t data);
uint8_t uartReceive();
void i2cStart();
void i2cStop();
void i2cWrite(uint8_t data);
uint8_t i2cReadAck();
uint8_t i2cReadNack();
void mcp4725WriteDAC(uint16_t value);
uint16_t mcp4725ReadDAC();
void writeDACValue(uint16_t value);
uint16_t readDACValue();
void initI2C2_PPS();
void processI2C2();
void i2c2InitAsSlave(uint8_t address);
void i2c2ReadData();
void i2c2WriteData();
void i2c2InterruptHandler();
void clearI2C2Interrupt();
void switchToInternalOscillator();
uint16_t readVoltage();
void monitorOscillator();
void watchdogSetup();
void startDataAcquisition();
void stopDataAcquisition();
void resetSystem();

char command[100];
char response[50];

#define MCP4725_ADDRESS 0x60
#define I2C2_ADDRESS 0x20

int main(void) {
    initSystem();  // Initialize the system
    while (1) {
        processI2C2(); // Check for I2C commands
        if (strlen(command) > 0) {
            processCommand(command); // Process the received command
            memset(command, 0, sizeof(command)); // Clear the command buffer
        }
        
        if (!OSCCONbits.COSC) {
            switchToInternalOscillator(); // Switch to internal oscillator if necessary
        }
    }
    return 0;
}

void unlockPPS() {
    SYSKEY = 0xAA996655; // Write Key1
    SYSKEY = 0x556699AA; // Write Key2 to unlock
}

void lockPPS() {
    SYSKEY = 0; // Lock PPS
}

void processI2C2() {
    if (IFS1bits.I2C2BIF) {
        i2c2InterruptHandler();  // Handle I2C2 communication
        clearI2C2Interrupt();     // Clear the interrupt flag
    }
}

void initI2C2_PPS() {
    unlockPPS(); 
    RPB8Rbits.RPB8R = 0b0010;  // Set RPB8 as SDA2
    RPB9Rbits.RPB9R = 0b0010;  // Set RPB9 as SCL2
    lockPPS();  // Lock PPS
    i2c2InitAsSlave(I2C2_ADDRESS);
}

void i2c2InitAsSlave(uint8_t address) {
    I2C2BRG = 0x0076;         // Set baud rate for I2C2
    I2C2CONbits.I2CEN = 1;  // Enable I2C2
    I2C2CONbits.SCLREL = 1; // Release the clock
//    I2C2CONbits.SLAVE = 1;  // Set as slave mode
    I2C2ADD = I2C2_ADDRESS;      // Set slave address
    I2C2CONbits.DISSLW = 1; // Disable slew rate control
    IFS1bits.I2C2BIF = 0;   // Clear the interrupt flag
    IEC1bits.I2C2BIE = 1;   // Enable I2C2 Master Interrupt
}

void i2c2InterruptHandler() {

    if (I2C2STATbits.R_W == 0) {
        i2c2ReadData();  // Read data from the master
    } else {
        i2c2WriteData();  // Write data to the master
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
    uint8_t dataToSend = 0x55;  // Example data to send
    I2C2TRN = dataToSend; // Transmit data to master
    while (I2C2STATbits.TRSTAT);  // Wait until transmission is complete
}

void clearI2C2Interrupt() {
    IFS1bits.I2C2BIF = 0;  // Clear the interrupt flag
}

void initI2C_PPS() {
    unlockPPS();  
    RPB8Rbits.RPB8R = 0b0001;
    RPB9Rbits.RPB9R = 0b0001; 
    lockPPS(); 
    I2C1CONbits.I2CEN = 1;   
    I2C1BRG = 0x27;          
}

void initUART_PPS() {
    unlockPPS();  
    U1RXR = 0011; 
    RPB15R = 0001;    
    lockPPS();  
    U1MODEbits.UARTEN = 1;   // Enable UART
    U1MODEbits.BRGH = 0;     // Low-speed mode
    U1BRG = 25;              // Baud rate configuration (depends on clock)
    U1STAbits.UTXEN = 1;     // Enable UART transmit
}

void initSystem() {
    initI2C_PPS();
    initUART_PPS(); 
    i2c2InitAsSlave(I2C2_ADDRESS);
}

void processCommand(char* command) {
    if (strcmp(command, "init") == 0) {
        initSystem();
        sendToUART("System Initialized\n");
    } else if (strcmp(command, "start") == 0) {
        startDataAcquisition();
        sendToUART("Data Acquisition Started\n");
    } else if (strcmp(command, "stop") == 0) {
        stopDataAcquisition();
        sendToUART("Data Acquisition Stopped\n");
    } else if (strcmp(command, "reset") == 0) {
        resetSystem();
        sendToUART("System Reset\n");
    } else if (strncmp(command, "setv ", 5) == 0) {
        uint16_t voltage = atoi(command + 5);
        writeDACValue(voltage);
        sendToUART("DAC Voltage Set\n");
    } else if (strcmp(command, "volts?") == 0) {
        uint16_t voltage = readDACValue();
        sprintf(response, "Current Voltage: %d\n", voltage);
        sendToUSB(response);
    } else {
        sendToUART("Unknown Command\n");
    }

    char response[100];
    uint16_t voltage = readDACValue();
    sprintf(response, "I2C Voltage: %d\n", voltage);
    sendToUSB(response);   

    uint8_t uartResponse = uartReceive(); 
    sprintf(response, "UART Response: %c\n", uartResponse);
    sendToUSB(response);    
}

void sendToUART(char* message) {
    for (uint16_t i = 0; i < strlen(message); i++) {
        uartTransmit(message[i]);
    }
}

void uartTransmit(uint8_t data) {
    while (U1STAbits.UTXBF); 
    U1TXREG = data;           
}

// UART Receive Function
uint8_t uartReceive() {
    while (!U1STAbits.URXDA);  
    return U1RXREG;            
}

void writeDACValue(uint16_t value) {
    if (value > 4095) value = 4095; 
    mcp4725WriteDAC(value);
}

uint16_t readDACValue() {
    return mcp4725ReadDAC();
}

void mcp4725WriteDAC(uint16_t value) {
    i2cStart();
    i2cWrite(MCP4725_ADDRESS << 1);   
    i2cWrite((value >> 4) & 0xFF);   
    i2cWrite((value & 0x0F) << 4);  
    i2cStop();
}

uint16_t mcp4725ReadDAC() {
    uint16_t value = 0;
    i2cStart();
    i2cWrite((MCP4725_ADDRESS << 1) | 1);  
    value = i2cReadAck() << 4;  
    value |= i2cReadNack() >> 4; 
    i2cStop();
    return value;
}

void i2cStart() {
    I2C1CONbits.SEN = 1;  
    while (I2C1CONbits.SEN);
}

void i2cStop() {
    I2C1CONbits.PEN = 1;  
    while (I2C1CONbits.PEN);
}

void i2cWrite(uint8_t data) {
    I2C1TRN = data;
    while (I2C1STATbits.TRSTAT);
}

uint8_t i2cReadAck() {
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    return I2C1RCV;
}

uint8_t i2cReadNack() {
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    return I2C1RCV;
}

void startDataAcquisition() {

}

void stopDataAcquisition() {
}

void monitorOscillator() {
    if (!OSCCONbits.COSC) {   
        switchToInternalOscillator();
    }
}

// Switch to Internal Oscillator
void switchToInternalOscillator() {
    OSCCONbits.NOSC = 0b001;  
    OSCCONbits.OSWEN = 1;     
    while (OSCCONbits.OSWEN);
}

void resetSystem() {
    unlockPPS();
    RCONbits.SWR = 1;
    lockPPS();
}
