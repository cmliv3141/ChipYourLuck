#include "test.h"
#include "board.h"
#include "clock.h"
#include "nvmctrl.h"
#include "ring_buffer.h"
#include "string_utils.h"
#include "uart.h"

static void test_report(bool passed, const char *message) {
  if (passed) {
    uart_puts("[PASS] ");
  } else {
    uart_puts("[FAIL] ");
  }

  uart_puts(message);
  uart_puts("\r\n");
}

void test_nvmctrl_write_page(void) {
  uart_puts("\r\n========= NVMCTRL FLASH WRITE TEST =========\r\n");

  bool unlocked = (NVMCTRL_REGS->NVMCTRL_RUNLOCK * (1u << 31) != 0);

  if (unlocked) {
    test_report(nvmctrl_erase_block(0x000FE000U), "NVMCTRL ERASE BLOCK");
  } else {
    uart_puts("[FAILURE] BLOCK IS LOCKED BY SMARTEEPROM\r\n");
  }

  uint8_t testData[7] = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56};

  test_report(nvmctrl_write_page(0x000FE000U, &testData[0], 7),
              "NVMCTRL WRITE PAGE");

  uint8_t readData[7];
  test_report(nvmctrl_read_flash(0x000FE000U, &readData[0], 7),
              "NVMCTRL READ DATA");

  for (uint32_t i = 0; i < 8; i++) {
    string formated;
    char s[64];

    if (!str_init(&formated, &s[0], 64, ""))
      return;
    str_format(&formated, "Expected Byte: %x Recieved Byte: %x", testData[i],
               readData[i]);

    test_report(testData[i] == readData[i], formated.chars);
  }
}

void test_nvmctrl_erase_flash(void) {
  uart_puts("\r\n========= NVMCTRL FLASH ERASE TEST =========\r\n");

  bool unlocked = (NVMCTRL_REGS->NVMCTRL_RUNLOCK * (1u << 31) != 0);

  if (unlocked) {
    test_report(nvmctrl_erase_block(0x000FE000U), "NVMCTRL ERASE BLOCK");
  } else {
    uart_puts("[FAILURE] BLOCK IS LOCKED BY SMARTEEPROM\r\n");
  }
}

void test_nvmctrl_read_flash(void) {

  uart_puts("\r\n========= NVMCTRL FLASH READ TEST =========\r\n");

  static const uint8_t flashTestPattern[] = {0xA5, 0x5A, 0x12, 0x34,
                                             0xDE, 0xAD, 0xBE, 0xEF};

  uint32_t flashTestAddress = (uint32_t)(uintptr_t)&flashTestPattern[0];

  uint8_t buffer[8];

  test_report(nvmctrl_read_flash(flashTestAddress, &buffer[0], 8),
              "READING FLASH ADDRESS");

  for (uint32_t i = 0; i < 8; i++) {
    string formated;
    char s[64];

    if (!str_init(&formated, &s[0], 64, ""))
      return;
    str_format(&formated, "Expected Byte: %x Recieved Byte: %x",
               flashTestPattern[i], buffer[i]);

    test_report(flashTestPattern[i] == buffer[i], formated.chars);
  }
}

// void test_nvmctrl_memory_info(void) {
//   struct nvmCtrlParams nvmCtrl;
//   uart_puts("========= NVM MEMORY INFORMATION =========\r\n");

//   nvmctrl_get_memory_info(&nvmCtrl);

//   char numpages[64];
//   char pagesize[64];
//   char capacity[64];
//   int_to_string(nvmCtrl.numPages, numpages);
//   int_to_string(nvmCtrl.pageSize, pagesize);
//   int_to_string(nvmCtrl.capacity, capacity);

//   uart_puts("Flash Pages: \t");
//   uart_puts(numpages);
//   uart_puts("\r\n");

//   uart_puts("Flash Page Size: \t");
//   uart_puts(pagesize);
//   uart_puts("\r\n");

//   uart_puts("Total Flash: \t");
//   uart_puts(capacity);
//   uart_puts("\r\n");

//   uart_puts("SmartEEPROM: \t");

//   uart_puts(nvmCtrl.smartEEPROM ? "True" : "False");
//   uart_puts("\r\n");
// }

