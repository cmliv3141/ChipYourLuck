#include "clock.h"
#include "test.h"
#include "uart.h"
#include <xc.h>

int main() {

  initUART();

  test_all(); // Test function

  while (1) {
    if (uart_rx_ready()) {
      char c = uart_getc();
      if (c == '\r') {
        uart_putc('\r');
        uart_putc('\n');
      } else {
        uart_putc(c);
      }
    }
  }
}