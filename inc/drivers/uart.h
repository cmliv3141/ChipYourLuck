#ifndef INC_UART
#define INC_UART
#include <stdbool.h>
#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);

bool uart_rx_ready(void);
char uart_getc(void);

void uart_enable_rx_interrupt(void);
bool uart_read_byte(uint8_t *outByte);

uint32_t uart_get_rx_overflow_count(void);

uint32_t uart_get_rx_error_count(void);
#endif /* INC_UART */