void test_ring_buffer(void) {
  uart_puts("========= RING BUFFER TEST =========\r\n");
  // init the ring buffer
  struct RingBuffer buff;
  ring_buffer_init(&buff);

  // test is empty since buffer is empty
  test_report(ring_buffer_is_empty(&buff), "BUFFER EMPTY");

  test_report(ring_buffer_push(&buff, (uint8_t)'A'), "PUSHED BYTE");

  uint8_t byte;
  test_report(ring_buffer_pop(&buff, &byte), "POPPED BYTE");

  test_report((byte == (uint8_t)'A'), "CORRECT BYTE RECIEVED");

  test_report(!ring_buffer_pop(&buff, &byte), "BUFFER EMPTY");

  ring_buffer_push(&buff, 'A');
  ring_buffer_push(&buff, 'B');
  ring_buffer_push(&buff, 'C');

  test_report(!ring_buffer_push(&buff, 'D'), "BUFFER FULL");

  ring_buffer_pop(&buff, &byte);

  ring_buffer_push(&buff, 'E');

  test_report((buff.array[0] == (uint8_t)'E'), "WRAP AROUND");

  uart_puts("========= RING BUFFER TEST COMPLETE =========\r\n");
}

void test_dpll0_configuration(void) {
  uart_puts("========= DPLL0 CONFIGURATION TEST =========\r\n");

  // Read hardware registers
  uint32_t ctrlA = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLA;

  uint32_t ctrlB = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLB;

  uint32_t ratio = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLRATIO;

  uint32_t sync = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSYNCBUSY;

  uint32_t status = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSTATUS;

  // Extract configuration fields
  uint32_t refClock =
      (ctrlB & OSCCTRL_DPLLCTRLB_REFCLK_Msk) >> OSCCTRL_DPLLCTRLB_REFCLK_Pos;

  uint32_t ldr =
      (ratio & OSCCTRL_DPLLRATIO_LDR_Msk) >> OSCCTRL_DPLLRATIO_LDR_Pos;

  uint32_t ldrFrac =
      (ratio & OSCCTRL_DPLLRATIO_LDRFRAC_Msk) >> OSCCTRL_DPLLRATIO_LDRFRAC_Pos;

  // 1. Verify DPLL0 is enabled
  test_report((ctrlA & OSCCTRL_DPLLCTRLA_ENABLE_Msk) != 0U, "DPLL0 ENABLED");

  // 2. Verify ONDEMAND is disabled
  test_report((ctrlA & OSCCTRL_DPLLCTRLA_ONDEMAND_Msk) == 0U,
              "ONDEMAND DISABLED");

  // 3. Verify reference clock selection
  test_report(refClock == OSCCTRL_DPLLCTRLB_REFCLK_GCLK_Val,
              "REFERENCE CLOCK = GCLK");

  // 4. Verify integer multiplier
  test_report(ldr == 119U, "MULTIPLIER LDR = 119");

  // 5. Verify fractional multiplier
  test_report(ldrFrac == 0U, "FRACTIONAL MULTIPLIER = 0");

  // 6. Verify enable synchronization
  test_report((sync & OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk) == 0U,
              "ENABLE SYNCHRONIZED");

  // 7. Verify ratio synchronization
  test_report((sync & OSCCTRL_DPLLSYNCBUSY_DPLLRATIO_Msk) == 0U,
              "RATIO SYNCHRONIZED");

  // 8. Verify PLL is locked
  test_report((status & OSCCTRL_DPLLSTATUS_LOCK_Msk) != 0U, "DPLL0 LOCKED");

  // 9. Verify output clock is ready
  test_report((status & OSCCTRL_DPLLSTATUS_CLKRDY_Msk) != 0U,
              "DPLL0 CLOCK READY");

  // 10. Verify both status flags simultaneously
  uint32_t requiredFlags =
      OSCCTRL_DPLLSTATUS_LOCK_Msk | OSCCTRL_DPLLSTATUS_CLKRDY_Msk;

  test_report((status & requiredFlags) == requiredFlags,
              "DPLL0 FULLY OPERATIONAL");
}

