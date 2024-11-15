// Configuration bits
#pragma config FNOSC = PRIPLL    // Primary Osc w/PLL
#pragma config POSCMOD = XT      // XT oscillator mode
#pragma config FPLLIDIV = DIV_2  // Divide FRC by 2
#pragma config FPLLMUL = MUL_20  // PLL Multiply by 20
#pragma config FPLLODIV = DIV_2  // Divide PLL output by 2
#pragma config FPBDIV = DIV_1    // Peripheral Clock divisor
#pragma config FWDTEN = OFF      // Watchdog Timer disabled

#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
//#include <peripheral/system.h>  // For system configuration
//#include <peripheral/oscillator.h>

// Constants
#define SYSTEM_FREQ 60000000L  // 60MHz system clock
#define I2C_FREQ 100000        // 100kHz I2C frequency
#define DAC_I2C_ADDR 0x0C      // AD5647RVRNZ I2C address
#define FPGA_I2C_ADDR 0x48     // FPGA random I2C address
#define MAX_CMD_LENGTH 32
#define USB_BUFFER_SIZE 64

#define SYSCLK          40000000L  // 40MHz system clock
#define PBCLK           40000000L  // 40MHz peripheral bus clock
#define GetSystemClock()       (SYSCLK)
#define GetPeripheralClock()   (PBCLK)

volatile uint8_t USB_rx_buffer[USB_BUFFER_SIZE];
volatile uint8_t USB_tx_buffer[USB_BUFFER_SIZE];
volatile uint16_t USB_rx_count = 0;
volatile uint16_t USB_rx_read_ptr = 0;
volatile bool USB_rx_buffer_full = false;



// Command definitions
typedef enum {
    CMD_START,
    CMD_STOP,
    CMD_RESET,
    CMD_SETV,
    CMD_GETV,
    CMD_CG,
    CMD_FG,
    CMD_DG,
    CMD_POL,
    CMD_TRS,
    CMD_RT,
    CMD_FT,
    CMD_PZ,
    CMD_DB,
    CMD_PR,
    CMD_INVALID
} Command_t;

// System state
typedef struct {
    int current_voltage;
    int course_gain;
    int fine_gain;
    int digital_gain;
    int polarity;
    int threshold;
    int rise_time;
    int flat_top;
    int poles_zeros;
    int digital_blur;
    int pile_up_rejection;
    int system_active;
} SystemState_t;

SystemState_t system_state = {0};

// Function headers
void init_system(void);
void init_i2c1(void);
void init_i2c2(void);
void init_usb(void);
Command_t parse_command(char* cmd_str);
void process_command(Command_t cmd, char* params);
void send_usb_response(const char* response);
void dac_write(uint16_t value);
uint16_t dac_read(void);
void fpga_write(uint8_t reg, uint16_t value);
uint16_t fpga_read(uint8_t reg);

// I2C communication functions 
void i2c1_start(void);
void i2c1_stop(void);
void i2c1_write(uint8_t data);
uint8_t i2c1_read(int ack);
void i2c2_start(void);
void i2c2_stop(void);
void i2c2_write(uint8_t data);
uint8_t i2c2_read(int ack);

// System initialization
void init_system(void) {
    __builtin_disable_interrupts();
    OSCCONbits.COSC = 0b001;    // SPLL mode
    OSCCONbits.NOSC = 0b001;    // New oscillator is SPLL
    OSCCONbits.FRCDIV = 0;      // FRC divider is 1
    while (OSCCONbits.OSWEN != 0);
    while(OSCCONbits.SLOCK != 1);
    init_i2c1();
    init_i2c2();
    init_usb();
    memset(&system_state, 0, sizeof(SystemState_t));
    __builtin_enable_interrupts();
}

uint32_t get_system_frequency(void) {
    return GetSystemClock();
}

// Function to get peripheral bus frequency
uint32_t get_peripheral_frequency(void) {
    return GetPeripheralClock();
}

// I2C1 initialization for FPGA communication
void init_i2c1(void) {
    I2C1CON = 0;
    I2C1BRG = (SYSTEM_FREQ/(2*I2C_FREQ)) - 2;
    I2C1CONbits.ON = 1;
}

// I2C2 initialization DAC Communication
void init_i2c2(void) {
    I2C2CON = 0;
    I2C2BRG = (SYSTEM_FREQ/(2*I2C_FREQ)) - 2;
    I2C2CONbits.ON = 1;
}

