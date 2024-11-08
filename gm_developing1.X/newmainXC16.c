#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#define FCY 32000000UL

#define FCY 60000000UL 
#include <libpic30.h>

#pragma config FNOSC = PRIPLL      // Primary Oscillator with PLL (XT, HS, EC)
#pragma config POSCMD = XT         // XT Oscillator mode
#pragma config OSCIOFNC = OFF      // OSC2 pin function is clock output
#pragma config FCKSM = CSDCMD      // Clock switching and monitor disabled
#pragma config FWDTEN = OFF        // Watchdog timer disabled
#pragma config IESO = OFF          // Two-speed start-up disabled
#pragma config IOL1WAY = OFF       // Allow multiple reconfigurations
#pragma config ICS = PGD2          // Emulator functions on PGD2 (Adjust based on your setup)
#define UART_BAUDRATE 115200       // Desired UART baud rate

#define MCP4725_ADDRESS 0x60           // MCP4725 DAC I2C address
#define I2C1_BAUD_RATE 100000      // Desired I2C baud rate (100 kHz)
#define LED_PIN LATAbits.LATA10         // LED on RA10
#define HIGH_VOLTAGE_SETTING 1200 // Adjust this value based on your application needs

#define OFF 0
#define ON 1
#define TOGGLE 3
#define BLINKSLOW 4
#define BLINKFAST 5

int LastError = 0;

volatile uint16_t count = 0; 

void InitMCP2200(unsigned int VendorID, unsigned int ProductID);
bool ConfigureMCP2200(unsigned char IOMap, unsigned long BaudRate, unsigned int RxLED, unsigned int TxLED, bool HardwareFlowControl, bool USBCFG, bool Suspend);
void init_oscillator(void);
void init_uart(void);


void init_oscillator(void) {

    CLKDIVbits.PLLPRE = 0;    // PLL prescaler (N1) = 2
    PLLFBDbits.PLLDIV = 38;   // PLL multiplier (M) = 40
    CLKDIVbits.PLLPOST = 0;   // PLL postscaler (N2) = 2
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(OSCCON | 0x01);
    while (OSCCONbits.COSC != 0b011);
    while (OSCCONbits.LOCK != 1);
    
}

void init_uart(void) {
    __builtin_write_OSCCONL(OSCCON & 0xbf);
    RPOR2bits.RP39R = 1;
    RPINR18bits.U1RXR = 40;
    __builtin_write_OSCCONL(OSCCON | 0x40);
    U1MODEbits.UARTEN = 0;    
    U1MODEbits.BRGH = 0;
    U1BRG = ((FCY / (16 * UART_BAUDRATE)) - 1); 
    U1STAbits.UTXEN = 1;    
    U1MODEbits.UARTEN = 1;    
    
//    printf("USB Initialized...");
}

void InitMCP2200(unsigned int VendorID, unsigned int ProductID) {
}


bool ConfigureMCP2200(unsigned char IOMap, unsigned long BaudRate, unsigned int RxLED, unsigned int TxLED, bool HardwareFlowControl, bool USBCFG, bool Suspend) {

//    printf("Configuring MCP2200: IOMap=%02X, BaudRate=%lu, RxLED=%d, TxLED=%d, HWFlowControl=%d, USBCFG=%d, Suspend=%d\n", 
//           IOMap, BaudRate, RxLED, TxLED, HardwareFlowControl, USBCFG, Suspend);
    return true; 
}

bool fnRxLED(unsigned int mode) {
//    printf("Configuring RxLED to mode: %d\n", mode);
    return true; // Assume success
}

bool fnTxLED(unsigned int mode) {
//    printf("Configuring TxLED to mode: %d\n", mode);
    return true; 
}

bool fnHardwareFlowControl(bool onOff) {
//    printf("Configuring Hardware Flow Control to: %d\n", onOff);
    return true; 
}

bool fnUSBCFG(bool onOff) {
//    printf("Configuring USBCFG to: %d\n", onOff);
    return true; 
}

bool fnSuspend(bool onOff) {
//    printf("Configuring Suspend to: %d\n", onOff);
    return true;
}

bool fnSetBaudRate(unsigned long BaudRateParam) {
//    printf("Setting Baud Rate to: %lu\n", BaudRateParam);
    return true; 
}

bool ConfigureIO(unsigned char IOMap) {
//    printf("Configuring IO: IOMap=%02X\n", IOMap);
    return true; // Assume success
}

bool WritePort(unsigned int portValue) {
//    printf("Writing to port: %02X\n", portValue);
    return true; // Assume success
}

bool ReadPort(unsigned int *returnvalue) {
    *returnvalue = 0x5A; 
//    printf("Reading from port: %02X\n", *returnvalue);
    return true; 
}

void I2C_Init(void) {
    I2C1BRG = (FCY / (2 * I2C1_BAUD_RATE)) - 1; // Set baud rate
    I2C1CONbits.I2CEN = 1; 
    return;
}

void I2C_Start(void) {
    I2C1CONbits.SEN = 1; 
    while (I2C1CONbits.SEN); 
    return;
}

void I2C_Stop(void) {
    I2C1CONbits.PEN = 1; 
    while (I2C1CONbits.PEN); 
    return; 
}

uint8_t I2C_Write(uint8_t data) {
    I2C1TRN = data; 
    while (I2C1STATbits.TRSTAT);
    if (I2C1STATbits.ACKSTAT) {
    }
    return I2C1STATbits.ACKSTAT; 
}

