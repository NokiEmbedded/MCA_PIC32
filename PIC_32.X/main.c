/*
 * File:   main.c
 * Author: info_noki
 *
 * Created on October 8, 2024, 5:52 PM
 */

#include <xc.h>
#include <string.h>

//******************************************************************************
//***********************************Declarations*******************************
//Configuration Settings
#pragma config FNOSC = PRIPLL       
#pragma config JTAGEN = OFF
#pragma config FWDTEN = OFF          
#pragma config ICESEL = ICS_PGx1    
#pragma config FUSBIDIO = ON        
#pragma config FVBUSONIO = ON        

#define SYS_FREQ 25000000  
#define BAUD_RATE 115200  
#define BUFFER_SIZE 20  // Increased buffer size for longer commands

// Command Definitions
#define CMD_START       0x00
#define CMD_STOP        0x01
#define CMD_READ_VOLTS  0x02  // Updated to use 0x02 for read volts
#define CMD_SET_VOLTAGE 0x03  // Updated to use 0x03 for set voltage

// Function Declarations
void UART1_Init();
void UART1_Write(unsigned char data);
unsigned char UART1_Read(void);
void Handle_Command(const char* command);
void UART1_Read_String(char* buffer, int maxLength);
void I2C_Init();
void I2C_Write(unsigned char address, unsigned char data);
unsigned int I2C_Read_Voltage(void);

//******************************************************************************
//**************************************USB************************************//
void UART1_Init() {
    U1MODEbits.BRGH = 0;  
    U1BRG = ((SYS_FREQ / (16 * BAUD_RATE)) - 1);  // Set baud rate
    U1MODEbits.PDSEL = 0;  // 8-bit data, no parity
    U1MODEbits.STSEL = 0;  // 1 stop bit
    U1STAbits.UTXEN = 1;   // Enable transmitter
    U1STAbits.URXEN = 1;   // Enable receiver
    U1MODEbits.ON = 1;     // Enable UART module

    // Configure RB10 (TX) and RB11 (RX) for UART
    TRISBbits.TRISB10 = 0; // TX (Output)
    TRISBbits.TRISB11 = 1; // RX (Input)
    U1RXRbits.U1RXR = 0b0011; // Set RB11 as U1RX
    RPB10Rbits.RPB10R = 0b0001; // Set RB10 as U1TX
}

void UART1_Write(unsigned char data) {
    while (U1STAbits.UTXBF); // Wait while the buffer is full
    U1TXREG = data; // Write the data to the register
}

unsigned char UART1_Read(void) {
    while (!U1STAbits.URXDA); // Wait for data to be available
    return U1RXREG; // Read the received data
}

void UART1_Read_String(char* buffer, int maxLength) {
    int i = 0;
    char received;
    do {
        received = UART1_Read();  // Read character
        buffer[i++] = received;
    } while (received != '\n' && i < maxLength - 1);  // Stop reading at newline or buffer full

    buffer[i - 1] = '\0';  // Null-terminate string, removing '\n'
}

void Handle_Command(const char* command) {
    if (strcmp(command, "START") == 0) {
        LATBbits.LATB0 = 1;  // Turn on the LED
        UART1_Write(CMD_START);    // Send address for START command (0x00)
    } else if (strcmp(command, "STOP") == 0) {
        LATBbits.LATB0 = 0;  // Turn off the LED
        UART1_Write(CMD_STOP);    // Send address for STOP command (0x01)
    } else if (strcmp(command, "READ_VOLTS") == 0) {
        unsigned int voltage = I2C_Read_Voltage();  // Read voltage from DAC
        UART1_Write(CMD_READ_VOLTS); // Send command to indicate reading voltage
        UART1_Write(voltage & 0xFF); // Send lower byte
        UART1_Write((voltage >> 8) & 0xFF); // Send upper byte
    } else if (strncmp(command, "SET_", 4) == 0) {
        int voltageValue = atoi(command + 4); // Get voltage value after "SET_"
        if (voltageValue >= 0 && voltageValue <= 1000) { // Check range
            unsigned int dacValue = (voltageValue * 16384) / 1000; 
            UART1_Write(CMD_SET_VOLTAGE); // Send command to indicate setting voltage
            I2C_Write(0x00, (dacValue >> 8) & 0xFF); 
            I2C_Write(0x00, dacValue & 0xFF);
        }
    } else {
        UART1_Write(0xFF);    // Send an error response for unknown commands (0xFF)
    }
}

//******************************I2C Functions***********************************//
void I2C_Init() {
    I2C1BRG = (SYS_FREQ / (2 * 100000)) - 1; // Set baud rate for I2C (100kHz)
    I2C1CONbits.I2CEN = 1; // Enable I2C1
}

void I2C_Write(unsigned char address, unsigned char data) {
    I2C1CONbits.SEN = 1; // Start condition
    while (I2C1CONbits.SEN); // Wait until start condition is complete

    I2C1TRN = address; // Load address into transmit register
    while (I2C1STATbits.TRSTAT); // Wait for transmission to finish

    I2C1TRN = data; // Load data to transmit
    while (I2C1STATbits.TRSTAT); // Wait for transmission to finish

    I2C1CONbits.PEN = 1; // Stop condition
    while (I2C1CONbits.PEN); // Wait until stop condition is complete
}

unsigned int I2C_Read_Voltage(void) {
    unsigned int voltage;
    I2C1CONbits.SEN = 1; // Start condition
    while (I2C1CONbits.SEN); // Wait until start condition is complete

    I2C1TRN = 0x28; // Slave address
    while (I2C1STATbits.TRSTAT); // Wait for transmission to finish

    I2C1TRN = 0x01; // Read voltage command
    while (I2C1STATbits.TRSTAT); // Wait for transmission to finish

    I2C1CONbits.RCEN = 1; // Start receiving
    while (!I2C1STATbits.RBF); // Wait for receive buffer to be filled

    voltage = I2C1RCV; // Read the voltage
    I2C1CONbits.PEN = 1; // Stop condition
    while (I2C1CONbits.PEN); // Wait until stop condition is complete

    return voltage; // Return the read voltage
}

//******************************************************************************
//*****************************Main Function************************************//
int main(void) {
    // Initialize UART and I2C
    UART1_Init();
    I2C_Init();

    while (1) {
        char commandBuffer[BUFFER_SIZE];
        UART1_Read_String(commandBuffer, BUFFER_SIZE); // Read command from UART
        Handle_Command(commandBuffer); // Handle the command
    }

    return 0;
}
