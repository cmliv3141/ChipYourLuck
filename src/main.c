#include <xc.h>
#include "board.h"
#include "uart.h"


int main() {
    initUART();
    uart_puts("Hello World\r\n");

    while(1){
        if(uart_rx_ready()){
            char c = uart_getc();
            uart_putc(c);
        }
    }
}