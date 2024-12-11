// Include necessary headers
#include <xc.h>
#include <sys/attribs.h>
#include <proc/p32mx250f128d.h>

// Configuration bits
#pragma config FNOSC = PRIPLL
#pragma config POSCMOD = HS
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_2
#pragma config FPBDIV = DIV_1
#pragma config FSOSCEN = OFF
#pragma config FWDTEN = OFF
#pragma config JTAGEN = OFF

// USB Constants
#define USB_MAX_PACKET_SIZE    64
#define USB_EP0_SIZE          8
#define CDC_BULK_IN_EP       2
#define CDC_BULK_OUT_EP      2
#define CDC_INT_EP           1

// USB Request Types
#define USB_DEVICE_DESCRIPTOR_TYPE     1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 2
#define USB_STRING_DESCRIPTOR_TYPE     3
#define USB_INTERFACE_DESCRIPTOR_TYPE  4
#define USB_ENDPOINT_DESCRIPTOR_TYPE   5
#define USB_CDC_DESCRIPTOR_TYPE       0x24

// CDC Specific Requests
#define SET_LINE_CODING        0x20
#define GET_LINE_CODING        0x21
#define SET_CONTROL_LINE_STATE 0x22

// Global Variables
static unsigned char EP0_OUT_buffer[USB_EP0_SIZE];
static unsigned char EP0_IN_buffer[USB_EP0_SIZE];
static unsigned char EP1_OUT_buffer[USB_MAX_PACKET_SIZE];
static unsigned char EP1_IN_buffer[USB_MAX_PACKET_SIZE];
static unsigned char EP2_OUT_buffer[USB_MAX_PACKET_SIZE];
static unsigned char EP2_IN_buffer[USB_MAX_PACKET_SIZE];
static unsigned char cdc_rx_buffer[USB_MAX_PACKET_SIZE];
static volatile unsigned char cdc_rx_length = 0;
static volatile unsigned char cdc_tx_busy = 0;

// Buffer Descriptor Table structure
typedef struct {
    volatile unsigned int BDSTAT;
    volatile unsigned int BDADDR;
} BufferDescriptor;
#pragma pack(push, 1)

typedef struct {
    BufferDescriptor EP0Out;
    BufferDescriptor EP0In;
    BufferDescriptor EP1Out;
    BufferDescriptor EP1In;
    BufferDescriptor EP2Out;
    BufferDescriptor EP2In;
} BDT;
#pragma pack(pop)

static BDT bdt __attribute__((aligned(512)));

// Line coding structure
typedef struct {
    unsigned long dwDTERate;    // Baud rate
    unsigned char bCharFormat;  // Stop bits
    unsigned char bParityType;  // Parity
    unsigned char bDataBits;    // Data bits
} LINE_CODING;

// Global variables
static BDT bdt __attribute__((aligned(512)));


static LINE_CODING line_coding = {115200, 0, 0, 8};

void handle_setup_packet(void) {
    unsigned char *setup_packet = EP0_OUT_buffer;
    
    switch(setup_packet[1]) {  // bRequest
        case GET_LINE_CODING:
            // Copy line coding structure to EP0 IN buffer
            for(int i = 0; i < sizeof(LINE_CODING); i++) {
                EP0_IN_buffer[i] = ((unsigned char*)&line_coding)[i];
            }
            bdt.EP0In.BDSTAT = ((sizeof(LINE_CODING) << 16) | 0x8080);
            break;
            
        case SET_LINE_CODING:
            // Copy received data to line coding structure
            for(int i = 0; i < sizeof(LINE_CODING); i++) {
                ((unsigned char*)&line_coding)[i] = EP0_OUT_buffer[i];
            }
            bdt.EP0In.BDSTAT = 0x8080;  // Send zero-length packet
            break;
            
        case SET_CONTROL_LINE_STATE:
            bdt.EP0In.BDSTAT = 0x8080;  // Send zero-length packet
            break;
            
        default:
            bdt.EP0In.BDSTAT = 0x8080;  // Send zero-length packet
            break;
    }
}

// Function prototypes
void init_usb(void);
void process_usb(void);
void handle_setup_packet(void);
void usb_device_tasks(void);
void cdc_tx_service(void);
void cdc_rx_service(void);

