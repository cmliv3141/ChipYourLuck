#include "clock.h"
#include "led.h"
#include "systick.h"
#include "test.h"
#include "uart.h"
#include "tc.h"

#include <xc.h>

int main() {

  // initilize led
  led_init();

  // setup GCLK 1 for UART
  gclk_configure_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);

  // initialize UART
  uart_init();

  // Prepare flash for higher CPU speed
  nvmctrl_set_flash_wait_states(6U);

  // configure DPll0

  if (oscctrl_configure_dpll0() != 0) {
    uart_puts("[Failure] COULD NOT CONFIGURE DPLL0 PROPERLY\r\n");
    return -1;
  }
  if (system_clock_switch_to_dpll0() != 0) {
    uart_puts("[Failure] COULD NOT SWITCH SYSTEM CLOCK PROPERLY\r\n");
    return -1;
  }
    
  //test out tc interputs
  tc0_init_periodic_interrupt(500U);

  tc0_start();

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