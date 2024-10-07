/*
 * File:   newmainXC16.c
 * Author: Dell
 *
 * Created on October 7, 2024, 10:26 AM
 */


#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <proc/p32mx250f128d.h>

// Configuring the oscillator settings for an external crystal oscillator
#pragma config FNOSC = FRCPLL     // Primary Oscillator: Fast RC with PLL
#pragma config POSCMOD = XT          // External Crystal Oscillator mode (XT, HS, or EC mode)
#pragma config IESO = ON          // Internal/External Oscillator Switchover (optional, for dynamic switching)
#pragma config OSCIOFNC = OFF     // Disable CLKO output, use the pin as GPIO instead of clock output
#pragma config FWDTEN = OFF       // Disable Watchdog Timer


// Function Prototypes
void initSystem();
void initI2C_PPS();
void initUART_PPS();
void processCommand(char* command);
void switchToInternalOscillator();
void setVoltage(uint16_t voltage);
uint16_t readVoltage();
void monitorOscillator();
void watchdogSetup();
void startDataAcquisition();
void stopDataAcquisition();
void resetSystem();
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

// Define MCP4725 address for I2C communication
#define MCP4725_ADDRESS 0x60 // MCP4725 I2C address (can vary with A0 pin)

// Main Function
int main(void) {
    initSystem();  // Initialize the system

    char command[100];
    
    // Main loop
    while (1) {
        // Read the command via UART (pseudo-code, replace with actual UART reading)
        fgets(command, sizeof(command), stdin); // For simplicity, using fgets (replace it)
        
        // Process command
        processCommand(command);
        
        // Monitor the oscillator health
        monitorOscillator();
    }
    
    return 0;
}

// Function to initialize system components
void initSystem() {
    // Initialize PPS and peripherals
    initI2C_PPS();
    initUART_PPS();
    watchdogSetup();  // Initialize the watchdog timer
    
    // Setup external oscillator
    if (!OSCCONbits.COSC) {
        switchToInternalOscillator();
    }
}

// I2C Initialization using PPS
// Function to unlock PPS
void unlockPPS() {
    SYSKEY = 0xAA996655; // Write Key1
    SYSKEY = 0x556699AA; // Write Key2 to unlock
}

// Function to lock PPS
void lockPPS() {
    SYSKEY = 0; // Lock PPS
}

// I2C Initialization using PPS
void initI2C_PPS() {
    unlockPPS();  // Unlock PPS

    // Assign I2C1 SDA1 and SCL1 to specific pins
    RPB8Rbits.RPB8R = 0b0001;  // Assign RPB9 to SDA1 input
    RPB9Rbits.RPB9R = 0b0001;  // Set RP11 as SCL1 input (Adjust as needed)

    lockPPS();  // Lock PPS

    // Setup I2C1 module
    I2C1CONbits.I2CEN = 1;   // Enable I2C1
    I2C1BRG = 0x27;          // Set I2C1 baud rate for 100kHz
}

// UART Initialization using PPS
void initUART_PPS() {
    unlockPPS();  // Unlock PPS

    // Assign UART1 TX and RX pins
    U1RXR = 0011;  // Set RP13 as UART1 RX input (Adjust as needed)
    RPB15R = 0001;      // Set RP14 as UART1 TX output (Adjust as needed)

    lockPPS();  // Lock PPS

    // Setup UART1 module
    U1MODEbits.UARTEN = 1;   // Enable UART
    U1MODEbits.BRGH = 0;     // Low-speed mode
    U1BRG = 25;              // Baud rate configuration (depends on clock)
    U1STAbits.UTXEN = 1;     // Enable UART transmit
}

// Command Processing
void processCommand(char* command) {
    if (strcmp(command, "init") == 0) {
        initSystem();
        sendToUART("System Initialized\n");
    }
    else if (strcmp(command, "start") == 0) {
        startDataAcquisition();
        sendToUART("Acquisition Started\n");
    }
    else if (strcmp(command, "stop") == 0) {
        stopDataAcquisition();
        sendToUART("Acquisition Stopped\n");
    }
    else if (strcmp(command, "reset") == 0) {
        resetSystem();
    }
    else if (strncmp(command, "setv", 4) == 0) {
        uint16_t voltage = atoi(command + 5);  // Parse voltage value
        writeDACValue(voltage);  // Use separate function to set the DAC voltage
        sendToUART("Voltage Set\n");
    }
    else if (strcmp(command, "volts?") == 0) {
        uint16_t voltage = readDACValue();  // Use separate function to read the DAC voltage
        char response[50];
        sprintf(response, "Current Voltage: %d\n", voltage);
        sendToUART(response);
    }
}

