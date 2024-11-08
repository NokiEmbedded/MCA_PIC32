#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "fpga.h"
 

void initUART1_PPS() {
    unlockPPS();  
    // Set RPA0 as U1TX (TX pin)
    RPA0Rbits.RPA0R = 0b0001;  
    // Set RPb2 as U1RX (RX pin)
    U1RXRbits.U1RXR = 0b0100;  
    lockPPS();  

    // Configure UART1 settings
    U1MODEbits.BRGH = 0;                // Low-speed mode
    U1MODEbits.PDSEL = 0;               // 8-bit data, no parity
    U1MODEbits.STSEL = 0;               // 1 Stop bit
    U1MODEbits.UARTEN = 1;              // Enable UART
    U1STAbits.UTXEN = 1;                // Enable UART transmit
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

uint8_t uartReceive() {
    while (!U1STAbits.URXDA);  
    return U1RXREG;            
}

void unlockPPS() {
    SYSKEY = 0xAA996655; // Write Key1
    SYSKEY = 0x556699AA; // Write Key2 to unlock
}

void lockPPS() {
    SYSKEY = 0; // Lock PPS
}

void startDataAcquisition() {
    // Implement data acquisition logic here
}

void stopDataAcquisition() {
    // Implement stopping logic here
}

//void initUART_PPS() {
//    unlockPPS();  
//    U1RXR = 0011; 
//    RPB15R = 0001;    
//    lockPPS();  
//    U1MODEbits.UARTEN = 1;   // Enable UART
//    U1MODEbits.BRGH = 0;     // Low-speed mode
//    U1BRG = 25;              // Baud rate configuration (depends on clock)
//    U1STAbits.UTXEN = 1;     // Enable UART transmit
//}