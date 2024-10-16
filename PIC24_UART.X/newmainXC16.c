/*
 * File:   newmainXC16.c
 * Author: info_noki
 *
 * Created on October 16, 2024, 11:15 AM
 */

#include <xc.h>

// Configuration bits settings
#pragma config FWDTEN = OFF  // Watchdog Timer Disabled

// Function prototypes
void UART_Init();
void UART_Write(char data);
void UART_WriteString(const char* str); // Function to send strings
char UART_Read();
void setup();
void handleCommand(char command); // New function to handle commands

void main(void) {
    // Initialize UART
    UART_Init();
    // Setup additional configurations (if needed)
    setup();
    
    while (1) {
        char command = UART_Read();  // Read command from Arduino
        handleCommand(command);       // Process the command
    }
}

void handleCommand(char command) {
    // Check received command
    if (command == '1') {
        LATAbits.LATA8 = 1;  // Turn on LED (RA8)
        UART_WriteString("START"); // Send "START" message to Arduino
    } else if (command == '2') {
        LATAbits.LATA8 = 0;  // Turn off LED (RA8)
        UART_WriteString("STOP"); // Send "STOP" message to Arduino
    }
}

void UART_Init() {
    // Set RP22 (Pin 2) as UART RX and RP23 (Pin 3) as UART TX
    RPINR18bits.U1RXR = 22;  // Assign RP22 as UART1 RX
    RPOR11bits.RP23R = 3;     // Assign RP23 as UART1 TX (3 corresponds to UART1 TX function)

    // Configure UART parameters
    U1MODEbits.BRGH = 0;      // Low-speed mode
    U1BRG = 25;               // Baud rate (9600 @ 8MHz Fcy)
    U1MODEbits.UARTEN = 1;    // Enable UART
    U1STAbits.UTXEN = 1;      // Enable Transmit
}

void UART_Write(char data) {
    while (U1STAbits.UTXBF);  // Wait until buffer is not full
    U1TXREG = data;           // Transmit data
}

void UART_WriteString(const char* str) {
    while (*str) {            // Loop through the string until the null terminator
        UART_Write(*str++);   // Send each character
    }
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

