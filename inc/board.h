#ifndef INC_BOARD
#define INC_BOARD

#include <stdint.h>
#include <xc.h>

#define LED0_PIN            (14U)
#define UART_PIN_GROUP      (1U)
#define UART_TX_PIN         (16U)
#define UART_RX_PIN         (17U)
#define UART_PMUX_FUNC      (0x2U)

#define UART_CORE_HZ        (48000000UL)
#define UART_BAUD           (115200UL)
#endif /* INC_BOARD */
