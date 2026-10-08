#ifndef INC_CLOCK
#define INC_CLOCK

void master_clock_enable_bus(volatile uint32_t *reg, uint32_t mask);

void gclk_disable_peripherial(uint8_t channelNumber);

void gclk_enable_peripherial(uint8_t channelNumber,
                             uint8_t clockGeneratorSource);

int gclk_setup_clock_generator(uint8_t clockGeneratorNumber,
                               uint32_t clockGeneratorSource,
                               uint16_t dividerValue);

#endif /* INC_CLOCK */
