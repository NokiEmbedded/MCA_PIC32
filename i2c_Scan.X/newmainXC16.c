#pragma config FNOSC = FRCPLL    // Internal Fast RC oscillator with PLL
#pragma config FSOSCEN = OFF     // Secondary Oscillator Disable
#pragma config POSCMOD = OFF     // Primary oscillator disabled
#pragma config OSCIOFNC = OFF    // CLKO Output Signal Disable
#pragma config FPBDIV = DIV_1    // Peripheral Bus Clock = System Clock
#pragma config FWDTEN = OFF      // Watchdog Timer Disable
#pragma config WDTPS = PS1       // Watchdog Timer Postscale
#pragma config FCKSM = CSDCMD    // Clock Switching & Fail Safe Clock Monitor Disable
#pragma config ICESEL = ICS_PGx1 // ICE/ICD Comm Channel Select
#pragma config PWP = OFF         // Program Flash Write Protect Disable
#pragma config BWP = OFF         // Boot Flash Write Protect Disable
#pragma config CP = OFF          // Code Protect Disable
#pragma config FPLLMUL = MUL_15  // PLL Multiplier (15x)
#pragma config FPLLIDIV = DIV_2  // PLL Input Divider (2x Divider)
#pragma config FPLLODIV = DIV_1  // PLL Output Divider (1x Divider)

#include <xc.h>
#include <sys/attribs.h>
#include <stdbool.h>

#define SYSCLK  60000000L
#define PBCLK   60000000L
#define UART_BAUD 115200
#define I2C_CLOCK_FREQ 100000

volatile char rxData;
volatile bool dataReceived = false;

void SYSTEM_Init(void) {
    __builtin_disable_interrupts();
    
    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    OSCCONbits.NOSC = 0x0001;
    OSCCONbits.FRCDIV = 0;
    OSCCONbits.PBDIV = 0;
    
    while(OSCCONbits.OSWEN != 0);
    while(OSCCONbits.SLOCK != 1);
    
    SYSKEY = 0x33333333;
    
    __builtin_enable_interrupts();
}

void GPIO_Init(void) {
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    
    TRISAbits.TRISA4 = 1;
    TRISAbits.TRISA0 = 0;
    
    RPA0Rbits.RPA0R = 0b0001;
    RPA4R = 0b0010;
}

void UART1_Init(void) {
    U1MODEbits.ON = 0;
    U1BRG = ((PBCLK / (16 * UART_BAUD)) - 1);
    
    U1MODE = 0x0000;
    U1MODEbits.BRGH = 0;
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;
    
    U1STAbits.URXEN = 1;
    U1STAbits.UTXEN = 1;
    
    IPC8bits.U1IP = 2;
    IPC8bits.U1IS = 0;
    IFS1bits.U1RXIF = 0;
    IEC1bits.U1RXIE = 1;
    
    U1MODEbits.ON = 1;
    
    delay_ms(1);
}

void I2C1_Init(void) {
    I2C1CONbits.ON = 0;
    I2C1BRG = (PBCLK / (2 * I2C_CLOCK_FREQ)) - 2;
    
    I2C1CONbits.SIDL = 0;
    I2C1CONbits.DISSLW = 1;
    I2C1CONbits.ACKDT = 0;
    
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB9 = 1;
    
    I2C1CONbits.ON = 1;
}

void UART1_Write(unsigned char data) {
    while(U1STAbits.UTXBF);
    U1TXREG = data;
}

void UART1_WriteString(const char* str) {
    while(*str != '\0') {
        UART1_Write(*str++);
    }
}

void delay_ms(unsigned int ms) {
    unsigned int ticks = ms * 60000;
    for(unsigned int i = 0; i < ticks; i++) {
        asm("nop");
    }
}

bool I2C1_Start(void) {
    if(I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || 
       I2C1CONbits.RCEN || I2C1CONbits.ACKEN) {
        return false;
    }
    
    I2C1CONbits.SEN = 1;
    while(I2C1CONbits.SEN);
    return true;
}

void I2C1_Stop(void) {
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN);
}

bool I2C1_WriteAddress(unsigned char address) {
    I2C1TRN = address;
    while(I2C1STATbits.TRSTAT);
    return !I2C1STATbits.ACKSTAT;
}

void PrintHex(unsigned char value) {
    char hexChars[] = "0123456789ABCDEF";
    char hexStr[6];
    
    hexStr[0] = '0';
    hexStr[1] = 'x';
    hexStr[2] = hexChars[(value >> 4) & 0x0F];
    hexStr[3] = hexChars[value & 0x0F];
    hexStr[4] = '\r';
    hexStr[5] = '\n';
    
    for(int i = 0; i < 6; i++) {
        UART1_Write(hexStr[i]);
    }
}

void I2C1_ScanBus(void) {
    bool deviceFound = false;
    
    UART1_WriteString("Starting I2C scan...\r\n");
    
    for(unsigned char address = 0x08; address < 0x78; address++) {
        if(!I2C1_Start()) {
            UART1_WriteString("Start condition failed\r\n");
            continue;
        }
        
        if(I2C1_WriteAddress((address << 1) | 0)) {
            UART1_WriteString("Device found at address: ");
            PrintHex(address);
            deviceFound = true;
        }
        
        I2C1_Stop();
        delay_ms(5);
    }
    
    if(!deviceFound) {
        UART1_WriteString("No I2C devices found\r\n");
    }
    UART1_WriteString("Scan complete\r\n\n");
}

void __ISR(_UART_1_VECTOR, IPL2AUTO) UART1_RX_Handler(void) {
    if(U1STAbits.URXDA) {
        rxData = U1RXREG;
        dataReceived = true;
    }
    IFS1bits.U1RXIF = 0;
}

int main(void) {
    SYSTEM_Init();
    GPIO_Init();
    UART1_Init();
    I2C1_Init();
    
    delay_ms(500);
    UART1_WriteString("PIC32 I2C Scanner Ready\r\n");
    UART1_WriteString("Send 's' to start scan\r\n");
    
    while(1) {
        if(dataReceived) {
            if(rxData == 's' || rxData == 'S') {
                I2C1_ScanBus();
            }
            dataReceived = false;
        }
        delay_ms(1000);
    }
    
    return 0;
}