/*
 * File:   main.c
 * Author: Dell
 *
 * Created on October 7, 2024, 12:31 PM
 */

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <usb_cdc.h>

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



void switchToInternalOscillator();
uint16_t readVoltage();
void monitorOscillator();
void watchdogSetup();
void startDataAcquisition();
void stopDataAcquisition();
void resetSystem();


#define MCP4725_ADDRESS 0x60

// Main Function
int main(void) {
    initSystem();  // Initialize the system

    char command[100];

    // Main loop
    while (1) {
        // Check if a command is received from USB
        if (USBCommandReceived()) {
            receiveFromUSB(command, sizeof(command));  // Fetch command from USB (implement USB read function)
            processCommand(command);
        }
        if (!OSCCONbits.COSC) {
            switchToInternalOscillator();
        }
    }

    return 0;
}

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

// Function to initialize system components
void initSystem() {
    initI2C_PPS();
    initUART_PPS();
    initUSB();     // Initialize USB CDC
}

// USB Initialization (USB as a CDC device)
void initUSB() {
    // USB CDC Initialization (Add your USB stack initialization code here)
    USBDeviceInit();      // Initialize the USB device
    USBDeviceAttach();     // Attach the device to the USB bus
}

// Command Processing
void processCommand(char* command) {
    if (strcmp(command, "init") == 0) {
        initSystem();  // Initialize system components (UART, I2C, etc.)
        sendToUART("System Initialized\n");
        sendToUSB("System Initialized\n");
    }
    else if (strcmp(command, "start") == 0) {
        startDataAcquisition();  // Start data acquisition from I2C/UART devices
        sendToUART("Acquisition Started\n");
        sendToUSB("Acquisition Started\n");
    }
    else if (strcmp(command, "stop") == 0) {
        stopDataAcquisition();  // Stop data acquisition
        sendToUART("Acquisition Stopped\n");
        sendToUSB("Acquisition Stopped\n");
    }
    else if (strcmp(command, "reset") == 0) {
        resetSystem();  // Reset the system (reinitialize components)
        sendToUART("System Reset\n");
        sendToUSB("System Reset\n");
    }
    else if (strncmp(command, "setv", 4) == 0) {
        uint16_t voltage = atoi(command + 5);  // Parse voltage value from the command
        writeDACValue(voltage);  // Set the DAC voltage via I2C
        sendToUART("Voltage Set\n");
        sendToUSB("Voltage Set\n");
    }
    else if (strcmp(command, "volts?") == 0) {
        uint16_t voltage = readDACValue();  // Read the current voltage from the DAC
        char response[50];
        sprintf(response, "Current Voltage: %d\n", voltage);
        sendToUART(response);
        sendToUSB(response);
    }
    else if (strcmp(command, "count?") == 0) {
        uint32_t count = readCounter();  // Read the current counter value (FPGA or I2C)
        char response[50];
        sprintf(response, "Current Count: %lu\n", count);
        sendToUART(response);
        sendToUSB(response);
    }
    else if (strncmp(command, "time", 4) == 0) {
        uint32_t time = atoi(command + 5);  // Parse time value from the command
        setTimeLimit(time);  // Set the time limit in the system
        sendToUART("Time Limit Set\n");
        sendToUSB("Time Limit Set\n");
    }
    else if (strcmp(command, "clear") == 0) {
        clearData();  // Clear any stored data or counters
        sendToUART("Data Cleared\n");
        sendToUSB("Data Cleared\n");
    }
    else {
        // Unknown or unsupported command
        sendToUART("Unknown Command\n");
        sendToUSB("Unknown Command\n");
    }

    char response[100];
    uint16_t voltage = readDACValue();  // Read from I2C
    sprintf(response, "I2C Voltage: %d\n", voltage);
    sendToUSB(response);    // Send I2C response to PC

    uint8_t uartResponse = uartReceive();  // Read from UART (FPGA)
    sprintf(response, "UART Response: %c\n", uartResponse);
    sendToUSB(response);    // Send UART response to PC
}

// Send Message to UART
void sendToUART(char* message) {
    for (uint16_t i = 0; i < strlen(message); i++) {
        uartTransmit(message[i]);
    }
}

// USB Send Function
void sendToUSB(char* message) {
    // Implement USB send function using the CDC class
    USB_DEVICE_CDC_Write(message, strlen(message));
}

// UART Transmit Function
void uartTransmit(uint8_t data) {
    while (U1STAbits.UTXBF);  // Wait until transmit buffer is empty
    U1TXREG = data;           // Send data
}

// UART Receive Function
uint8_t uartReceive() {
    while (!U1STAbits.URXDA);  // Wait until data is received
    return U1RXREG;            // Return received data
}

// I2C and DAC functions (similar to previous code)
void writeDACValue(uint16_t value) {
    if (value > 4095) value = 4095;  // Limit to 12-bit value
    mcp4725WriteDAC(value);
}

uint16_t readDACValue() {
    return mcp4725ReadDAC();
}

void mcp4725WriteDAC(uint16_t value) {
    i2cStart();
    i2cWrite(MCP4725_ADDRESS << 1);  // MCP4725 address + write bit
    i2cWrite((value >> 4) & 0xFF);   // Upper 8 bits
    i2cWrite((value & 0x0F) << 4);   // Lower 4 bits
    i2cStop();
}

uint16_t mcp4725ReadDAC() {
    uint16_t value = 0;
    i2cStart();
    i2cWrite((MCP4725_ADDRESS << 1) | 1);  // MCP4725 address + read bit
    value = i2cReadAck() << 4;  // Upper 8 bits
    value |= i2cReadNack() >> 4;  // Lower 4 bits
    i2cStop();
    return value;
}

// I2C Start and Stop Conditions
void i2cStart() {
    I2C1CONbits.SEN = 1;  // Start condition
    while (I2C1CONbits.SEN);
}

void i2cStop() {
    I2C1CONbits.PEN = 1;  // Stop condition
    while (I2C1CONbits.PEN);
}

// I2C Write and Read Functions
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

// Data Acquisition Start and Stop (custom code as needed)
void startDataAcquisition() {
    // Add code to start acquiring data from FPGA/I2C sensors
}

void stopDataAcquisition() {
    // Add code to stop acquiring data from FPGA/I2C sensors
}

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

void resetSystem() {
    unlockPPS();
    RCONbits.SWR = 1;
    lockPPS();
}