void MCP4725_SetOutput(uint16_t dacValue) {
    uint8_t upperData = (dacValue >> 4) & 0xFF; // Upper 8 bits
    uint8_t lowerData = (dacValue & 0x0F) << 4; // Lower 4 bits

    I2C_Start(); // Begin I2C communication

    if (I2C_Write(MCP4725_ADDRESS << 1) != 0) { 
//      
        I2C_Stop();
        return; 
    }
    
    if (I2C_Write(0x40) != 0) { 
        I2C_Stop();
        return;
    }
    
    if (I2C_Write(upperData) != 0) {
        I2C_Stop();
        return;
    }
    
    if (I2C_Write(lowerData) != 0) {
        I2C_Stop();
        return;
    }

    I2C_Stop(); // End communication
    printf("MCP4725 output set to: %u\n", dacValue);
}

void BlinkLED(uint16_t duration) {
    LED_PIN = 1; 
    __delay_ms(duration); 
    LED_PIN = 0; 
    return; 
}

void initInterrupt(void) {
    TRISBbits.TRISB12 = 1; // Set RB12 as input for CN interrupt
    CNENBbits.CNIEB12 = 1; // Enable CN interrupt for RB12
    IPC4bits.CNIP = 3;     // Set interrupt priority
    IFS1bits.CNIF = 0;     // Clear CN interrupt flag
    IEC1bits.CNIE = 1;     // Enable change notification interrupt
    __builtin_enable_interrupts(); // Enable global interrupts
}

void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void) {
    if (IFS1bits.CNIF) {
        count++; 
        IFS1bits.CNIF = 0; 
    }
}

void ProcessCount(void) {
    if (count > 0) {
        BlinkLED(100); 
        count = 0; 
    }
    return; 
}

void ADC_Init(void) {

    ANSELAbits.ANSA1 = 1; // Set RA1 as analog input

    TRISAbits.TRISA1 = 1; // Set RA1 as input
    AD1CON1bits.SAMP = 0; // Disable sampling
    AD1CON1bits.ASAM = 1; // Manual/auto sampling - auto sampling
    AD1CON2bits.SMPI = 0; // Interrupt after one sample
    AD1CON3bits.ADCS = 0b111; // ADC sample time (e.g., 15 TAD)
    AD1CON1bits.ADON = 1; // Turn on the ADC
}

uint16_t ADC_Read(void) {
    AD1CON1bits.SAMP = 1;
    __delay_us(10); // Wait for the sampling time
    AD1CON1bits.SAMP = 0; // Stop sampling
    AD1CON1bits.ADON = 1; // Start conversion
    while (!AD1CON1bits.DONE); // Wait for conversion to complete
    
    return ADC1BUF0;
}

void initRA10() {
    TRISAbits.TRISA10 = 0;  
}

void controlLED(int input) {
    if (input == 1) {
        LATAbits.LATA10 = 1; 
    } else {
        LATAbits.LATA10 = 0;  
    }
}

char UART_Receive(void) {
    while (!U1STAbits.URXDA);    // Wait until data is available
    return U1RXREG;              // Return received data
}

bool UART_Available(void) {
    return U1STAbits.URXDA;
}

void processCommand(char command) {
    static bool counting_enabled = false;
    
    switch(command) {
        case 's':  
        case 'S':
            counting_enabled = true;
            controlLED(1);  
            printf("Counting started\n");
            break;
            
        case 'p':  
        case 'P':
            counting_enabled = false;
            controlLED(0);  
            printf("Counting stopped\n");
            break;
            
        case 'r':   
        case 'R':
            count = 0;
            printf("Count reset to 0\n");
            break;
            
        default:
            printf("Invalid command. Available commands:\n");
            printf("s - Start counting\n");
            printf("p - Pause counting\n");
            printf("r - Reset count\n");
    }
}

uint32_t __get_ms(void) {
    static uint32_t ms_counter = 0;
    ms_counter++;
    __delay_ms(1);
    return ms_counter;
}

int main(void) {
    // Initialize all required systems
    init_oscillator();
    initRA10();
    I2C_Init();
    InitMCP2200(0x04D8, 0x00DF);
    initInterrupt();
    init_uart();
    ADC_Init();
    
    // Set initial high voltage
    MCP4725_SetOutput(HIGH_VOLTAGE_SETTING); 

    // Configure MCP2200
    if (!ConfigureMCP2200(0x43, UART_BAUDRATE, BLINKFAST, BLINKSLOW, false, false, false)) {
        printf("MCP2200 configuration failed. Error: %d\n", LastError);
        return 1;
    }

    printf("GM Detector System Ready\n");
    printf("Commands:\n");
    printf("s - Start counting\n");
    printf("p - Pause counting\n");
    printf("r - Reset count\n");

    while (1) {
        // Check for user input
        if (UART_Available()) {
            char command = UART_Receive();
            processCommand(command);
        }

        // Process and display count every second
        static uint32_t lastPrintTime = 0;
        if (__get_ms() - lastPrintTime >= 1000) {  // Print every 1 second
            printf("Count: %u\n", count);
            uint16_t adcValue = ADC_Read();
            printf("ADC Value: %u\n", adcValue);
            lastPrintTime = __get_ms();
        }

        // Other system tasks can continue here
        ProcessCount();
    }

    return 0;
}