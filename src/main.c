#include "clock.h"
#include "test.h"
#include "uart.h"
#include <xc.h>

int main() {

  // setup GCLK 1 for UART
  gclk_configure_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);

  // initialize UART
  uart_init();

  // Prepare flash for higher CPU speed
  nvmctrl_set_flash_wait_states(6U);

  // configure DPll0

  if (oscctrl_configure_dpll0() == 0) {
    uart_puts("[PASS] DPLL0 INITIALIZATION\r\n");
    test_dpll0_configuration();
  } else {
    uart_puts("[FAIL] DPLL0 INITIALIZATION\r\n");
  }

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