#include "tc.h"
#include "clock.h"
#include "led.h"
#include "systick.h"

#include <stdbool.h>

int tc0_init_periodic_interrupt(uint32_t interval_ms) {

  // enable tc bus clock
  mclk_enable_bus_clock(&MCLK_REGS->MCLK_APBAMASK, MCLK_APBAMASK_TC0_Msk);

  // check if GCLK1 is enabled
  if (((GCLK_REGS->GCLK_GENCTRL[1] & GCLK_GENCTRL_GENEN_Msk) >>
       GCLK_GENCTRL_GENEN_Pos) != 1)
    return -2;

  // route GCLK1 for the periphrial
  gclk_enable_peripheral_channel(TC0_PERIPHERAL_CHANNEL,
                                 TC0_PERIPHERAL_CHANNEL_SOURCE);

  // make sure that tc0 is not currently enabled
  if ((TC0_REGS->COUNT16.TC_CTRLA & TC_CTRLA_ENABLE_Msk) >>
          TC_CTRLA_ENABLE_Pos ==
      1)
    return -1;

  // configure the CTRLA reg
  uint32_t ctrlaConfig = 0x0;

  // enable 16 bit count
  ctrlaConfig |= TC_CTRLA_MODE_COUNT16;

  // write the prescaler
  ctrlaConfig |= TC_CTRLA_PRESCALER_DIV1024;

  // set synch mode to reset counter on next clock
  ctrlaConfig |= TC_CTRLA_PRESCSYNC_RESYNC;

  // write the CTRLA configuration
  TC0_REGS->COUNT16.TC_CTRLA = ctrlaConfig;

  // wait for synch
  SysTickTimer timer;
  sysTick_init(&timer);

  bool timedOut = true;

  // write the wave register(Normal Frequency)
  TC0_REGS->COUNT16.TC_WAVE = TC_WAVE_WAVEGEN_MFRQ;

  // calculate the compare value
  uint64_t N = (((uint64_t)48000000U * interval_ms) / (1024U * 1000U));

  //prevent underflow
  N = (N > 1) ? N -1 : UINT32_MAX; 

  // check if the value is bigger than 16 bits
  if (N > UINT16_MAX)
    return -4;

  // set the compare value
  TC0_REGS->COUNT16.TC_CC[0] = (uint16_t)N;

  // wait for synch
  sysTick_set_start_value(&timer);

  while (sysTick_get_time_difference_us(&timer) < 50000) {
    // make sure read synched
    if ((TC0_REGS->COUNT16.TC_SYNCBUSY & TC_SYNCBUSY_CC0_Msk) == 0) {
      timedOut = false;
      break;
    }
  }

  if (timedOut) {
    return -4;
  }

  // clear enabled interupts
  TC0_REGS->COUNT16.TC_INTENCLR = TC_INTENCLR_Msk;

  // Enable the interput flag
  TC0_REGS->COUNT16.TC_INTENSET = TC_INTENSET_MC0_Msk;

  // Enable TC0
  TC0_REGS->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;

  // wait for synch
  timedOut = true;

  sysTick_set_start_value(&timer);

  while (sysTick_get_time_difference_us(&timer) < 50000) {
    if ((TC0_REGS->COUNT16.TC_SYNCBUSY & TC_SYNCBUSY_ENABLE_Msk) == 0) {
      timedOut = false;
      break;
    }
  }

  if (timedOut) {
    return -3;
  }

  //stop the clock
  tc0_stop();

  //clear any pending interputs
  TC0_REGS->COUNT16.TC_INTFLAG = TC_INTFLAG_MC0_Msk;

  NVIC_ClearPendingIRQ(TC0_IRQn);
  // enable the cpu to use handler
  NVIC_EnableIRQ(TC0_IRQn);

  return 0;
}

void tc0_start(void) {
  // check if command is being processed
  while ((TC0_REGS->COUNT16.TC_CTRLBSET & TC_CTRLBSET_CMD_Msk) != 0)
    ;

  // force the count to Start
  TC0_REGS->COUNT16.TC_CTRLBSET = TC_CTRLBSET_CMD_RETRIGGER;

  // wait synch
  while ((TC0_REGS->COUNT16.TC_SYNCBUSY & TC_SYNCBUSY_CTRLB_Msk) != 0)
    ;
  // wait synch
  while ((TC0_REGS->COUNT16.TC_CTRLBSET & TC_CTRLBSET_CMD_Msk) != 0)
    ;
}
void tc0_stop(void) {
  // check if command is being processed
  while ((TC0_REGS->COUNT16.TC_CTRLBSET & TC_CTRLBSET_CMD_Msk) != 0)
    ;

  // force the count to stop
  TC0_REGS->COUNT16.TC_CTRLBSET = TC_CTRLBSET_CMD_STOP;
  // wait synch
  while ((TC0_REGS->COUNT16.TC_SYNCBUSY & TC_SYNCBUSY_CTRLB_Msk) != 0)
    ;
  // wait synch
  while ((TC0_REGS->COUNT16.TC_CTRLBSET & TC_CTRLBSET_CMD_Msk) != 0)
    ;
}
void TC0_Handler(void) {

  if ((TC0_REGS->COUNT16.TC_INTFLAG & TC_INTFLAG_MC0_Msk) != 0) {
    // togle LED
    led_toggle();

    // Clear the flag
    TC0_REGS->COUNT16.TC_INTFLAG = TC_INTFLAG_MC0_Msk;
  }
}
