#include <xc.h>
#include "board.h"
<<<<<<< HEAD
#include "uart.h"





int main() {
    initUART();
    uart_puts("Hello World\r\n");

    while(1){
        if(uart_rx_ready()){
            char c = uart_getc();
            uart_putc(c);
=======



void ledInit() {
    port_registers_t *Ports = PORT_REGS;
    port_group_registers_t *PortA = &Ports->GROUP[0];

    PortA->PORT_OUTSET = (1U << LED0_PIN);
    PortA->PORT_DIRSET = (1u << LED0_PIN);

}

void toggleLED() {
    port_registers_t *Ports = PORT_REGS;
    port_group_registers_t *PortA = &Ports->GROUP[0];

    PortA->PORT_OUTTGL = (1u << LED0_PIN);
}

void sysTickInit() {
    *STRVR = 0xFFFFFF; // max count
    *STCVR = 0; // force a re-load of the counter value register
    *STCSR = 5; // enable FCLK count without interrupt
}

int main() {
    ledInit();
    
    sysTickInit();
    unsigned int startCount = *STCVR;
    
    while (1) {
        
        unsigned int nowCount = *STCVR;
        unsigned int cyclesPassed = startCount - nowCount;
        
        if(cyclesPassed >= 10000000){
            toggleLED();
            startCount = nowCount;
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
        }
    }
}