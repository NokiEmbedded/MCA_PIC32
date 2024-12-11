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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Define frequencies and constants
#define SYSCLK  60000000L
#define PBCLK   60000000L
#define UART_BAUD 115200
#define I2C_FREQ 100000  // 100kHz I2C frequency
#define DAC_ADDR 0x0C    // I2C address for AD5647RVRNZ (adjust as needed)
#define CMD_SIZE 64      // Increased buffer size for longer commands

// Command buffer
char cmdBuffer[CMD_SIZE];
unsigned int cmdIndex = 0;

#define RESPONSE_SIZE 512

// Function prototypes (including new ones)
void SYSTEM_Init(void);
void GPIO_Init(void);
void UART1_Init(void);
void I2C1_Init(void);
void I2C2_Init(void);
void I2C2_Start(void);
void I2C2_Stop(void);
void I2C2_Write(unsigned char data);
unsigned char I2C2_Read(unsigned char ack);
void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Write(unsigned char data);
unsigned char I2C1_Read(unsigned char ack);
void setDAC_Voltage(unsigned int voltage);
unsigned int getDAC_Voltage(void);
void sendFPGA_Command(const char* cmd, unsigned int value);
void processCommand(const char* cmd);
bool checkDAC_Connection(void);
bool checkFPGA_Connection(void);

bool dacConnected = false;
bool fpgaConnected = false;

// System initialization remains the same


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

void GPIO_Init(void) {
    // Unlock PPS
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    // Configure UART pins
    TRISAbits.TRISA4 = 1;    // RX as input
    TRISAbits.TRISA0 = 0;    // TX as output
    
    // Configure I2C2 pins for DAC
    TRISBbits.TRISB3 = 0;    // SCL2
    TRISBbits.TRISB2 = 0;    // SDA2
    
    // Configure I2C1 pins for FPGA
    TRISBbits.TRISB8 = 0;   // SCL1
    TRISBbits.TRISB9 = 0;   // SDA1
    
    // Map pins
    U1RXR = 0b0010;          // RX
    RPA0R = 0b0001;          // TX
    
    SYSKEY = 0x33333333;     // Lock PPS
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

// I2C1 Initialization (for FPGA)
void I2C1_Init(void) {
    I2C1CON = 0;
    I2C1BRG = (PBCLK / (2 * I2C_FREQ)) - 2;
    I2C1CONbits.ON = 1;
}

// I2C1 Functions for FPGA communication
void I2C1_Start(void) {
    I2C1CONbits.SEN = 1;
    while(I2C1CONbits.SEN);
}

void I2C1_Stop(void) {
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN);
}

void I2C1_Write(unsigned char data) {
    I2C1TRN = data;
    while(I2C1STATbits.TRSTAT);
}

unsigned char I2C1_Read(unsigned char ack) {
    I2C1CONbits.RCEN = 1;
    while(!I2C1STATbits.RBF);
    unsigned char data = I2C1RCV;
    I2C1CONbits.ACKDT = !ack;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
    return data;
}

// I2C2 Initialization (for DAC)
void I2C2_Init(void) {
    I2C2CON = 0;
    I2C2BRG = (PBCLK / (2 * I2C_FREQ)) - 2;
    I2C2CONbits.ON = 1;
}

// I2C2 Functions
void I2C2_Start(void) {
    I2C2CONbits.SEN = 1;
    while(I2C2CONbits.SEN);
}

void I2C2_Stop(void) {
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);
}

void I2C2_Write(unsigned char data) {
    I2C2TRN = data;
    while(I2C2STATbits.TRSTAT);
}

unsigned char I2C2_Read(unsigned char ack) {
    I2C2CONbits.RCEN = 1;
    while(!I2C2STATbits.RBF);
    unsigned char data = I2C2RCV;
    I2C2CONbits.ACKDT = !ack;
    I2C2CONbits.ACKEN = 1;
    while(I2C2CONbits.ACKEN);
    return data;
}

// Set DAC Voltage
void setDAC_Voltage(unsigned int voltage) {
    unsigned int dac_value = (voltage * 65535) / 1000; // Convert voltage to DAC value
    I2C2_Start();
    I2C2_Write(DAC_ADDR << 1);    // Write address
    I2C2_Write((dac_value >> 8) & 0xFF);  // High byte
    I2C2_Write(dac_value & 0xFF);         // Low byte
    I2C2_Stop();
}

