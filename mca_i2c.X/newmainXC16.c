/*
 * File:   newmainXC16.c
 * Author: Dell
 *
 * Created on October 7, 2024, 12:32 PM
 */


#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "dacCommunication.h"
#include "fpgaCommunication.h"
#include "i2cCommunication.h"

#define FCY 16000000  
#define BAUDRATE 9600 

#pragma config FNOSC = FRCPLL     // Primary Oscillator: Fast RC with PLL
#pragma config POSCMOD = XT          // External Crystal Oscillator mode (XT, HS, or EC mode)
#pragma config IESO = ON          // Internal/External Oscillator Switchover (optional, for dynamic switching)
#pragma config OSCIOFNC = OFF     // Disable CLKO output, use the pin as GPIO instead of clock output
#pragma config FWDTEN = OFF       // Disable Watchdog Timer

#define I2C2_ADDRESS 0x20

// Function Prototypes
void initSystem();
void processCommand(char* command);
void switchToInternalOscillator();
void monitorOscillator();
void watchdogSetup();
void resetSystem();

char command[100];
char response[50];

// Main Function
int main(void) {
    initSystem();
    while (1) {
        processI2C2();
        if (strlen(command) > 0) {
            processCommand(command);
            memset(command, 0, sizeof(command));
        }
        if (!OSCCONbits.COSC) {
            switchToInternalOscillator(); // Switch to internal oscillator if necessary
        }
    }
    return 0;
}

void initSystem() {
    initI2C1();
    initUART_PPS(); 
    i2c2InitSlave(I2C2_ADDRESS);
}

void processCommand(char* command) {
    if (strcmp(command, "setup") == 0) {
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
    } else if (strncmp(command, "SETV", 5) == 0) {
        uint16_t voltage = atoi(command + 5);
        writeDACValue(voltage);
        sendToUART("DAC Voltage Set\n");
    } else if (strcmp(command, "volts?") == 0) {
        uint16_t voltage = readDACValue();
        sprintf(response, "Current Voltage: %d\n", voltage);
    } else {
        sendToUART("Unknown Command\n");
    }

    char response[100];
    uint16_t voltage = readDACValue();
    sprintf(response, "I2C Voltage: %d\n", voltage);  

    uint8_t uartResponse = uartReceive(); 
    sprintf(response, "UART Response: %c\n", uartResponse);   
}

void monitorOscillator() {
    if (!OSCCONbits.COSC) {   
        switchToInternalOscillator();
    }
}

void switchToInternalOscillator() {
    OSCCONbits.NOSC = 0b001;  
    OSCCONbits.OSWEN = 1;     
    while (OSCCONbits.OSWEN);
}

void resetSystem() {
    unlockPPS();
    RCONbits.SWR = 1;
    lockPPS();
    printf("print statements included");
}
