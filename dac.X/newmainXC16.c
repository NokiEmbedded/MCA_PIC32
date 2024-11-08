#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define FCY 60000000UL 
#include <libpic30.h>

#pragma config FNOSC = PRIPLL      // Primary Oscillator with PLL (XT, HS, EC)
#pragma config POSCMD = XT         // XT Oscillator mode
#pragma config OSCIOFNC = OFF      // OSC2 pin function is clock output
#pragma config FCKSM = CSDCMD      // Clock switching and monitor disabled
#pragma config FWDTEN = OFF        // Watchdog timer disabled
#pragma config IESO = OFF          // Two-speed start-up disabled
#pragma config IOL1WAY = OFF       // Allow multiple reconfigurations
#pragma config ICS = PGD2          // Emulator functions on PGD2

#define UART_BAUDRATE 115200       // UART baud rate
#define MCP4725_ADDRESS 0x60       // MCP4725 DAC I2C address
#define I2C1_BAUD_RATE 100000      // I2C baud rate (100 kHz)
#define LED_PIN LATAbits.LATA10    // LED on RA10
#define HIGH_VOLTAGE_SETTING 1000  // DAC output voltage setting

// Control modes
#define OFF 0
#define ON 1
#define TOGGLE 3
#define BLINKSLOW 4
#define BLINKFAST 5

// Global Variables
volatile uint16_t count = 0;       // Counter for interrupts

// Function Prototypes
void init_oscillator(void);
void init_uart(void);
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Write(uint8_t data);
void MCP4725_SetOutput(uint16_t dacValue);
void initInterrupt(void);
void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void);
void BlinkLED(uint16_t duration);
void ProcessCount(void);
void ADC_Init(void);
uint16_t ADC_Read(void);

// Initialize Oscillator
void init_oscillator(void) {
    CLKDIVbits.PLLPRE = 0;        // PLL prescaler (N1) = 2
    PLLFBDbits.PLLDIV = 38;       // PLL multiplier (M) = 40
    CLKDIVbits.PLLPOST = 0;       // PLL postscaler (N2) = 2
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(OSCCON | 0x01);
    while (OSCCONbits.COSC != 0b011); // Wait for switch
    while (!OSCCONbits.LOCK);         // Wait for PLL to lock
}

// Initialize UART
void init_uart(void) {
    __builtin_write_OSCCONL(OSCCON & 0xbf);   // Unlock PPS
    RPOR2bits.RP39R = 1;                      // U1TX to RP39
    RPINR18bits.U1RXR = 40;                   // U1RX to RP40
    __builtin_write_OSCCONL(OSCCON | 0x40);   // Lock PPS
    U1MODEbits.UARTEN = 0;    
    U1MODEbits.BRGH = 0;
    U1BRG = ((FCY / (16 * UART_BAUDRATE)) - 1); 
    U1STAbits.UTXEN = 1;    
    U1MODEbits.UARTEN = 1;    
}

// Initialize I2C
void I2C_Init(void) {
    I2C1BRG = (FCY / (2 * I2C1_BAUD_RATE)) - 1;
    I2C1CONbits.I2CEN = 1; // Enable I2C1
}

void I2C_Start(void) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN); // Wait for Start
}

void I2C_Stop(void) {
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN); // Wait for Stop
}

uint8_t I2C_Write(uint8_t data) {
    I2C1TRN = data;               // Load data
    while (I2C1STATbits.TRSTAT);  // Wait for transmission
    return I2C1STATbits.ACKSTAT;  // Return ACK status
}

// Set MCP4725 DAC Output
void MCP4725_SetOutput(uint16_t dacValue) {
    uint8_t upperData = (dacValue >> 4) & 0xFF;
    uint8_t lowerData = (dacValue & 0x0F) << 4;

    I2C_Start(); 
    if (I2C_Write(MCP4725_ADDRESS << 1) != 0) {
        I2C_Stop();
        return;
    }
    I2C_Write(0x40); // Control byte
    I2C_Write(upperData);
    I2C_Write(lowerData);
    I2C_Stop();
}

// Blink LED
void BlinkLED(uint16_t duration) {
    LED_PIN = 1; 
    __delay_ms(duration); 
    LED_PIN = 0;
}

// Initialize Interrupt for Counting
void initInterrupt(void) {
    TRISBbits.TRISB12 = 1;          // Set RB12 as input
    CNENBbits.CNIEB12 = 1;          // Enable CN interrupt for RB12
    IPC4bits.CNIP = 3;              // Set priority
    IFS1bits.CNIF = 0;              // Clear flag
    IEC1bits.CNIE = 1;              // Enable CN interrupt
    __builtin_enable_interrupts();  // Global interrupt enable
}

void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void) {
    if (IFS1bits.CNIF) {
        count++;
        IFS1bits.CNIF = 0; // Clear flag
    }
}

// Process Count
void ProcessCount(void) {
    if (count > 0) {
        BlinkLED(100);
        count = 0;
    }
}

// Initialize ADC
void ADC_Init(void) {
    ANSELAbits.ANSA1 = 1;
    TRISAbits.TRISA1 = 1;
    AD1CON1bits.SAMP = 0;
    AD1CON1bits.ASAM = 1;
    AD1CON2bits.SMPI = 0;
    AD1CON3bits.ADCS = 0b111;
    AD1CON1bits.ADON = 1;
}

// Read ADC Value
uint16_t ADC_Read(void) {
    AD1CON1bits.SAMP = 1;
    __delay_us(10);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

// Main Function
int main(void) {
    init_oscillator();
    I2C_Init();
    initInterrupt();
    init_uart();
    MCP4725_SetOutput(HIGH_VOLTAGE_SETTING); // Set DAC

    while (1) {
        ProcessCount();
        __delay_ms(1000);  // 1 second delay

        uint16_t adcValue = ADC_Read(); // ADC value
    }
    return 0;
}
