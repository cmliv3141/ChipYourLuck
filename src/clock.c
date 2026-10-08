#include "clock.h"
#include "board.h"

void master_clock_enable_bus(volatile uint32_t *reg, uint32_t mask) {
  *reg |= mask;
}

void gclk_enable_peripherial(uint8_t channelNumber,
                             uint8_t clockGeneratorSource) {
  // First disable the clock so changes don't mess with active hardware
  gclk_disable_peripherial(channelNumber);

  // First Clear the the source bits
  GCLK_REGS->GCLK_PCHCTRL[channelNumber] &= ~GCLK_PCHCTRL_GEN_Msk;

  // Select the source of clock for the peripherial
  GCLK_REGS->GCLK_PCHCTRL[channelNumber] |=
      (clockGeneratorSource << GCLK_PCHCTRL_GEN_Pos);

  // enable the peripherial's clock
  GCLK_REGS->GCLK_PCHCTRL[channelNumber] |= GCLK_PCHCTRL_CHEN_Msk;

  // wait for it to actually occcur
  while ((GCLK_REGS->GCLK_PCHCTRL[channelNumber] & GCLK_PCHCTRL_CHEN_Msk) ==
         0U) {
  }
}

void gclk_disable_peripherial(uint8_t channelNumber) {
  // Disable the clock to the peripherial
  GCLK_REGS->GCLK_PCHCTRL[channelNumber] &= ~GCLK_PCHCTRL_CHEN_Msk;

  // wait for it to actually happen
  while ((GCLK_REGS->GCLK_PCHCTRL[channelNumber] & GCLK_PCHCTRL_CHEN_Msk) !=
         0U) {
  }
}

int gclk_setup_clock_generator(uint8_t clockGeneratorNumber,
                               uint32_t clockGeneratorSource,
                               uint16_t dividerValue) {

  // Data Validation Return if Not Supported
  if (clockGeneratorNumber == 0 || clockGeneratorNumber > GCLK_MAX_GENERATOR) {
    return 2;
  }
  // Have to build a mask with configurations best practice to only write once
  uint32_t gclkConfigureMsk = 0x0;

  // set the Clock's source
  gclkConfigureMsk |= GCLK_GENCTRL_SRC(clockGeneratorSource);

  // set the clock's division settings
  if (clockGeneratorNumber == 1) {
    gclkConfigureMsk |= dividerValue << GCLK_GENCTRL_DIV_Pos;
  } else {
    // Only Gen Clock 1 Supports 16 div bits
    if (dividerValue > UINT8_MAX)
      return 1; // Return Error Value to big for current register

    gclkConfigureMsk |= (uint8_t)dividerValue << GCLK_GENCTRL_DIV_Pos;
  }

  // Enable the clock
  gclkConfigureMsk |= GCLK_GENCTRL_GENEN_Msk;

  // Check to see if clock is synchronizing, wait if it is
  while ((GCLK_REGS->GCLK_SYNCBUSY & (1 << (clockGeneratorNumber + 2))) != 0) {
  }

  // write the configurations to the register
  GCLK_REGS->GCLK_GENCTRL[clockGeneratorNumber] = gclkConfigureMsk;

  // Wait for it to synch
  while ((GCLK_REGS->GCLK_SYNCBUSY & (1 << (clockGeneratorNumber + 2))) != 0) {
  }

  return 0; // Success
}
