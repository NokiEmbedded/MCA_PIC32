#ifndef UART_USB_H
#define UART_USB_H

void initUART2_PPS();
void sendToSlave(char* message);
void slaveTransmit(uint8_t data);
uint8_t slaveReceive();
//void unlockPPS();
//void lockPPS();

#endif


#endif
