#include "clock.h"
#include "led.h"
#include "systick.h"
#include "test.h"
#include "uart.h"
#include <xc.h>

int main() {

  // initilize led
  led_init();

  // setup GCLK 1 for UART
  gclk_configure_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);

  // initialize UART
  uart_init();

  struct SysTickTimer timer = {0, 0};
  sysTick_init(&timer);

  // test led toggle
  int count = 0;
  int cycles = 0;
  while (count < 10) {
    if (sysTick_get_time_difference_us(&timer) > 50000) {
      cycles += 1;
      sysTick_set_start_value(&timer);

      if (cycles == 10) {
        led_toggle();
        count += 1;
        cycles = 0;
      }
    }
  }

  // Prepare flash for higher CPU speed
  nvmctrl_set_flash_wait_states(6U);

  // configure DPll0

  if (oscctrl_configure_dpll0() == 0) {
    uart_puts("[PASS] DPLL0 INITIALIZATION\r\n");
  } else {
    uart_puts("[FAIL] DPLL0 INITIALIZATION\r\n");
  }

  if (system_clock_switch_to_dpll0() == 0) {
    uart_puts("[PASS] System Clock is DPLL0 (120 MHz)\r\n");
  } else {
    uart_puts("[FAIL] System Clock is not DPLL0 (120 MHz)\r\n");
  }
  count = 0;
  sysTick_set_start_value(&timer);
  cycles = 0;

  while (count < 10) {
    if (sysTick_get_time_difference_us(&timer) > 50000) {
      cycles += 1;
      sysTick_set_start_value(&timer);

      if (cycles == 10) {
        led_toggle();
        count += 1;
        cycles = 0;
      }
    }
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