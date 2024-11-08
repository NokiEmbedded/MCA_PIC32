#include <xc.h>
#include <stdio.h>
#include <stdbool.h>

#define FCY 60000000UL             // Define FCY for 60 MHz (FOSC / 2)
#include <libpic30.h>

// Configuration bits for using an external oscillator
#pragma config FNOSC = PRIPLL      // Primary Oscillator with PLL (XT, HS, EC)
#pragma config POSCMD = XT         // XT Oscillator mode
#pragma config OSCIOFNC = OFF      // OSC2 pin function is clock output
#pragma config FCKSM = CSDCMD      // Clock switching and monitor disabled
#pragma config FWDTEN = OFF        // Watchdog timer disabled
#pragma config IESO = OFF          // Two-speed start-up disabled
#pragma config IOL1WAY = OFF       // Allow multiple reconfigurations
#pragma config ICS = PGD2          // Emulator functions on PGD2 (Adjust based on your setup)


#define UART_BAUDRATE 115200       // Desired UART baud rate
// Constants for LED modes
#define OFF 0
#define ON 1
#define TOGGLE 3
#define BLINKSLOW 4
#define BLINKFAST 5
#define UART_BAUDRATE 115200

// Error handling
int LastError = 0;
unsigned long BaudRate = 0;

// Function Prototypes
void InitMCP2200(unsigned int VendorID, unsigned int ProductID);
bool ConfigureMCP2200(unsigned char IOMap, unsigned long BaudRate, unsigned int RxLED, unsigned int TxLED, bool HardwareFlowControl, bool USBCFG, bool Suspend);
bool fnRxLED(unsigned int mode);
bool fnTxLED(unsigned int mode);
bool fnHardwareFlowControl(bool onOff);
bool fnUSBCFG(bool onOff);
bool fnSuspend(bool onOff);
bool fnSetBaudRate(unsigned long BaudRateParam);
bool ConfigureIO(unsigned char IOMap);
bool WritePort(unsigned int portValue);
bool ReadPort(unsigned int *returnvalue);

// Function to initialize the oscillator
void init_oscillator(void) {
    // Configure PLL and wait for the oscillator to stabilize (if necessary)
    CLKDIVbits.PLLPRE = 0;    // PLL prescaler (N1) = 2
    PLLFBDbits.PLLDIV = 38;   // PLL multiplier (M) = 40
    CLKDIVbits.PLLPOST = 0;   // PLL postscaler (N2) = 2

    // Initiate Clock Switch to Primary Oscillator with PLL (NOSC = 0b011)
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(OSCCON | 0x01);

    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b011);

    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);
}

// Function to initialize UART
void init_uart(void) {
    // Unlock the RPOR and RPINR registers
    __builtin_write_OSCCONL(OSCCON & 0xbf);

    // Configure RP39 as UART TX (U1TX) and RP40 as UART RX (U1RX)
    RPOR2bits.RP39R = 1; // Ensure this matches your device's register
    RPINR18bits.U1RXR = 40; // Map RP39 to U1RX (UART1 Receive)

    // Lock the RPOR and RPINR registers
    __builtin_write_OSCCONL(OSCCON | 0x40);

    U1MODEbits.UARTEN = 0;    // Disable UART during configuration
    U1MODEbits.BRGH = 0;      // Standard Speed mode (16x baud clock, BRGH = 0)

    U1BRG = ((FCY / (16 * UART_BAUDRATE)) - 1);  // Baud rate setting
    BaudRate = U1BRG;

    U1STAbits.UTXEN = 1;      // Enable UART transmitter
    U1MODEbits.UARTEN = 1;    // Enable UART
}

// Example implementation for InitMCP2200
void InitMCP2200(unsigned int VendorID, unsigned int ProductID) {
    // Initialize the MCP2200 with VendorID and ProductID
    printf("Initializing MCP2200 with Vendor ID: %X, Product ID: %X\n", VendorID, ProductID);
}

// Example implementation for ConfigureMCP2200
bool ConfigureMCP2200(unsigned char IOMap, unsigned long BaudRate, unsigned int RxLED, unsigned int TxLED, bool HardwareFlowControl, bool USBCFG, bool Suspend) {
    // Configure MCP2200
    printf("Configuring MCP2200: IOMap=%02X, BaudRate=%lu, RxLED=%d, TxLED=%d, HWFlowControl=%d, USBCFG=%d, Suspend=%d\n", 
           IOMap, BaudRate, RxLED, TxLED, HardwareFlowControl, USBCFG, Suspend);
    return true; // Assume success
}

// Example implementation for fnRxLED
bool fnRxLED(unsigned int mode) {
    printf("Configuring RxLED to mode: %d\n", mode);
    return true; // Assume success
}

// Example implementation for fnTxLED
bool fnTxLED(unsigned int mode) {
    printf("Configuring TxLED to mode: %d\n", mode);
    return true; // Assume success
}

// Example implementation for fnHardwareFlowControl
bool fnHardwareFlowControl(bool onOff) {
    printf("Configuring Hardware Flow Control to: %d\n", onOff);
    return true; // Assume success
}

// Example implementation for fnUSBCFG
bool fnUSBCFG(bool onOff) {
    printf("Configuring USBCFG to: %d\n", onOff);
    return true; // Assume success
}

// Example implementation for fnSuspend
bool fnSuspend(bool onOff) {
    printf("Configuring Suspend to: %d\n", onOff);
    return true; // Assume success
}

// Example implementation for fnSetBaudRate
bool fnSetBaudRate(unsigned long BaudRateParam) {
    printf("Setting Baud Rate to: %lu\n", BaudRateParam);
    return true; // Assume success
}

// Example implementation for ConfigureIO
bool ConfigureIO(unsigned char IOMap) {
    printf("Configuring IO: IOMap=%02X\n", IOMap);
    return true; // Assume success
}

// Example implementation for WritePort
bool WritePort(unsigned int portValue) {
    printf("Writing to port: %02X\n", portValue);
    return true; // Assume success
}

// Example implementation for ReadPort
bool ReadPort(unsigned int *returnvalue) {
    *returnvalue = 0x5A; // Simulate reading a value
    printf("Reading from port: %02X\n", *returnvalue);
    return true; // Assume success
}

// Main function for demonstration
int main() {
    // Initialize oscillator
    init_oscillator();
    
    // Initialize UART
    init_uart();
    
    // Initialize MCP2200
    InitMCP2200(0x04D8, 0x00DF);
    
    // Configure MCP2200
    if (ConfigureMCP2200(0x43, UART_BAUDRATE, BLINKFAST, BLINKSLOW, false, false, false)) {
        printf("MCP2200 configured successfully.\n");
    } else {
        printf("MCP2200 configuration failed. Error: %d\n", LastError);
    }
    
    // Configure LEDs
    fnRxLED(BLINKFAST);
    fnTxLED(BLINKSLOW);
    
    // Example of writing and reading from the port
    WritePort(0x5A);
    unsigned int portValue;
    ReadPort(&portValue);
    
    __delay_ms(2000);
    
    return 0;
}
