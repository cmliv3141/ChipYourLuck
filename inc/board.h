#ifndef INC_BOARD
#define INC_BOARD

#include <xc.h>

#define NULL ((void *)0)

#define FLASH_STORAGE_SETTINGS_VERSION 1U
#define FLASH_STORAGE_GAME_HISTORY_VERSION 1U
#define FLASH_STORAGE_IMAGE_VERSION 1U

#define RING_BUFFER_SIZE 16

#define TC0_PERIPHERAL_CHANNEL (9U)
#define TC0_PERIPHERAL_CHANNEL_SOURCE (1U)

#define GCLK_MAX_GENERATOR 11U

#define GCLK_OSCTRL_DFll48 0
#define GCLK_OSCTRL_FDPLL0 1

extern uint32_t SystemCoreClock;

#define DPLL_LOCK_TIMEOUT_US (50000U)

#define LED0_PIN (14U)
#define UART_PIN_GROUP (1U)
#define UART_TX_PIN (16U)
#define UART_RX_PIN (17U)
#define UART_PMUX_FUNC (0x2U)

#define UART_CORE_HZ (SystemCoreClock)
#define UART_BAUD (115200UL)

#endif /* INC_BOARD */
