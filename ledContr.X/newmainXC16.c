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
    
//    printf("Oscillator Initialized... ");
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
//    printf("Initializing MCP2200 with Vendor ID: %X, Product ID: %X\n", VendorID, ProductID);
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
    I2C1CONbits.I2CEN = 1; // Enable I2C1
//    printf("I2C_Init completed\n");
    return; // Explicit return
}

void I2C_Start(void) {
    I2C1CONbits.SEN = 1; // Initiate Start condition
    while (I2C1CONbits.SEN); // Wait until Start condition is generated
//    printf("I2C_Start completed\n");
    return; // Explicit return
}

void I2C_Stop(void) {
    I2C1CONbits.PEN = 1; // Initiate Stop condition
    while (I2C1CONbits.PEN); // Wait until Stop condition is generated
//    printf("I2C_Stop completed\n");
    return; // Explicit return
}

uint8_t I2C_Write(uint8_t data) {
    I2C1TRN = data; // Load data to transmit register
//    printf("I2C Write data: %u\n", data);
    while (I2C1STATbits.TRSTAT);
    if (I2C1STATbits.ACKSTAT) {
//        printf("ACK failure\n");
    }
//    printf("I2C_Write completed, ACK status: %u\n", I2C1STATbits.ACKSTAT);
    return I2C1STATbits.ACKSTAT; // Return ACK status
}

void MCP4725_SetOutput(uint16_t dacValue) {
    uint8_t upperData = (dacValue >> 4) & 0xFF; // Upper 8 bits
    uint8_t lowerData = (dacValue & 0x0F) << 4; // Lower 4 bits

    I2C_Start(); // Begin I2C communication

    if (I2C_Write(MCP4725_ADDRESS << 1) != 0) { // Write address
//        printf("Failed to acknowledge MCP4725 address\n");
        I2C_Stop();
        return; 
    }
    
    if (I2C_Write(0x40) != 0) { // Control byte
//        printf("Failed to acknowledge control byte\n");
        I2C_Stop();
        return;
    }
    
    if (I2C_Write(upperData) != 0) { // Upper data byte
//        printf("Failed to acknowledge upper data\n");
        I2C_Stop();
        return;
    }
    
    if (I2C_Write(lowerData) != 0) { // Lower data byte
//        printf("Failed to acknowledge lower data\n");
        I2C_Stop();
        return;
    }

    I2C_Stop(); // End communication
    printf("MCP4725 output set to: %u\n", dacValue);
}

//void ScanI2CDevices(void) {
//    printf("Scanning I2C devices...\n");
//    for (uint8_t address = 1; address < 127; address++) {
//        I2C_Start(); 
//        if (I2C_Write(address << 1) == 0) { 
//            printf("Found I2C device at address 0x%02X\n", address);
//        }
//        I2C_Stop(); 
//    }
//}
void BlinkLED(uint16_t duration) {
    LED_PIN = 1; 
    __delay_ms(duration); 
    LED_PIN = 0; // Turn off LED
//    printf("BlinkLED completed\n");
    return; // Explicit return
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
        count++; // Increment the count on interrupt
        IFS1bits.CNIF = 0; // Clear the interrupt flag
    }
}

void ProcessCount(void) {
    if (count > 0) {
        BlinkLED(100); // Blink LED for 100 ms
        count = 0; // Reset count after processing
    }
//    printf("ProcessCount completed\n");
    return; // Explicit return
}

void ADC_Init(void) {

    ANSELAbits.ANSA1 = 1; // Set RA1 as analog input

    TRISAbits.TRISA1 = 1; // Set RA1 as input
    AD1CON1bits.SAMP = 0; // Disable sampling
    AD1CON1bits.ASAM = 1; // Manual/auto sampling
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
    TRISAbits.TRISA10 = 0;  // Set RA10 as an output
}

// Function to control LED on RA10
void controlLED(int input) {
    if (input == 1) {
        LATAbits.LATA10 = 1;  // Turn on LED
    } else {
        LATAbits.LATA10 = 0;  // Turn off LED
    }
}

int main(void) {
    // Initialize all required systems
    init_oscillator();
    initRA10();
    I2C_Init();
    InitMCP2200(0x04D8, 0x00DF);
    initInterrupt();
    init_uart();
    ADC_Init();  // Added ADC initialization
    
    // Set initial high voltage
    MCP4725_SetOutput(HIGH_VOLTAGE_SETTING); 

    // Configure MCP2200
    if (!ConfigureMCP2200(0x43, UART_BAUDRATE, BLINKFAST, BLINKSLOW, false, false, false)) {
        printf("MCP2200 configuration failed. Error: %d\n", LastError);
        return 1;  // Exit if configuration fails
    }

    // Initialize LED state
    int ledState = 0;
    uint16_t adcValue = 0;

    printf("System initialized and running...\n");

    while (1) {
        // Toggle LED state
//        printf("System entered into while loop");
        ledState = !ledState;
        controlLED(ledState);

        // Read and process GM tube events
//        printf("Count: %u\n", count);
        printf("%u\n", count);

        ProcessCount();

        // Read ADC value
        adcValue = ADC_Read();
//        printf("ADC Converted Value: %u\n", adcValue);

        // Process any received data or perform other tasks
        fnRxLED(BLINKFAST);
        fnTxLED(BLINKSLOW);
        
        // Read port status
        unsigned int portValue;
        ReadPort(&portValue);
//        printf("Port Value: 0x%X\n", portValue);

        // Delay to control loop timing
        __delay_ms(1000);
    }

    return 0;  // This line will never be reached due to infinite loop
}
//int main(void) {
//    init_oscillator();
//    I2C_Init();
//    InitMCP2200(0x04D8, 0x00DF);
//    initInterrupt(); // Initialize interrupts
//    init_uart();
//    MCP4725_SetOutput(HIGH_VOLTAGE_SETTING); 
//    if (ConfigureMCP2200(0x43, UART_BAUDRATE, BLINKFAST, BLINKSLOW, false, false, false)) {
////        printf("MCP2200 configured successfully.\n");
//    } else {
////        printf("MCP2200 configuration failed. Error: %d\n", LastError);
//    }
//    
//    printf("System running...\n");
//    printf("Count: %u\n", count);
//    ProcessCount();
//    
//    uint16_t adcValue = ADC_Read();
//    printf("Adc Converted Value : %u\n", adcValue);
//
//    while (1) {
////        uint16_t adcValue = ADC_Read();
//        printf("System running...\n");
//        printf("Count: %u\n", count);
//        ProcessCount();
//    }
//    fnRxLED(BLINKFAST);
//    fnTxLED(BLINKSLOW);
//    WritePort(0x5A);
//    unsigned int portValue;
//    ReadPort(&portValue);
//    
//    __delay_ms(2000);
//    return 0;
//}
