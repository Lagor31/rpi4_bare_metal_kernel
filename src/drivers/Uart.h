#ifndef UART_H
#define UART_H

#include "../Console.h"
#include "../Mem.h"
#include "Gpio.h"

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK / (baud * 8)) - 1)

class UART : public Driver, public Console {
 public:
  UART(GPIO *);
  UART();
  void init();
  const char *getName();
  void unload();
  void write(char);
  void write(const char*);
  void flush();

  enum {
    AUX_BASE = MMIO::PERIPHERAL_BASE + 0x215000,
    AUX_IRQ = AUX_BASE,
    AUX_ENABLES = AUX_BASE + 4,
    AUX_MU_IO_REG = AUX_BASE + 64,
    AUX_MU_IER_REG = AUX_BASE + 68,
    AUX_MU_IIR_REG = AUX_BASE + 72,
    AUX_MU_LCR_REG = AUX_BASE + 76,
    AUX_MU_MCR_REG = AUX_BASE + 80,
    AUX_MU_LSR_REG = AUX_BASE + 84,
    AUX_MU_MSR_REG = AUX_BASE + 88,
    AUX_MU_SCRATCH = AUX_BASE + 92,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_STAT_REG = AUX_BASE + 100,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK = 500000000,
    UART_MAX_QUEUE = 16 * 1024
  };

  static unsigned char uart_output_queue[UART_MAX_QUEUE];
  static unsigned int uart_output_queue_write;
  static unsigned int uart_output_queue_read;

  void uart_init(GPIO *);
  void loadOutputFifo();
  unsigned char readByte();
  unsigned int isReadByteReady();
  void writeByteBlocking(unsigned char ch);
  void update();

 private:
  unsigned int isOutputQueueEmpty();
  unsigned int isWriteByteReady();
  void writeByteBlockingActual(unsigned char ch);
  void drainOutputQueue();
};

#endif