// Get DAC Voltage
unsigned int getDAC_Voltage(void) {
    unsigned int dac_value;
    I2C2_Start();
    I2C2_Write((DAC_ADDR << 1) | 1);  // Read address
    dac_value = I2C2_Read(1) << 8;    // High byte
    dac_value |= I2C2_Read(0);        // Low byte
    I2C2_Stop();
    return (dac_value * 1000) / 65535; // Convert DAC value to voltage
}

// Send command to FPGA
void sendFPGA_Command(const char* cmd, unsigned int value) {
    I2C1_Start();
    I2C1_Write(0x50 << 1);  // FPGA address with write bit
    I2C1_Write(cmd[0]);     // Command identifier
    I2C1_Write((value >> 8) & 0xFF);  // High byte
    I2C1_Write(value & 0xFF);         // Low byte
    I2C1_Stop();
}

unsigned int readFPGA_Value(const char* cmd) {
    unsigned int value;
    
    I2C1_Start();
    I2C1_Write(0x50 << 1);       // FPGA address with write bit
    I2C1_Write(cmd[0]);          // Command identifier for what we want to read
    I2C1_Start();                // Repeated start
    I2C1_Write((0x50 << 1) | 1); // FPGA address with read bit
    
    value = I2C1_Read(1) << 8;   // Read high byte, send ACK
    value |= I2C1_Read(0);       // Read low byte, send NACK
    
    I2C1_Stop();
    return value;
}

// Enhanced command processing
// Global structure to store system status
typedef struct {
    int voltage;        // DAC voltage
    int courseGain;     // Course gain
    int fineGain;      // Fine gain
    int digitalGain;    // Digital gain
    int polarity;       // Input polarity
    int thresholdTime;  // Threshold time
    int riseTime;       // Rise time
    int flatTime;       // Flat time
    int poleZero;      // Pole-zero
    int digitalBL;      // Digital baseline
    int pileupReject;   // Pile-up reject
    int presetTime;     // Preset time
    int systemState;    // 0=stopped, 1=running
} SystemStatus;

// Global status variable
SystemStatus status = {
    .voltage = 0,
    .courseGain = 0,
    .fineGain = 0,
    .digitalGain = 0,
    .polarity = 0,
    .thresholdTime = 0,
    .riseTime = 0,
    .flatTime = 0,
    .poleZero = 0,
    .digitalBL = 0,
    .pileupReject = 0,
    .presetTime = 0,
    .systemState = 0
};

