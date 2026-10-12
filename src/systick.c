#include "systick.h"
#include "board.h"
#include <stdint.h>


#define SYSTICK_MAX 0x00FFFFFF



void sysTick_set_current_value(struct SysTickTimer *timer) {
  timer->currentTime = SysTick->VAL;
}

void sysTick_set_start_value(struct SysTickTimer *timer) {
  timer->startTime = SysTick->VAL;
}

uint32_t sysTick_get_time_difference_us(struct SysTickTimer *timer) {

  timer->currentTime = SysTick->VAL;
  uint32_t elapsed = timer->startTime - timer->currentTime;

  elapsed &= SYSTICK_MAX;

  uint32_t hz = SystemCoreClock;

  uint32_t microseconds = (uint32_t)(((uint64_t)elapsed * 1000000ULL) / hz);
  return microseconds;
}

void sysTick_init(struct SysTickTimer *timer) {
  // Check if SysTick is already enabled
  if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0) {
    

    //set CPU clock source
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    //set Load to max value of 24bit

    SysTick->LOAD = SYSTICK_MAX;

    // If not enable reset value and initalize timer
    SysTick->VAL = 0;

    timer->startTime = SysTick->VAL;
    timer->currentTime = SysTick->VAL;

    // enable systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

  } else {
    // If enable just take current values
    timer->startTime = SysTick->VAL;
    timer->currentTime = SysTick->VAL;
  }
}