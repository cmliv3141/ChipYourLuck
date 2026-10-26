#ifndef INC_DRIVERS_CLOCK
#define INC_DRIVERS_CLOCK
#include <stdint.h>

void mclk_enable_bus_clock(volatile uint32_t *reg, uint32_t mask);

void gclk_enable_peripheral_channel(uint8_t channelNumber,
                                    uint8_t clockGeneratorSource);

void gclk_disable_peripheral_channel(uint8_t channelNumber);

int gclk_configure_generator(uint8_t clockGeneratorNumber,
                             uint32_t clockGeneratorSource,
                             uint16_t dividerValue);

int nvmctrl_set_flash_wait_states(uint8_t numWaitStates);

int oscctrl_configure_dpll0(void);

int system_clock_switch_to_dpll0(void);

#endif /* INC_DRIVERS_CLOCK */
