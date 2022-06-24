#include "io/io.h"
#include "io/lagor.h"

extern "C" void main() {
  uart_init();
  uart_writeText("Dioporco!\n");
  Lagor test(1);
  uart_writeText(test.get());
  while (1) uart_update();
}
