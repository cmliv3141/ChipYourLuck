#include <stdint.h>
#include <xc.h>

#include "board.h"
#include "uart.h"
#include "clock.h"

#define UART_REGS (SERCOM5_REGS->USART_INT)

#define SERCOM5_APBD_BIT (1U << 1)

#define SERCOM5_GCLK_CHANNEL (35U)
#define GCLK_PCHCTRL_CHEN_BIT (1U << 6)

#define PINCFG_PMUXEN (1U << 0)
#define PINCFG_INEN (1U << 1)

#define CTRLA_ENABLE (1U << 1)
#define CTRLA_MODE_INT_CLK (0x1U << 2)
#define CTRLA_SAMPR_16X_ARITH (0x0U << 13)
#define CTRLA_TXPO_PAD0 (0x0U << 16)
#define CTRLA_RXPO_PAD1 (0x1U << 20)
#define CTRLA_DORD_LSB_FIRST (1U << 30)
#define CTRLB_CHSIZE_8BIT (0x0U << 0)
#define CTRLB_TXEN (1U << 16)
#define CTRLB_RXEN (1U << 17)

#define INTFLAG_DRE (1U << 0)

static uint16_t uart_baud_value(uint32_t fref, uint32_t baud) {
  return (uint16_t)(65536ULL - ((65536UL * 16ULL * baud) / fref));
}

static void pin_to_peripheral(uint32_t group, uint32_t pin, uint32_t func,
                              uint32_t extraCfg) {
  port_group_registers_t *g = &PORT_REGS->GROUP[group];
  uint32_t idx = pin >> 1;
  uint8_t mux = g->PORT_PMUX[idx];

  if ((pin & 1U) == 0U) {
    mux = (uint8_t)((mux & 0xF0U) | func);
  } else {
    mux = (uint8_t)((mux & 0xFU) | (func << 4));
  }

  g->PORT_PMUX[idx] = mux;
  g->PORT_PINCFG[pin] = (uint8_t)(PINCFG_PMUXEN | extraCfg);
}

void initUART(void) {

  master_clock_enable_bus(&MCLK_REGS->MCLK_APBDMASK, MCLK_APBDMASK_SERCOM5_Msk);

  gclk_enable_peripherial(SERCOM5_GCLK_CHANNEL, 0);
  
  
  pin_to_peripheral(UART_PIN_GROUP, UART_TX_PIN, UART_PMUX_FUNC, 0U);
  pin_to_peripheral(UART_PIN_GROUP, UART_RX_PIN, UART_PMUX_FUNC, PINCFG_INEN);

  UART_REGS.SERCOM_CTRLA = CTRLA_MODE_INT_CLK | CTRLA_SAMPR_16X_ARITH |
                           CTRLA_TXPO_PAD0 | CTRLA_RXPO_PAD1 |
                           CTRLA_DORD_LSB_FIRST;
  UART_REGS.SERCOM_CTRLB = CTRLB_CHSIZE_8BIT | CTRLB_TXEN | CTRLB_RXEN;
  UART_REGS.SERCOM_BAUD = uart_baud_value(UART_CORE_HZ, UART_BAUD);

  while (UART_REGS.SERCOM_SYNCBUSY != 0U) {
  } /* wait: settings still crossing clocks */

  UART_REGS.SERCOM_CTRLA |= CTRLA_ENABLE;
  while (UART_REGS.SERCOM_SYNCBUSY != 0U) {
  }
}

void uart_putc(char c) {
  while ((UART_REGS.SERCOM_INTFLAG & INTFLAG_DRE) == 0U) {
  }

  UART_REGS.SERCOM_DATA = (uint8_t)c;
}

void uart_puts(const char *s) {
  while (*s != '\0') {
    uart_putc(*s++);
  }
}

bool uart_rx_ready(void) { return (UART_REGS.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) != 0u; }

char uart_getc(void) {
  if (uart_rx_ready()) {
    char c = (uint8_t)UART_REGS.SERCOM_DATA;
    return c;
  }
}