// USB initialization
void init_usb(void) {
    U1PWRCbits.USBPWR = 1;
    TRISBbits.TRISB11 = 1;  // D- as input
    TRISBbits.TRISB10 = 1;  // D+ as input
    U1CONbits.SOFEN = 0;    // Disable SOF packets
    U1CONbits.PPBRST = 0;   // Peripheral reset bit
    U1CONbits.RESUME = 0;   // No resume signaling
    U1CONbits.USBEN = 1;    // Enable USB module
    IEC1bits.USBIE = 1;     // Enable USB interrupts
    IPC7bits.USBIP = 4;    // Set USB interrupt priority
    IPC7bits.USBIS = 0;    // Set USB interrupt sub-priority
    U1EP0 = 0x0D;           // Enable EP0 for OUT, IN, and SETUP
    U1IR = 0xFF;
    
    // Enable USB device
    U1CONbits.USBEN = 1;
}

// User input commands
Command_t parse_command(char* cmd_str) {
    if (strcmp(cmd_str, "start") == 0) return CMD_START;
    if (strcmp(cmd_str, "stop") == 0) return CMD_STOP;
    if (strcmp(cmd_str, "reset") == 0) return CMD_RESET;
    if (strncmp(cmd_str, "setv,", 5) == 0) return CMD_SETV;
    if (strcmp(cmd_str, "getv") == 0) return CMD_GETV;
    if (strncmp(cmd_str, "cg,", 3) == 0) return CMD_CG;
    if (strncmp(cmd_str, "fg,", 3) == 0) return CMD_FG;
    if (strncmp(cmd_str, "dg,", 3) == 0) return CMD_DG;
    if (strncmp(cmd_str, "pol,", 4) == 0) return CMD_POL;
    if (strncmp(cmd_str, "trs,", 4) == 0) return CMD_TRS;
    if (strncmp(cmd_str, "rt,", 3) == 0) return CMD_RT;
    if (strncmp(cmd_str, "ft,", 3) == 0) return CMD_FT;
    if (strncmp(cmd_str, "pz,", 3) == 0) return CMD_PZ;
    if (strncmp(cmd_str, "DB,", 3) == 0) return CMD_DB;
    if (strncmp(cmd_str, "pr,", 3) == 0) return CMD_PR;
    return CMD_INVALID;
}

// usb ISR
void __ISR(_USB_1_VECTOR, IPL4SOFT) USB_ISR(void) {
    if (U1IR & 0x04) {  // Token processing complete
        if (U1STAT & 0x04) {  // RX packet ready

            uint8_t byte_count = U1STAT >> 16;
            uint8_t *packet_addr = (uint8_t *)((U1STAT & 0xFF) << 3);
            
            for (int i = 0; i < byte_count && !USB_rx_buffer_full; i++) {
                USB_rx_buffer[USB_rx_count++] = packet_addr[i];
                if (USB_rx_count >= USB_BUFFER_SIZE) {
                    USB_rx_buffer_full = true;
                    USB_rx_count = 0;
                }
            }
        }
    }
    
    // Clear processed interrupts
    U1IR = 0xFF;
    IFS1bits.USBIF = 0;
}

// USB data is availability check
inline bool is_usb_data_available(void) {
    return (USB_rx_count > 0) || USB_rx_buffer_full;
}

// Function to read a byte from USB buffer
uint8_t read_usb_byte(void) {
    uint8_t data = 0;
    
    if (is_usb_data_available()) {
        data = USB_rx_buffer[USB_rx_read_ptr++];
        
        if (USB_rx_read_ptr >= USB_BUFFER_SIZE) {
            USB_rx_read_ptr = 0;
        }
        
        if (USB_rx_buffer_full) {
            USB_rx_buffer_full = false;
        } else if (USB_rx_read_ptr == USB_rx_count) {
            USB_rx_count = 0;
            USB_rx_read_ptr = 0;
        }
    }
    
    return data;
}


// DAC communication functions for write data to DAC
void dac_write(uint16_t value) {
    i2c2_start();
    i2c2_write(DAC_I2C_ADDR << 1);
    i2c2_write((value >> 8) & 0xFF);  // High byte
    i2c2_write(value & 0xFF);         // Low byte
    i2c2_stop();
}

// DAC communication functions for read data from DAC
uint16_t dac_read(void) {
    uint16_t value;
    
    i2c2_start();
    i2c2_write((DAC_I2C_ADDR << 1) | 1);  // Read mode
    value = (i2c2_read(1) << 8);          // High byte
    value |= i2c2_read(0);                // Low byte
    i2c2_stop();
    
    return value;
}

