#ifndef FPGA_H
#define FPGA_H

#include <stdint.h>

void initUART_PPS();
void sendToUART(char* message);
void uartTransmit(uint8_t data);
uint8_t uartReceive();
void unlockPPS();
void lockPPS();
void startDataAcquisition();
void stopDataAcquisition();

#endif