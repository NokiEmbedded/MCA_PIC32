#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "fpga.h"
#include "uart_usb.h"

#define BUDRATE 9600

void initUART2_PPS() {
    unlockPPS();  
    RPB14Rbits.RPB14R = 0b0010;  // Set RPB14 as U2TX (TX pin)
    U2RXRbits.U2RXR = 0b0100;    // Set RPB8 as U2RX (RX pin)
    lockPPS();  

    U2MODEbits.BRGH = 0;                // Low-speed mode
    U2MODEbits.PDSEL = 0;               // 8-bit data, no parity
    U2MODEbits.STSEL = 0;               // 1 Stop bit
    U2MODEbits.UARTEN = 1;              // Enable UART
    U2STAbits.UTXEN = 1;                // Enable UART transmit
}


void sendToSlave(char* message) {
    for (uint16_t i = 0; i < strlen(message); i++) {
        uartTransmit(message[i]);
    }
}

void SlaveTransmit(uint8_t data) {
    while (U1STAbits.UTXBF); 
    U1TXREG = data;           
}

uint8_t slaveReceive() {
    while (!U1STAbits.URXDA);  
    return U1RXREG;            
}