void processCommand(const char* cmd) {
    char command[16];
    char param1[16];
    char param2[16];
    unsigned int value;
    char response[128];
    
    // Initialize parameters to empty strings
    command[0] = param1[0] = param2[0] = '\0';
    
    // Parse command and parameters
    sscanf(cmd, "%[^,],%[^,],%s", command, param1, param2);
    
    // Convert command to uppercase
    for(int i = 0; command[i]; i++) {
        if(command[i] >= 'a' && command[i] <= 'z') {
            command[i] = command[i] - 32;
        }
    }
    
    // System Control Commands
    if(strcmp(command, "START") == 0) {
        I2C2_Start();
        I2C1_Start();
        status.systemState = 1;
        UART1_WriteString("System started\r\n");
    }
    else if(strcmp(command, "STOP") == 0) {
        I2C2_Stop();
        I2C1_Stop();
        status.systemState = 0;
        UART1_WriteString("System stopped\r\n");
    }
    else if(strcmp(command, "RESET") == 0) {
        setDAC_Voltage(0);
        sendFPGA_Command("RST", 0);
        // Reset all status values
        memset(&status, 0, sizeof(SystemStatus));
        UART1_WriteString("System reset complete\r\n");
    }
    else if(strcmp(command, "STATUS") == 0) {
        char response[RESPONSE_SIZE];

        // Output all values in a single line, comma-separated
        sprintf(response, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                status.systemState,     // 0=stopped, 1=running
                status.voltage,         // DAC voltage
                status.courseGain,      // Course gain
                status.fineGain,        // Fine gain
                status.digitalGain,     // Digital gain
                status.polarity,        // 0=negative, 1=positive
                status.thresholdTime,   // Threshold time in seconds
                status.riseTime,        // Rise time in ns
                status.flatTime,        // Flat time in ns
                status.poleZero,        // Pole-zero value
                status.digitalBL,       // Digital baseline
                status.pileupReject,    // Pile-up reject percentage
                status.presetTime);     // Preset time in seconds
        UART1_WriteString(response);
    }
    
    // DAC Commands
    else if(strcmp(command, "SETV") == 0) {
        value = atoi(param1);
        if(value > 0 && value <= 1000) {
            setDAC_Voltage(value);
            status.voltage = value;
            sprintf(response, "High voltage set to %d V\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Voltage must be between 1 and 1000V\r\n");
        }
    }
    else if(strcmp(command, "GETV?") == 0) {
        value = getDAC_Voltage();
        status.voltage = value;  // Update status with actual reading
        sprintf(response, "Current high voltage: %d V\r\n", value);
        UART1_WriteString(response);
    }
    
    // FPGA - ADC Gain Commands
    else if(strcmp(command, "SETCG") == 0 || strcmp(command, "CG") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 63) {
            sendFPGA_Command("CG", value);
            status.courseGain = value;
            sprintf(response, "Course gain set to %d\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Course gain must be between 0 and 63\r\n");
        }
    }
    else if(strcmp(command, "SETFG") == 0 || strcmp(command, "FG") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 255) {
            sendFPGA_Command("FG", value);
            status.fineGain = value;
            sprintf(response, "Fine gain set to %d\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Fine gain must be between 0 and 255\r\n");
        }
    }
    else if(strcmp(command, "SETDG") == 0 || strcmp(command, "DG") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 255) {
            sendFPGA_Command("DG", value);
            status.digitalGain = value;
            sprintf(response, "Digital gain set to %d\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Digital gain must be between 0 and 255\r\n");
        }
    }
    
    // FPGA - ADC Polarity Command
    else if(strcmp(command, "SETPOL") == 0 || strcmp(command, "POL") == 0) {
        value = atoi(param2);
        if(value == 0 || value == 1) {
            sendFPGA_Command("POL", value);
            status.polarity = value;
            sprintf(response, "Input polarity set to %s\r\n", value ? "positive" : "negative");
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Polarity must be 0 (negative) or 1 (positive)\r\n");
        }
    }
    
    // FPGA - Timing Commands
    else if(strcmp(command, "SETTT") == 0 || strcmp(command, "TT") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 1000) {
            sendFPGA_Command("TT", value);
            status.thresholdTime = value;
            sprintf(response, "Threshold time set to %d seconds\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Threshold time must be between 0 and 1000 seconds\r\n");
        }
    }
    else if(strcmp(command, "SETRT") == 0 || strcmp(command, "RT") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 10000) {
            sendFPGA_Command("RT", value);
            status.riseTime = value;
            sprintf(response, "Rise time set to %d ns\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Rise time must be between 0 and 10000 ns\r\n");
        }
    }
    else if(strcmp(command, "SETFT") == 0 || strcmp(command, "FT") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 10000) {
            sendFPGA_Command("FT", value);
            status.flatTime = value;
            sprintf(response, "Flat time set to %d ns\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Flat time must be between 0 and 10000 ns\r\n");
        }
    }
    
    // FPGA - Signal Processing Commands
    else if(strcmp(command, "SETPZ") == 0 || strcmp(command, "PZ") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 1000) {
            sendFPGA_Command("PZ", value);
            status.poleZero = value;
            sprintf(response, "Pole-zero set to %d\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Pole-zero must be between 0 and 1000\r\n");
        }
    }
    else if(strcmp(command, "SETDB") == 0 || strcmp(command, "DB") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 1000) {
            sendFPGA_Command("DB", value);
            status.digitalBL = value;
            sprintf(response, "Digital baseline set to %d\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Digital baseline must be between 0 and 1000\r\n");
        }
    }
    else if(strcmp(command, "SETPR") == 0 || strcmp(command, "PR") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 100) {
            sendFPGA_Command("PR", value);
            status.pileupReject = value;
            sprintf(response, "Pile-up reject set to %d%%\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Pile-up reject must be between 0 and 100\r\n");
        }
    }
    else if(strcmp(command, "SETPT") == 0 || strcmp(command, "PT") == 0) {
        value = atoi(param2);
        if(value >= 0 && value <= 86400) {
            sendFPGA_Command("PT", value);
            status.presetTime = value;
            sprintf(response, "Preset time set to %d seconds\r\n", value);
            UART1_WriteString(response);
        } else {
            UART1_WriteString("Error: Preset time must be between 0 and 86400 seconds\r\n");
        }
    }
    else if(strcmp(command, "HELP") == 0) {
        UART1_WriteString("\r\nAvailable Commands:\r\n");
        UART1_WriteString("System Control:\r\n");
        UART1_WriteString("  START  - Start DAC and FPGA\r\n");
        UART1_WriteString("  STOP   - Stop DAC and FPGA\r\n");
        UART1_WriteString("  RESET  - Reset system\r\n");
        UART1_WriteString("  STATUS - Display all current settings\r\n");
        UART1_WriteString("\r\nDAC Commands:\r\n");
        UART1_WriteString("  SETV,value    - Set high voltage (1-1000V)\r\n");
        UART1_WriteString("  GETV?         - Read current voltage\r\n");
        UART1_WriteString("\r\nFPGA-ADC Commands:\r\n");
        UART1_WriteString("  SETCG,cg,value - Set course gain (0-63)\r\n");
        UART1_WriteString("  SETFG,fg,value - Set fine gain (0-255)\r\n");
        UART1_WriteString("  SETDG,dg,value - Set digital gain (0-255)\r\n");
        UART1_WriteString("  SETPOL,pol,value - Set polarity (0=neg, 1=pos)\r\n");
        UART1_WriteString("  SETTT,tt,value - Set threshold time (0-1000s)\r\n");
        UART1_WriteString("  SETRT,rt,value - Set rise time (0-10000ns)\r\n");
        UART1_WriteString("  SETFT,ft,value - Set flat time (0-10000ns)\r\n");
        UART1_WriteString("  SETPZ,pz,value - Set pole-zero (0-1000)\r\n");
        UART1_WriteString("  SETDB,db,value - Set digital baseline (0-1000)\r\n");
        UART1_WriteString("  SETPR,pr,value - Set pile-up reject (0-100%)\r\n");
        UART1_WriteString("  SETPT,pt,value - Set preset time (0-86400s)\r\n");
    }
    else {
        UART1_WriteString("Unknown command. Type 'HELP' for available commands.\r\n");
    }
}

