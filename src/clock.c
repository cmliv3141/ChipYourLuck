#include "clock.h"
#include "board.h"
#include "systick.h"
#include <stdbool.h>
#include <stdint.h>


uint32_t SystemCoreClock = 48000000U;

void mclk_enable_bus_clock(volatile uint32_t *reg, uint32_t mask) {
  *reg |= mask;
}

void gclk_enable_peripheral_channel(uint8_t channelNumber,
                                    uint8_t clockGeneratorSource) {
  // First disable the clock so changes don't mess with active hardware
  gclk_disable_peripheral_channel(channelNumber);

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

void gclk_disable_peripheral_channel(uint8_t channelNumber) {
  // Disable the clock to the peripherial
  GCLK_REGS->GCLK_PCHCTRL[channelNumber] &= ~GCLK_PCHCTRL_CHEN_Msk;

  // wait for it to actually happen
  while ((GCLK_REGS->GCLK_PCHCTRL[channelNumber] & GCLK_PCHCTRL_CHEN_Msk) !=
         0U) {
  }
}

int gclk_configure_generator(uint8_t clockGeneratorNumber,
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
    gclkConfigureMsk |= (uint16_t)dividerValue << GCLK_GENCTRL_DIV_Pos;
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

int nvmctrl_set_flash_wait_states(uint8_t numWaitStates) {

  // validate the size of wait states
  if (numWaitStates > 15U) {
    return -1;
  }

  // store the current settings
  uint16_t NVMctrlConfig = NVMCTRL_REGS->NVMCTRL_CTRLA;

  // clear and set NVM read wait states
  NVMctrlConfig &= ~(NVMCTRL_CTRLA_RWS_Msk);
  NVMctrlConfig |= ((uint8_t)numWaitStates << NVMCTRL_CTRLA_RWS_Pos);

  // clear auto wait state
  NVMctrlConfig &= ~(NVMCTRL_CTRLA_AUTOWS_Msk);

  // write the new configuration
  NVMCTRL_REGS->NVMCTRL_CTRLA = NVMctrlConfig;

  return 0; // Success
}

int oscctrl_configure_dpll0(void) {

  struct SysTickTimer timer;

  sysTick_init(&timer);

  bool timeOutReached = true;

  if ((OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLA &
       OSCCTRL_DPLLCTRLA_ENABLE_Msk) != 0U) {
    return -1; // FAIL CLOCK ENABLED
  }

  if (gclk_configure_generator(2, GCLK_GENCTRL_SRC_DFLL_Val, 48) != 0) {
    return -2; // FAIL GCLK CONFIG GEN
  }

  gclk_enable_peripheral_channel(GCLK_OSCTRL_FDPLL0,
                                 GCLK_PCHCTRL_GEN_GCLK2_Val);

  // Reset and Set Reference Clock (GCLK)
  OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLB &= ~(OSCCTRL_DPLLCTRLB_REFCLK_Msk);
  OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLB |=
      (0U << OSCCTRL_DPLLCTRLB_REFCLK_Pos);

  // Set multiplier Configs
  uint32_t DPLLConfig = 0x0;

  DPLLConfig |= (119U << OSCCTRL_DPLLRATIO_LDR_Pos);

  DPLLConfig |= (0U << OSCCTRL_DPLLRATIO_LDRFRAC_Pos);

  // Write the configuration
  OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLRATIO = DPLLConfig;

  sysTick_set_start_value(&timer);

  // wait for synch of ratio
  while (sysTick_get_time_difference_us(&timer) < DPLL_LOCK_TIMEOUT_US) {
    if ((OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSYNCBUSY &
         OSCCTRL_DPLLSYNCBUSY_DPLLRATIO_Msk) == 0) {
      timeOutReached = false;
      break;
    }
  }

  if (timeOutReached) {
    return -3;
  }

  // Clear on demand mode

  OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLA &= ~(OSCCTRL_DPLLCTRLA_ONDEMAND_Msk);

  // Enable the Clock
  OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLCTRLA |=
      (1U << OSCCTRL_DPLLCTRLA_ENABLE_Pos);

  timeOutReached = true;

  sysTick_set_start_value(&timer);

  // wait for Synch of Enable
  while (sysTick_get_time_difference_us(&timer) < DPLL_LOCK_TIMEOUT_US) {
    if ((OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSYNCBUSY &
         OSCCTRL_DPLLSYNCBUSY_ENABLE_Msk) == 0) {
      timeOutReached = false;
      break;
    }
  }

  if (timeOutReached) {
    return -4;
  }

  // wait 50ms for clkReady & clkLocked
  timeOutReached = true;

  sysTick_set_start_value(&timer);

  uint32_t requiredFlags =
      OSCCTRL_DPLLSTATUS_LOCK_Msk | OSCCTRL_DPLLSTATUS_CLKRDY_Msk;

  while (sysTick_get_time_difference_us(&timer) < DPLL_LOCK_TIMEOUT_US) {

    uint32_t status = OSCCTRL_REGS->DPLL[0].OSCCTRL_DPLLSTATUS;

    if ((status & requiredFlags) == requiredFlags) {
      timeOutReached = false;
      break;
    }
  }

  if (timeOutReached) {
    return -5;
  }

  return 0; // Success
}
