#include <xc.h>

#include "drivers/clock.h"
#include "drivers/led.h"
#include "drivers/tc.h"
#include "drivers/uart.h"
//#include "storage/flash_storage.h"

#include "test/test.h"

int main() {

  // initalize led
  led_init();

  // setup GCLK 1 for UART
  gclk_configure_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);

  // initialize UART
  uart_init();
  uart_puts("\r\n");
  uart_enable_rx_interrupt();

  uint8_t receivedBytes = 0x0;

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

  // test out tc interputs
  tc0_init_periodic_interrupt(500U);

  tc0_start();

  // completed tests
  {
    // test_nvmctrl_memory_info();

    // test_string_utils();

    // test_nvmctrl_read_flash();

    // int i = 276;

    // char buff[2 * sizeof(i) + 1];

    // string s;

    // str_init(&s, &buff[0], 64, "");

    // str_format(&s, "The Int: %d \r\n", i);

    // uart_puts(s.chars);

    // test_nvmctrl_erase_flash();

    // test_nvmctrl_write_page();

    // test_nvmctrl_get_next_page();

    // test_flash_write_record();

    // test_flash_write_record_multi_page();

    // test_flash_read_record_multi_page();

    // test_crcc_calculation();
  }

  // current test
  test_storage_flash();

  while (1) {
    if (uart_read_byte(&receivedBytes)) {
      uart_putc(receivedBytes);
      if (receivedBytes == '\r') {
        uart_putc('\n');
      }
    }
  }
}