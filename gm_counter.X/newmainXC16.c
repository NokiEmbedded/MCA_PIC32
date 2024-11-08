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

#define SLAVE_ADDRESS 0x50

#define CMD_START 0x01
#define CMD_STOP  0x02
#define CMD_RESET 0x03
#define CMD_SET_VOLTAGE 0x04  // New command for setting voltage 
#define CMD_GET_VOLTAGE 0x05  // New command for reading voltage

#define UART_BUFFER_SIZE 64
#define START_CMD "START\r\n"
#define STOP_CMD "STOP\r\n"
#define RESET_CMD "RESET\r\n"
#define SET_VOLTAGE_CMD "VOLTAGE="

volatile uint32_t pulseCount = 0;
volatile uint8_t isCountingEnabled = 0;
volatile uint16_t currentVoltage = 0;  // Store ADC reading
volatile uint16_t targetVoltage = 0;

volatile char uartBuffer[UART_BUFFER_SIZE];
volatile uint8_t bufferIndex = 0;
volatile bool systemEnabled = false;


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
    RPOR2bits.RP39R = 1;      // Map U1TX to RP39
    RPINR18bits.U1RXR = 40;   // Map U1RX to RP40
    __builtin_write_OSCCONL(OSCCON | 0x40);
    
    U1MODEbits.UARTEN = 0;    // Disable UART while configuring
    U1MODEbits.BRGH = 0;      // Standard speed mode
    U1BRG = ((FCY / (16 * UART_BAUDRATE)) - 1);
    
    // Configure UART receive interrupt
    IEC0bits.U1RXIE = 1;      // Enable UART1 RX interrupt
    IPC2bits.U1RXIP = 5;      // Set interrupt priority (1-7)
    IFS0bits.U1RXIF = 0;      // Clear interrupt flag
    
    U1STAbits.URXISEL = 0;    // Interrupt when any character is received
    U1STAbits.UTXEN = 1;      // Enable transmit
    U1MODEbits.UARTEN = 1;    // Enable UART
    
//    printf("UART Initialized. Waiting for commands...\n");
//    printf("Available commands:\n");
//    printf("START - Start the system\n");
//    printf("STOP - Stop the system\n");
//    printf("RESET - Reset pulse count\n");
//    printf("VOLTAGE=xxxx - Set voltage (0-4095)\n");
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
//        BlinkLED(100); // Blink LED for 100 ms
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


void initI2C2Slave(void) {
    // Disable I2C2 module during setup
    I2C2CONbits.I2CEN = 0;    
    
    // Reset I2C2 module
    I2C2CON = 0;
    I2C2STAT = 0;
    
    // Clear interrupt flags
    IFS3bits.MI2C2IF = 0;
    IFS3bits.SI2C2IF = 0;
    
    // Set slave address
    I2C2ADD = SLAVE_ADDRESS;
    
    // Configure I2C2 in slave mode
    I2C2CONbits.A10M = 0;     // 7-bit address mode
    I2C2CONbits.SCLREL = 1;   // Release SCL
    I2C2CONbits.STREN = 1;    // Enable clock stretching
    I2C2CONbits.GCEN = 0;     // Disable general call
    I2C2CONbits.DISSLW = 1;   // Disable slew rate control
    
    // Configure interrupts
    IEC3bits.MI2C2IE = 1;     // Master interrupt enable
    IEC3bits.SI2C2IE = 1;     // Slave interrupt enable
    IPC12bits.SI2C2IP = 5;    // Set slave interrupt priority
    
    // Enable I2C module
    I2C2CONbits.I2CEN = 1;    
}

void initLED(void) {
    // Configure RA10 as output for LED
    TRISAbits.TRISA10 = 0;    // Set as output
    LATAbits.LATA10 = 0;      // Initially off
}

void __attribute__((interrupt, no_auto_psv)) _MI2C2Interrupt(void) {
    IFS3bits.MI2C2IF = 0;     // Clear master interrupt flag
}

void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void) {
    char receivedChar;
    
    while (U1STAbits.URXDA) {  // While there is data in the receive buffer
        receivedChar = U1RXREG;
        
        if (receivedChar == '\n' || receivedChar == '\r') {
            // Null terminate the string
            uartBuffer[bufferIndex] = '\0';
            
            // Process the command
            processUARTCommand();
            
            // Reset buffer index
            bufferIndex = 0;
        }
        else if (bufferIndex < UART_BUFFER_SIZE - 1) {
            uartBuffer[bufferIndex++] = receivedChar;
        }
    }
    
    IFS0bits.U1RXIF = 0;  // Clear interrupt flag
}

