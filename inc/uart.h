#ifndef INC_UART
#define INC_UART
#include <stdbool.h>

void initUART(void);
void uart_putc(char c);
void uart_puts(const char *s);

bool uart_rx_ready(void);
char uart_getc(void);
#endif /* INC_UART */
