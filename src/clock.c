#include "board.h"
#include "clock.h"


//MASTER CLOCK REGISTER -> Enable the Bus with the selected Mask
//GCLK REGISTER -> Enable the selected peripherial

void master_clock_enable_bus(volatile uint32_t *reg, uint32_t mask)
{
    *reg |= mask;
}

void gclk_enable_peripherial(uint8_t channelNumber, uint8_t clockGeneratorSource)
{
    //First disable the clock so changes don't mess with active hardware
    gclk_disable_peripherial(channelNumber);

    //First Clear the the source bits
    GCLK_REGS->GCLK_PCHCTRL[channelNumber] &= ~GCLK_PCHCTRL_GEN_Msk;

    //Select the source of clock for the peripherial
    GCLK_REGS->GCLK_PCHCTRL[channelNumber] |= (clockGeneratorSource << GCLK_PCHCTRL_GEN_Pos);


    //enable the peripherial's clock
    GCLK_REGS->GCLK_PCHCTRL[channelNumber] |= GCLK_PCHCTRL_CHEN_Msk;
    

    //wait for it to actually occcur
    while((GCLK_REGS->GCLK_PCHCTRL[channelNumber] & GCLK_PCHCTRL_CHEN_Msk) == 0U){}

}

void gclk_disable_peripherial(uint8_t channelNumber)
{
    //Disable the clock to the peripherial
    GCLK_REGS->GCLK_PCHCTRL[channelNumber] &= ~GCLK_PCHCTRL_CHEN_Msk;


    //wait for it to actually happen
    while((GCLK_REGS->GCLK_PCHCTRL[channelNumber] & GCLK_PCHCTRL_CHEN_Msk) != 0U){}
}


