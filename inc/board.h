#ifndef INC_BOARD
#define INC_BOARD

#include <stdint.h>
<<<<<<< HEAD
#include <xc.h>

#define LED0_PIN            (14U)
#define UART_PIN_GROUP      (1U)
#define UART_TX_PIN         (16U)
#define UART_RX_PIN         (17U)
#define UART_PMUX_FUNC      (0x2U)

#define UART_CORE_HZ        (48000000UL)
#define UART_BAUD           (115200UL)
=======

#define LED0_PIN (14U)

volatile uint32_t *STCSR = (uint32_t *) 0xE000E010;
volatile uint32_t *STRVR = (uint32_t *) 0xE000E014;
volatile uint32_t *STCVR = (uint32_t *) 0xE000E018;

>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
#endif /* INC_BOARD */
