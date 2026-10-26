#include "board.h"

#include "drivers/led.h"
#include "drivers/clock.h"

void led_init()
{
  mclk_enable_bus_clock(&MCLK_REGS->MCLK_APBBMASK, MCLK_APBBMASK_PORT_Msk);
 
  PORT_REGS->GROUP[0].PORT_PINCFG[LED0_PIN] = 0u;

  PORT_REGS->GROUP[0].PORT_DIRSET = (1U << LED0_PIN);

  PORT_REGS->GROUP[0].PORT_OUT = (0U << LED0_PIN);

}

void led_toggle()
{
  PORT_REGS->GROUP[0].PORT_OUTTGL = (1U << LED0_PIN);
}

void led_off()
{
  PORT_REGS->GROUP[0].PORT_OUTCLR = (1U << LED0_PIN);
}

void led_on()
{
  PORT_REGS->GROUP[0].PORT_OUTSET = (1U << LED0_PIN);
}