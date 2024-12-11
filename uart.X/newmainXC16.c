// Configuration Bits
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
#include <string.h>

// Define frequencies
#define SYSCLK  60000000L
#define PBCLK   60000000L
#define UART_BAUD 115200

// Buffer size for commands
#define CMD_SIZE 32

// Command buffer
char cmdBuffer[CMD_SIZE];
unsigned int cmdIndex = 0;

// Function prototypes
void SYSTEM_Init(void);
void GPIO_Init(void);
void UART1_Init(void);
void UART1_Write(unsigned char data);
void UART1_WriteString(const char* str);
char UART1_Read(void);
unsigned char UART1_DataReady(void);
void processCommand(const char* cmd);
void delay_ms(unsigned int ms);

// Simple delay function
void delay_ms(unsigned int ms) {
    unsigned int i;
    unsigned int count = ms * (SYSCLK / 2000000);
    for(i = 0; i < count; i++) {
        asm("nop");
    }
}

// System initialization
void SYSTEM_Init(void) {
    // Unlock system for clock configuration
    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    // Configure OSCCON
    OSCCONbits.NOSC = 0x0001;     // FRC with PLL
    OSCCONbits.FRCDIV = 0;        // FRC divider = 1
    OSCCONbits.PBDIV = 0;         // Peripheral bus clock = SYSCLK
    
    while(OSCCONbits.OSWEN != 0); // Wait for switch
    while(OSCCONbits.SLOCK != 1); // Wait for PLL lock
    
    SYSKEY = 0x33333333;          // Lock system configuration
}

// GPIO initialization
void GPIO_Init(void) {
    // Unlock PPS
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    // Configure UART pins (RB13 for RX, RB15 for TX)
    TRISAbits.TRISA4 = 1;    // RB13 (RX) as input
    TRISAbits.TRISA0 = 0;    // RB15 (TX) as output
    
//    ANSELBbits.ANSB13 = 0;    // Disable analog for RB13
//    ANSELBbits.ANSB15 = 0;    // Disable analog for RB15
    
    // Map UART1 to the pins
    U1RXR = 0b0010;           // RB13 as U1RX
    RPA0R = 0b0001;          // RB15 as U1TX
    
    SYSKEY = 0x33333333;      // Lock PPS
}

// UART initialization
void UART1_Init(void) {
    // Disable UART1 before configuration
    U1MODEbits.ON = 0;
    
    // Clear UART1 configuration
    U1MODE = 0;
    U1STA = 0;
    
    // Calculate and set baud rate
    U1BRG = ((PBCLK / (16 * UART_BAUD)) - 1);
    
    // Configure UART1 mode
    U1MODEbits.BRGH = 0;      // Standard Speed mode
    U1MODEbits.PDSEL = 0;     // 8-bit data, no parity
    U1MODEbits.STSEL = 0;     // 1 stop bit
    
    // Enable TX and RX
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    
    // Enable UART1
    U1MODEbits.ON = 1;
    
    delay_ms(1);
}

// Write single character to UART
void UART1_Write(unsigned char data) {
    while(U1STAbits.UTXBF);   // Wait until transmit buffer is not full
    U1TXREG = data;
    while(!U1STAbits.TRMT);   // Wait until transmit shift register is empty
}

// Write string to UART
void UART1_WriteString(const char* str) {
    while(*str != '\0') {
        UART1_Write(*str++);
    }
}

// Check if UART data is available
unsigned char UART1_DataReady(void) {
    return U1STAbits.URXDA;
}

// Read character from UART
char UART1_Read(void) {
    if(U1STAbits.OERR) {          // Clear overrun error
        U1STAbits.OERR = 0;
    }
    
    while(!U1STAbits.URXDA);      // Wait for data to be available
    return U1RXREG;               // Return received data
}

// Process commands
void processCommand(const char* cmd) {
    // Convert command to uppercase for case-insensitive comparison
    char upperCmd[CMD_SIZE];
    strcpy(upperCmd, cmd);
    for(int i = 0; upperCmd[i]; i++) {
        if(upperCmd[i] >= 'a' && upperCmd[i] <= 'z') {
            upperCmd[i] = upperCmd[i] - 32;
        }
    }
    
    if(strcmp(upperCmd, "WHO") == 0) {
        UART1_WriteString("Hi, I'm PIC32 Microcontroller!\r\n");
        UART1_WriteString("Running at 60MHz with UART at 115200 baud\r\n");
    }
    else if(strcmp(upperCmd, "STATUS") == 0) {
        UART1_WriteString("System Status:\r\n");
        UART1_WriteString("- CPU: Running\r\n");
        UART1_WriteString("- UART: Active\r\n");
        UART1_WriteString("- Buffer: OK\r\n");
    }
    else if(strcmp(upperCmd, "HELP") == 0) {
        UART1_WriteString("Available Commands:\r\n");
        UART1_WriteString("WHO     - Display device information\r\n");
        UART1_WriteString("STATUS  - Show system status\r\n");
        UART1_WriteString("HELP    - Show this help message\r\n");
//        UART1_WriteString("RESET   - Software reset the system\r\n");
    }
//    else if(strcmp(upperCmd, "RESET") == 0) {
//        UART1_WriteString("Performing software reset...\r\n");
//        delay_ms(100);
//        asm volatile("reset");
//    }
    else {
        UART1_WriteString("Unknown command: ");
        UART1_WriteString(cmd);
        UART1_WriteString("\r\nType 'HELP' for available commands.\r\n");
    }
}

int main(void) {
    // Initialize system
    SYSTEM_Init();
    GPIO_Init();
    UART1_Init();
    
    // Initial message
    UART1_WriteString("\r\nPIC32 Command Interface Ready\r\n");
    UART1_WriteString("Type 'HELP' for available commands\r\n");
    
    while(1) {
        if(UART1_DataReady()) {
            char data = U1RXREG;
            
            // Echo received character
            UART1_Write(data);
            
            // Process the received character
            if(data == '\r' || data == '\n') {
                if(cmdIndex > 0) {
                    cmdBuffer[cmdIndex] = '\0';
                    UART1_WriteString("\r\n");
                    processCommand(cmdBuffer);
                    cmdIndex = 0;
                }
            }
            else if(data == '\b' || data == 0x7F) {  // Backspace or Delete
                if(cmdIndex > 0) {
                    cmdIndex--;
                    UART1_WriteString("\b \b");
                }
            }
            else if(cmdIndex < CMD_SIZE - 1) {
                cmdBuffer[cmdIndex++] = data;
            }
        }
    }
    
    return 0;
}