// Write Voltage using MCP4725 DAC (via I2C)
void writeDACValue(uint16_t value) {
    // Ensure the value is 12-bit (MCP4725 is 12-bit DAC)
    if (value > 4095) {
        value = 4095;
    }
    mcp4725WriteDAC(value);  // Call the function to write the value
}

// Read Voltage from MCP4725 DAC (via I2C)
uint16_t readDACValue() {
    return mcp4725ReadDAC();   // Call the function to read the value
}

// MCP4725 DAC Write Function (Set Voltage)
void mcp4725WriteDAC(uint16_t value) {
    i2cStart();
    i2cWrite(MCP4725_ADDRESS << 1);  // Send MCP4725 address with write bit
    i2cWrite((value >> 4) & 0xFF);   // Send MSB (upper 8 bits)
    i2cWrite((value & 0x0F) << 4);    // Send LSB (lower 4 bits shifted)
    i2cStop();
}

// MCP4725 DAC Read Function (Get Current Voltage)
uint16_t mcp4725ReadDAC() {
    uint16_t value = 0;

    i2cStart();
    i2cWrite((MCP4725_ADDRESS << 1) | 1);  // Send MCP4725 address with read bit
    value = i2cReadAck() << 4;             // Read MSB (upper 8 bits)
    value |= i2cReadNack() >> 4;           // Read LSB (lower 4 bits)
    i2cStop();

    return value;
}

// I2C Start Condition
void i2cStart() {
    I2C1CONbits.SEN = 1;  // Initiate Start condition
    while (I2C1CONbits.SEN);  // Wait until start condition is finished
}

// I2C Stop Condition
void i2cStop() {
    I2C1CONbits.PEN = 1;  // Initiate Stop condition
    while (I2C1CONbits.PEN);  // Wait until stop condition is finished
}

// I2C Write Function
void i2cWrite(uint8_t data) {
    I2C1TRN = data;  // Load data into transmit register
    while (I2C1STATbits.TRSTAT);  // Wait until transmission is complete
}

// I2C Read with Acknowledge
uint8_t i2cReadAck() {
    I2C1CONbits.RCEN = 1;  // Enable Receive mode
    while (!I2C1STATbits.RBF);  // Wait until data is received
    return I2C1RCV;  // Return received data
}

// I2C Read with No Acknowledge
uint8_t i2cReadNack() {
    I2C1CONbits.RCEN = 1;  // Enable Receive mode
    while (!I2C1STATbits.RBF);  // Wait until data is received
    return I2C1RCV;  // Return received data
}

// Monitor Oscillator Health
void monitorOscillator() {
    if (!OSCCONbits.COSC) {   // Check if external oscillator fails
        switchToInternalOscillator();
        sendToUART("Switched to Internal Oscillator\n");
    }
}

// Switch to Internal Oscillator
void switchToInternalOscillator() {
    OSCCONbits.NOSC = 0b001;  // Internal Fast RC (FRC) Oscillator
    OSCCONbits.OSWEN = 1;     // Initiate clock switch
    while (OSCCONbits.OSWEN); // Wait for switch to complete
}

// Watchdog Timer Setup
void watchdogSetup() {
    RCONbits.WDTO = 1;  // Enable Watchdog Timer
}

// Send Message via UART
void sendToUART(char* message) {
    for (uint16_t i = 0; i < strlen(message); i++) {
        uartTransmit(message[i]);
    }
}

// UART Transmit Function
void uartTransmit(uint8_t data) {
    while (U1STAbits.UTXBF);  // Wait until transmit buffer is empty
    U1TXREG = data;           // Send data
}

// UART Receive Function (Blocking)
uint8_t uartReceive() {
    while (!U1STAbits.URXDA); // Wait until data is available
    return U1RXREG;           // Return received data
}

// Data Acquisition Start
void startDataAcquisition() {
    // Add custom data acquisition code (communicate with FPGA, sensors, etc.)
}

// Data Acquisition Stop
void stopDataAcquisition() {
    // Add custom data stopping code
}

// System Reset
void resetSystem() {
    unlockPPS();
    RCONbits.SWR = 1;
    lockPPS();
}

void configureBrownOutReset() {
    // Enable Brown-Out Reset
    RCONbits.BOR = 1; // Enable the Brown-Out Reset (BOR) functionality

    // Set the BOR voltage threshold to 2.8V (using BORV = 0b100)
//    RCONbits.BORV = 0b100;   // Set the Brown-Out Reset voltage threshold to 2.8V
}