// System Configuration
void SYSTEMConfig(unsigned int freq, unsigned int flags) {
    // Configure the system clock
    OSCCONbits.NOSC = 0x1;     // FRCPLL
    OSCCONbits.PBDIV = 0x0;    // PBCLK is SYSCLK/1
    
    // Wait for PLL to lock
    while(!(OSCCONbits.SLOCK)); // Changed LOCK to SLOCK
}

// Enable Multi-vector Interrupts
void INTEnableSystemMultiVectoredInt(void) {
    INTCONSET = _INTCON_MVEC_MASK;
    __builtin_enable_interrupts();
}
// USB Initialization
void init_usb(void) {
    // Configure USB Clock
    SYSTEMConfig(48000000, 0);
    
    // Reset USB module
    U1CON = 0;
    
    // Configure Buffer Descriptor Table
    U1BDTP1 = ((unsigned int)&bdt >> 8) & 0xFF;
    
    // Initialize Endpoints
    U1EP0 = 0x0C;  // Enable EP0 IN/OUT
    U1EP1 = 0x1C;  // Enable EP1 IN/OUT, Interrupt
    U1EP2 = 0x1C;  // Enable EP2 IN/OUT, Bulk
    
    // Setup BDT entries
    bdt.EP0Out.BDSTAT = 0x80;
    bdt.EP0Out.BDADDR = (unsigned int)EP0_OUT_buffer;
    bdt.EP0In.BDSTAT = 0x00;
    bdt.EP0In.BDADDR = (unsigned int)EP0_IN_buffer;
    
    bdt.EP1Out.BDSTAT = 0x80;
    bdt.EP1Out.BDADDR = (unsigned int)EP1_OUT_buffer;
    bdt.EP1In.BDSTAT = 0x00;
    bdt.EP1In.BDADDR = (unsigned int)EP1_IN_buffer;
    
    bdt.EP2Out.BDSTAT = 0x80;
    bdt.EP2Out.BDADDR = (unsigned int)EP2_OUT_buffer;
    bdt.EP2In.BDSTAT = 0x00;
    bdt.EP2In.BDADDR = (unsigned int)EP2_IN_buffer;
    
    // Enable USB interrupts
    IEC1SET = 0x02000000;  // Enable USB interrupt
    IPC7SET = 0x1C000000;  // Set USB interrupt priority to 7 (Changed from IPC11SET to IPC7SET)
    
    // Enable USB module
    U1CON = 0x8008;
    U1CNFG1 = 0x0400;
}

// Process USB tasks
void process_usb(void) {
    if(U1IR & 0x04) {  // TOKEN_DONE interrupt
        unsigned int ep = (U1STAT >> 4) & 0x0F;
        unsigned int dir = (U1STAT >> 3) & 0x01;
        
        if(ep == 0) {
            if((bdt.EP0Out.BDSTAT & 0x3C) == 0x34) {
                handle_setup_packet();
            }
            else if(!dir) {
                bdt.EP0Out.BDSTAT = 0x8080;
            }
        }
        else if(ep == CDC_BULK_OUT_EP) {
            cdc_rx_length = (bdt.EP2Out.BDSTAT >> 16) & 0x3FF;
            for(int i = 0; i < cdc_rx_length; i++) {
                cdc_rx_buffer[i] = EP2_OUT_buffer[i];
            }
            bdt.EP2Out.BDSTAT = ((USB_MAX_PACKET_SIZE << 16) | 0x8080);
            cdc_rx_service();
        }
        else if(ep == CDC_BULK_IN_EP && dir) {
            cdc_tx_busy = 0;
            cdc_tx_service();
        }
        
        U1IR = 0x04;
    }
}

// Main function
int main(void) {
    // Initialize USB
    init_usb();
    
    // Enable interrupts
    INTEnableSystemMultiVectoredInt();
    
    while(1) {
        process_usb();
    }
    
    return 0;
}

// USB Interrupt Handler
void __ISR(_USB_1_VECTOR, IPL7AUTO) USB1_ISR(void) {
    process_usb();
    IFS1CLR = 0x02000000;  // Clear USB interrupt flag
}