#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Configuration bits for system clock and peripherals
#pragma config FNOSC = PRIPLL    
#pragma config POSCMOD = XT      
#pragma config FPLLIDIV = DIV_2  
#pragma config FPLLMUL = MUL_20  
#pragma config FPLLODIV = DIV_2  
#pragma config FPBDIV = DIV_1    
#pragma config FWDTEN = OFF      

// Command enumerations

//    CMD_START,
//    CMD_STOP,
//    CMD_RESET,
//    CMD_SETV,
//    CMD_GETV,
//    CMD_CG,
//    CMD_FG,
//    CMD_DG,
//    CMD_POL,
//    CMD_TRS,
//    CMD_RT,
//    CMD_FT,
//    CMD_PZ,
//    CMD_DB,
//    CMD_PR,
//    CMD_INVALID
//} 

// System state structure
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


// Function prototypes for peripheral initialization
void init_i2c1(void);      // Initialize I2C1 for FPGA communication
void init_i2c2(void);      // Initialize I2C2 for DAC communication
void init_usb(void);       // Initialize USB communication
void init_system(void);    // Initialize the entire system


// Function prototypes for command parsing and processing
Command_t parse_command(char* cmd_str);          // Parse the command string
void process_command(Command_t cmd, char* params); // Process the parsed command


// I2C communication function prototypes
void i2c1_start(void);   // Start I2C1 communication
void i2c1_stop(void);    // Stop I2C1 communication
void i2c1_write(uint8_t data);  // Write a byte to I2C1
uint8_t i2c1_read(int ack);     // Read a byte from I2C1

void i2c2_start(void);   // Start I2C2 communication
void i2c2_stop(void);    // Stop I2C2 communication
void i2c2_write(uint8_t data);  // Write a byte to I2C2
uint8_t i2c2_read(int ack);     // Read a byte from I2C2


// Main loop function prototype
int main(void);  // Main program loop
