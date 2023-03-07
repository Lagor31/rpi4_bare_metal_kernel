#include "../include/Uart.h"

#include "../include/GPIO.h"
#include "../include/Mem.h"

unsigned char UART::uart_output_queue[];
unsigned int UART::uart_output_queue_write;
unsigned int UART::uart_output_queue_read;

void UART::init() {
  MMIO::write(AUX_ENABLES, 1);  // enable UART1
  // MMIO::write(AUX_MU_IER_REG, 0);
  MMIO::write(AUX_MU_CNTL_REG, 0);
  MMIO::write(AUX_MU_LCR_REG, 3);  // 8 bits
  MMIO::write(AUX_MU_MCR_REG, 0);
  MMIO::write(AUX_MU_IER_REG, 0xD);
  // MMIO::write(AUX_MU_IIR_REG, 0xC6);  // disable interrupts
  MMIO::write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
  gpio->useAsAlt5(14);
  gpio->useAsAlt5(15);
  MMIO::write(AUX_MU_CNTL_REG, 3);  // enable RX/TX
}


const char *UART::getName() { return "UART Driver"; }
void UART::unload() {}

UART::UART(){};

UART::UART(GPIO *g) : UART() {
  UART::uart_output_queue_read = 0;
  UART::uart_output_queue_write = 0;
  gpio = g;
}

unsigned int UART::isOutputQueueEmpty() {
  return uart_output_queue_read == uart_output_queue_write;
}

unsigned int UART::isReadByteReady() {
  return MMIO::read(AUX_MU_LSR_REG) & 0x01;
}
unsigned int UART::isWriteByteReady() {
  return MMIO::read(AUX_MU_LSR_REG) & 0x20;
}

unsigned char UART::readChar() { return readByte(); }


unsigned char UART::readByte() {
  while (!isReadByteReady())
    ;
  return (unsigned char)MMIO::read(AUX_MU_IO_REG);
}

void UART::writeByteBlockingActual(unsigned char ch) {
  while (!isWriteByteReady())
    ;
  MMIO::write(AUX_MU_IO_REG, (unsigned int)ch);
}

void UART::loadOutputFifo() {
  while (!isOutputQueueEmpty() && isWriteByteReady()) {
    writeByteBlockingActual(uart_output_queue[uart_output_queue_read]);
    uart_output_queue_read =
        (uart_output_queue_read + 1) & (UART_MAX_QUEUE - 1);  // Don't overrun
  }
}

void UART::writeByteBlocking(unsigned char ch) {
  unsigned int next =
      (uart_output_queue_write + 1) & (UART_MAX_QUEUE - 1);  // Don't overrun

  while (next == uart_output_queue_read) loadOutputFifo();

  uart_output_queue[uart_output_queue_write] = ch;
  uart_output_queue_write = next;
}

void UART::write(const char *buffer) {
  while (*buffer) {
    if (*buffer == '\n') {
      writeByteBlockingActual('\r');
    }
    writeByteBlockingActual(*buffer++);
  }
}

void UART::write(const char b) { writeByteBlockingActual(b); }

void UART::drainOutputQueue() {
  while (!isOutputQueueEmpty()) loadOutputFifo();
}

void UART::flush() {
  loadOutputFifo();

  if (isReadByteReady()) {
    unsigned char ch = readByte();
    if (ch == '\r')
      write('\n');
    else
      writeByteBlocking(ch);
  }
}
