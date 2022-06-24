#ifndef IO_H
#define IO_H
void uart_init();
void uart_writeText(const char *buffer);
void uart_loadOutputFifo();
unsigned char uart_readByte();
unsigned int uart_isReadByteReady();
void uart_writeByteBlocking(unsigned char ch);
void uart_update();

#endif