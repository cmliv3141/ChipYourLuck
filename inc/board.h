#ifndef INC_BOARD
#define INC_BOARD

#include <stdint.h>

#define LED0_PIN (14U)

volatile uint32_t *STCSR = (uint32_t *) 0xE000E010;
volatile uint32_t *STRVR = (uint32_t *) 0xE000E014;
volatile uint32_t *STCVR = (uint32_t *) 0xE000E018;

#endif /* INC_BOARD */
