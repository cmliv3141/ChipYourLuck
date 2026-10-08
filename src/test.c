#include "board.h"
#include "clock.h"
#include "uart.h"

// test clock driver
void test_gclk1_configuration(void)
{

    gclk_setup_clock_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);
    
    uart_puts("========= GCLK1 REGISTER TEST =========\r\n");

    // Read the current configuration
    uint32_t reg = GCLK_REGS->GCLK_GENCTRL[1];

    // Extract individual fields
    uint32_t source = (reg & GCLK_GENCTRL_SRC_Msk)
                      >> GCLK_GENCTRL_SRC_Pos;

    uint32_t divider = (reg & GCLK_GENCTRL_DIV_Msk)
                       >> GCLK_GENCTRL_DIV_Pos;

    uint32_t enabled = (reg & GCLK_GENCTRL_GENEN_Msk)
                       >> GCLK_GENCTRL_GENEN_Pos;

    // Verify clock source
    if (source == GCLK_GENCTRL_SRC_DFLL_Val) {
        uart_puts("[PASS] GCLK1 SOURCE = DFLL48M\r\n");
    } else {
        uart_puts("[FAIL] GCLK1 SOURCE\r\n");
    }

    // Verify divider
    if (divider == 1U) {
        uart_puts("[PASS] GCLK1 DIVIDER = 1\r\n");
    } else {
        uart_puts("[FAIL] GCLK1 DIVIDER\r\n");
    }

    // Verify generator enabled
    if (enabled == 1U) {
        uart_puts("[PASS] GCLK1 ENABLED\r\n");
    } else {
        uart_puts("[FAIL] GCLK1 DISABLED\r\n");
    }

    // Verify synchronization completed
    if (((GCLK_REGS->GCLK_SYNCBUSY & (1 << (1 + 2))) != 0) == 0U) {
        uart_puts("[PASS] GCLK1 SYNCHRONIZED\r\n");
    } else {
        uart_puts("[FAIL] GCLK1 SYNCHRONIZATION\r\n");
    }
}

void test_uart(void) {
  uart_puts("=========UART TEST=========\r\n");
  uart_puts("[PASS] UART TEST\r\n");
}

// Used to run all tests
void test_all(void) {
  test_gclk1_configuration();
  test_uart();
}