// FPGA communication functions for write data to FPGA
void fpga_write(uint8_t reg, uint16_t value) {
    i2c1_start();
    i2c1_write(FPGA_I2C_ADDR << 1);
    i2c1_write(reg);
    i2c1_write((value >> 8) & 0xFF);  // High byte
    i2c1_write(value & 0xFF);         // Low byte
    i2c1_stop();
}

// FPGA communication functions for read data from FPGA
uint16_t fpga_read(uint8_t reg) {
    uint16_t value;
    
    i2c1_start();
    i2c1_write(FPGA_I2C_ADDR << 1);
    i2c1_write(reg);
    i2c1_start();  // Repeated start
    i2c1_write((FPGA_I2C_ADDR << 1) | 1);
    value = (i2c1_read(1) << 8);  // High byte
    value |= i2c1_read(0);        // Low byte
    i2c1_stop();
    
    return value;
}

// Command processing
void process_command(Command_t cmd, char* params) {
    char response[64];
    int value;
    
    switch(cmd) {
        case CMD_START:
            system_state.system_active = 1;
            strcpy(response, "System started");
            break;
            
        case CMD_STOP:
            system_state.system_active = 0;
            strcpy(response, "System stopped");
            break;
            
        case CMD_RESET:
            memset(&system_state, 0, sizeof(SystemState_t));
            init_system();
            strcpy(response, "System reset complete");
            break;
            
        case CMD_SETV:
            if (sscanf(params, "%d", &value) == 1) {
                system_state.current_voltage = value;
                dac_write(value);
                sprintf(response, "Voltage set to %dmV", value);
            } else {
                strcpy(response, "Invalid voltage value");
            }
            break;
            
        case CMD_GETV:
            value = dac_read();
            sprintf(response, "Current voltage: %dmV", value);
            break;
            
        // Additional command implementations...
        
        default:
            strcpy(response, "Invalid command");
            break;
    }
    
    send_usb_response(response);
}

// Main program loop
int main(void) {
    char cmd_buffer[MAX_CMD_LENGTH];
    int cmd_pos = 0;
    Command_t cmd;
    init_system();
    
    while(1) {
        if (is_usb_data_available()) {
            char c = read_usb_byte();
            
            if (c == '\n' || c == '\r') {
                if (cmd_pos > 0) {
                    cmd_buffer[cmd_pos] = '\0';
                    cmd = parse_command(cmd_buffer);
                    process_command(cmd, strchr(cmd_buffer, ','));
                    cmd_pos = 0;
                }
            } else if (cmd_pos < MAX_CMD_LENGTH - 1) {
                cmd_buffer[cmd_pos++] = c;
            }
        }
        
        // System maintenance tasks
        if (system_state.system_active) {
            if (I2C1STATbits.BCL || I2C2STATbits.BCL) {
                init_i2c1();
                init_i2c2();
                send_usb_response("I2C bus error detected and recovered");
            }
            if (!U1CONbits.USBEN) {
                init_usb();
                send_usb_response("USB connection reestablished");
            }
        }
    }
}

// Function to send response back through USB
void send_usb_response(const char* response) {
    size_t len = strlen(response);
    size_t i;
    while (U1STAT & 0x02);
    for (i = 0; i < len && i < USB_BUFFER_SIZE - 2; i++) {
        USB_tx_buffer[i] = response[i];
    }
    
    USB_tx_buffer[i++] = '\r';
    USB_tx_buffer[i++] = '\n';
    U1TOK = 0x4D;  
    while (U1STAT & 0x02);  
}

void i2c1_start(void) {
    I2C1CONbits.SEN = 1;
    while(I2C1CONbits.SEN);
}

void i2c1_stop(void) {
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN);
}

void i2c1_write(uint8_t data) {
    I2C1TRN = data;
    while(I2C1STATbits.TRSTAT);
}

uint8_t i2c1_read(int ack) {
    I2C1CONbits.RCEN = 1;
    while(!I2C1STATbits.RBF);
    uint8_t data = I2C1RCV;
    
    I2C1CONbits.ACKDT = !ack;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
    
    return data;
}

void i2c2_start(void) {
    I2C2CONbits.SEN = 1;
    while(I2C2CONbits.SEN);
}

void i2c2_stop(void) {
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);
}

void i2c2_write(uint8_t data) {
    I2C2TRN = data;
    while(I2C2STATbits.TRSTAT);
}

uint8_t i2c2_read(int ack) {
    I2C2CONbits.RCEN = 1;
    while(!I2C2STATbits.RBF);
    uint8_t data = I2C2RCV;
    
    I2C2CONbits.ACKDT = !ack;
    I2C2CONbits.ACKEN = 1;
    while(I2C2CONbits.ACKEN);
    
    return data;
}