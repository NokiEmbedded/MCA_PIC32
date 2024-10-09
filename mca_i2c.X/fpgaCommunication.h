#ifndef FPGACOMMUNICATION_H
#define FPGACOMMUNICATION_H

void initUART_PPS();
void sendToUART(char* message);
void uartTransmit(uint8_t data);
uint8_t uartReceive();
void unlockPPS();
void lockPPS();
void startDataAcquisition();
void stopDataAcquisition();

#endif