void test_dpll_reference_clock(void) {
  uart_puts("========= GCLK SETUP DPll REFERENCE TEST =========\r\n");

  if (gclk_configure_generator(2, GCLK_GENCTRL_SRC_DFLL_Val, 48) != 0) {
    uart_puts("[FAILED] CONFIGURE GENERATOR FAILED\r\n");
  }

  gclk_enable_peripheral_channel(GCLK_OSCTRL_FDPLL0,
                                 GCLK_PCHCTRL_GEN_GCLK2_Val);

  uint32_t currentConfigsGen = GCLK_REGS->GCLK_GENCTRL[2];
  uint32_t currentConfigsPCH = GCLK_REGS->GCLK_PCHCTRL[GCLK_OSCTRL_FDPLL0];

  // Extract individual fields
  uint32_t source =
      (currentConfigsGen & GCLK_GENCTRL_SRC_Msk) >> GCLK_GENCTRL_SRC_Pos;

  uint32_t divider =
      (currentConfigsGen & GCLK_GENCTRL_DIV_Msk) >> GCLK_GENCTRL_DIV_Pos;

  uint32_t enabled =
      (currentConfigsGen & GCLK_GENCTRL_GENEN_Msk) >> GCLK_GENCTRL_GENEN_Pos;

  uint32_t generator =
      (currentConfigsPCH & GCLK_PCHCTRL_GEN_Msk) >> GCLK_PCHCTRL_GEN_Pos;

  uint32_t channelEnable =
      (currentConfigsPCH & GCLK_PCHCTRL_CHEN_Msk) >> GCLK_PCHCTRL_CHEN_Pos;

  // Verify clock source
  if (source == GCLK_GENCTRL_SRC_DFLL_Val) {
    uart_puts("[PASS] DFLL SET AS SOURCE\r\n");
  } else {
    uart_puts("[FAIL] DFLL SET AS SOURCE\r\n");
  }

  // Verify divider
  if (divider == 48U) {
    uart_puts("[PASS] DIVIDER = 48\r\n");
  } else {
    uart_puts("[FAIL] DIVIDER\r\n");
  }

  // Verify generator enabled
  if (enabled == 1U) {
    uart_puts("[PASS] GCLK2 ENABLED\r\n");
  } else {
    uart_puts("[FAIL] GCLK2 DISABLED\r\n");
  }

  // Verify synchronization completed
  if (((GCLK_REGS->GCLK_SYNCBUSY & (1 << (2 + 2))) != 0) == 0U) {
    uart_puts("[PASS] GCLK2 SYNCHRONIZED\r\n");
  } else {
    uart_puts("[FAIL] GCLK2 SYNCHRONIZATION\r\n");
  }

  if (generator == GCLK_PCHCTRL_GEN_GCLK2_Val) {
    uart_puts("[PASS] PCHCTRL GENERATOR SET\r\n");
  } else {
    uart_puts("[FAIL] PCHCTRL GENERATOR SET\r\n");
  }

  if (channelEnable == 1) {
    uart_puts("[PASS] PCHCTRL CHANNEL ENABLED\r\n");
  } else {
    uart_puts("[FAIL] PCHCTRL CHANNEL ENABLE\r\n");
  }
}

void test_nvmctrl_set_flash_wait_states(uint8_t numWaitStates) {
  uart_puts("========= GCLK SETUP FLASH MEMORY TEST =========\r\n");
  // Do configurations
  if (nvmctrl_set_flash_wait_states(numWaitStates) != 0) {
    uart_puts("[FAILED] ERROR IN FLASH SETUP\r\n");

    return;
  }

  uint16_t currentConfigs = NVMCTRL_REGS->NVMCTRL_CTRLA;

  if ((currentConfigs & NVMCTRL_CTRLA_AUTOWS_Msk) >> NVMCTRL_CTRLA_AUTOWS_Pos ==
      0) {
    uart_puts("[PASS] AUTO WAIT STATE TURN OFFM\r\n");
  } else {
    uart_puts("[FAILED] AUTO WAIT STATE TURN OFFM\r\n");
  }

  if ((currentConfigs & NVMCTRL_CTRLA_RWS_Msk) >> NVMCTRL_CTRLA_RWS_Pos ==
      (uint8_t)numWaitStates) {
    uart_puts("[PASS] NUMBER OF WAIT STATES MATCHES\r\n");
  } else {
    uart_puts("[FAIL] NUMBER OF WAIT STATES MATCHES\r\n");
  }
}

void test_gclk1_configuration(void) {

  gclk_configure_generator(1, GCLK_GENCTRL_SRC_DFLL_Val, 1);

  uart_puts("========= GCLK1 REGISTER TEST =========\r\n");

  // Read the current configuration
  uint32_t reg = GCLK_REGS->GCLK_GENCTRL[1];

  // Extract individual fields
  uint32_t source = (reg & GCLK_GENCTRL_SRC_Msk) >> GCLK_GENCTRL_SRC_Pos;

  uint32_t divider = (reg & GCLK_GENCTRL_DIV_Msk) >> GCLK_GENCTRL_DIV_Pos;

  uint32_t enabled = (reg & GCLK_GENCTRL_GENEN_Msk) >> GCLK_GENCTRL_GENEN_Pos;

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
  test_dpll0_configuration();
  test_dpll_reference_clock();
  test_gclk1_configuration();
  test_uart();
  test_nvmctrl_set_flash_wait_states(6U);
}