void performPOR(void) {
    // Reset all peripherals
    I2C1CONbits.ON = 0;
    I2C2CONbits.ON = 0;
    U1MODEbits.ON = 0;
    
    // Wait for stabilization
    delay_ms(100);
    
    // Reset status structure
    memset(&status, 0, sizeof(SystemStatus));
    
    // Re-initialize peripherals
    GPIO_Init();
//    I2C1_Init();
//    I2C2_Init();
    UART1_Init();
    
    // Check connections
    dacConnected = checkDAC_Connection();
    fpgaConnected = checkFPGA_Connection();
    
    UART1_WriteString("\r\nInitializing System:\r\n");
    if(dacConnected) {
        UART1_WriteString("DAC: Connected\r\n");
        setDAC_Voltage(0);
    } else {
        UART1_WriteString("DAC: Not Connected\r\n");
    }
    
    if(fpgaConnected) {
        UART1_WriteString("FPGA: Connected\r\n");
        sendFPGA_Command("RST", 0);
    } else {
        UART1_WriteString("FPGA: Not Connected\r\n");
    }
    
    // Wait for system to stabilize
    delay_ms(100);
    
    UART1_WriteString("Power-On Reset complete\r\n");
}


bool checkDAC_Connection(void) {
    I2C2_Start();
    I2C2_Write(DAC_ADDR << 1);    // Write address
    bool connected = !I2C2STATbits.ACKSTAT;  // Check if ACK received
    I2C2_Stop();
    return connected;
}

bool checkFPGA_Connection(void) {
    I2C1_Start();
    I2C1_Write(0x50 << 1);        // FPGA address
    bool connected = !I2C1STATbits.ACKSTAT;  // Check if ACK received
    I2C1_Stop();
    return connected;
}


int main(void) {
    // Initialize all systems
    performPOR();
    SYSTEM_Init();
    GPIO_Init();
    UART1_Init();
//    I2C1_Init();
//    I2C2_Init();
    
    // Initial message
    UART1_WriteString("\r\nPIC32 Control System Ready\r\n");
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