void __attribute__((interrupt, no_auto_psv)) _SI2C2Interrupt(void) {
    static uint8_t receivedBytes[3];  // Buffer for receiving command + data
    static uint8_t receiveIndex = 0;
    static uint8_t byteToSend = 0;
    static uint8_t sendIndex = 0;
    static uint8_t expectedBytes = 0;
    static uint8_t currentCommand = 0;
    
    // Address match event
    if(I2C2STATbits.D_A == 0) {
        // Clear receive buffer
        uint8_t dummy = I2C2RCV;
        receiveIndex = 0;
        sendIndex = 0;
        
        // If it's a read operation based on previous command
        if(I2C2STATbits.R_W == 1) {
            switch(currentCommand) {
                case CMD_GET_VOLTAGE:
                    // Send high byte of voltage first
                    byteToSend = (currentVoltage >> 8) & 0xFF;
                    break;
                    
                default:  // Normal count reading
                    // Send highest byte of count first
                    byteToSend = (pulseCount >> 24) & 0xFF;
                    break;
            }
            while(I2C2STATbits.TBF);
            I2C2TRN = byteToSend;
        }
    }
    // Data event
    else {
        if(I2C2STATbits.R_W == 0) {  // Master Write
            receivedBytes[receiveIndex] = I2C2RCV;
            
            if(receiveIndex == 0) {
                currentCommand = receivedBytes[0];
                // Determine how many additional bytes to expect
                switch(currentCommand) {
                    case CMD_SET_VOLTAGE:
                        expectedBytes = 2;  // Expect 2 more bytes for voltage value
                        break;
                    default:
                        expectedBytes = 0;  // Single byte commands
                        break;
                }
            }
            
            if(receiveIndex == expectedBytes) {
                // Process complete command
                switch(currentCommand) {
                    case CMD_START:
                        isCountingEnabled = 1;
                        LATAbits.LATA10 = 1;
                        break;
                        
                    case CMD_STOP:
                        isCountingEnabled = 0;
                        LATAbits.LATA10 = 0;
                        break;
                        
                    case CMD_RESET:
                        pulseCount = 0;
                        break;
                        
                    case CMD_SET_VOLTAGE:
                        // Combine two bytes into 16-bit voltage value
                        targetVoltage = (receivedBytes[1] << 8) | receivedBytes[2];
                        // Add your code here to set the actual voltage using DAC/PWM
                        break;
                }
                receiveIndex = 0;
            } else {
                receiveIndex++;
            }
        }
        else {  // Master Read
            switch(currentCommand) {
                case CMD_GET_VOLTAGE:
                    if(sendIndex == 0) {
                        byteToSend = currentVoltage & 0xFF;  // Send low byte
                    }
                    break;
                    
                default:  // Normal count reading
                    sendIndex++;
                    if(sendIndex < 4) {
                        byteToSend = (pulseCount >> (24 - (sendIndex * 8))) & 0xFF;
                    }
                    break;
            }
            
            if((sendIndex < 4 && currentCommand != CMD_GET_VOLTAGE) || 
               (sendIndex < 1 && currentCommand == CMD_GET_VOLTAGE)) {
                while(I2C2STATbits.TBF);
                I2C2TRN = byteToSend;
            }
        }
    }
    
    // Release clock stretching
    I2C2CONbits.SCLREL = 1;
    IFS3bits.SI2C2IF = 0;
}

void processUARTCommand(void) {
    if (strncmp((const char*)uartBuffer, START_CMD, strlen(START_CMD)-2) == 0) {
        systemEnabled = true;
        isCountingEnabled = 1;
        LATAbits.LATA10 = 1;  // Turn on LED
        printf("System Started\n");
    }
    else if (strncmp((const char*)uartBuffer, STOP_CMD, strlen(STOP_CMD)-2) == 0) {
        systemEnabled = false;
        isCountingEnabled = 0;
        LATAbits.LATA10 = 0;  // Turn off LED
        printf("System Stopped\n");
    }
    else if (strncmp((const char*)uartBuffer, RESET_CMD, strlen(RESET_CMD)-2) == 0) {
        pulseCount = 0;
        count = 0;
        printf("Counters Reset\n");
    }
    else if (strncmp((const char*)uartBuffer, SET_VOLTAGE_CMD, strlen(SET_VOLTAGE_CMD)) == 0) {
        uint16_t voltage = atoi((const char*)&uartBuffer[strlen(SET_VOLTAGE_CMD)]);
        if (voltage <= 4095) {  // 12-bit DAC
            MCP4725_SetOutput(voltage);
            printf("Voltage set to: %u\n", voltage);
        } else {
            printf("Invalid voltage value. Range: 0-4095\n");
        }
    }
    else {
        printf("Invalid command. Available commands:\n");
        printf("START - Start the system\n");
        printf("STOP - Stop the system\n");
        printf("RESET - Reset pulse count\n");
        printf("VOLTAGE=xxxx - Set voltage (0-4095)\n");
    }
}


int main(void) {
    // Initialize all peripherals
    init_oscillator();
    initRA10();
    I2C_Init();
    initLED();
    TRISAbits.TRISA10 = 0;
    LATAbits.LATA10 = 0;
    initI2C2Slave();
    INTCON2bits.GIE = 1;
    InitMCP2200(0x04D8, 0x00DF);
    initInterrupt();
    init_uart();
    ADC_Init();

    // Initial DAC setting
    MCP4725_SetOutput(HIGH_VOLTAGE_SETTING);

    // Configure MCP2200
    if (!ConfigureMCP2200(0x43, UART_BAUDRATE, BLINKFAST, BLINKSLOW, false, false, false)) {
        printf("MCP2200 configuration failed. Error: %d\n", LastError);
        return 1;
    }

    printf("System initialized. Send 'START' to begin...\n");

    while (1) {
        if (systemEnabled) {
            // Print count only if system is enabled
            printf("%u\n", count);
            ProcessCount();

            // Read ADC value
            uint16_t adcValue = ADC_Read();

            // Process any received data or perform other tasks
            fnRxLED(BLINKFAST);
            fnTxLED(BLINKSLOW);

            // Read port status
            unsigned int portValue;
            ReadPort(&portValue);
        }
        __delay_ms(1000);
    }

    return 0;
}