#ifndef INC_IC
#define INC_IC
#include "board.h"
#include <stdint.h>

int tc0_init_periodic_interrupt(uint32_t intervl_ms);
void tc0_start(void);
void tc0_stop(void);
void TC0_Handler(void);

#endif /* INC_IC */
