/*
 * File:   newmainXC16.c
 * Author: info_noki
 *
 * Created on October 16, 2024, 11:15 AM
 */

#include <xc.h>

// Configuration bits settings
#pragma config FWDTEN = OFF  // Watchdog Timer Disabled

void UART_Init() {
    // Set up UART on PIC24 (using UART1)
    U1MODEbits.BRGH = 0;      // Low-speed mode
    U1BRG = 25;               // Baud rate (9600 @ 8MHz Fcy)
    U1MODEbits.UARTEN = 1;    // Enable UART
    U1STAbits.UTXEN = 1;      // Enable Transmit
}

void UART_Write(char data) {
    while (U1STAbits.UTXBF);  // Wait until buffer is not full
    U1TXREG = data;           // Transmit data
}

char UART_Read() {
    while (!U1STAbits.URXDA); // Wait for data to be available
    return U1RXREG;           // Return received data
}

void setup() {
    // Set RA8 as output (LED control)
    TRISAbits.TRISA8 = 0;     // Set RA8 as output
    LATAbits.LATA8 = 0;       // Initially turn off the LED
}

void loop() {
    char command = UART_Read();  

    if (command == '1') {
        LATAbits.LATA8 = 1;      // Turn on the LED (RA8)
    } else if (command == '2') {
        LATAbits.LATA8 = 0;      // Turn off the LED (RA8)
    }
}

int main() {
    setup();
    UART_Init();
    while (1) {
        loop();
    }
}
