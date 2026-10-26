#include "board.h"

#include "drivers/uart.h"
#include "drivers/clock.h"

#include "utils/ring_buffer.h"

#include <stdint.h>
#include <xc.h>

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

static struct RingBuffer uartRxBuffer;

static volatile uint32_t overFlowCount = 0U;
static volatile uint32_t rxErrorCount = 0U;

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

void uart_init() {

  mclk_enable_bus_clock(&MCLK_REGS->MCLK_APBDMASK, MCLK_APBDMASK_SERCOM5_Msk);

  gclk_enable_peripheral_channel(SERCOM5_GCLK_CHANNEL,
                                 GCLK_PCHCTRL_GEN_GCLK1_Val);

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

bool uart_rx_ready(void) {
  return (UART_REGS.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) != 0u;
}

char uart_getc(void) {
  if (uart_rx_ready()) {
    char c = (uint8_t)UART_REGS.SERCOM_DATA;
    return c;
  }
}

void uart_enable_rx_interrupt(void) {
  // initialize the ring buffer
  ring_buffer_init(&uartRxBuffer);

  // reset overflow count
  overFlowCount = 0U;

  // reset error count
  rxErrorCount = 0;
  // allow interputs
  UART_REGS.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk;

  // clear pending interupts
  NVIC_ClearPendingIRQ(SERCOM5_2_IRQn);

  // enable interupts
  NVIC_EnableIRQ(SERCOM5_2_IRQn);
}

bool uart_read_byte(uint8_t *outByte) {
  // prevent new interputs from disrupting the process
  NVIC_DisableIRQ(SERCOM5_2_IRQn);

  // read the byte
  bool success = ring_buffer_pop(&uartRxBuffer, outByte);

  // reenable interputs
  NVIC_EnableIRQ(SERCOM5_2_IRQn);

  return success;
}

uint32_t uart_get_rx_overflow_count(void) { return overFlowCount; }

uint32_t uart_get_rx_error_count(void) { return rxErrorCount; }

void SERCOM5_2_Handler(void) {
  if (uart_rx_ready()) {

    // Read the hardware status
    uint32_t stats = UART_REGS.SERCOM_STATUS;

    // Read received data exactly once
    uint8_t data = (uint8_t)UART_REGS.SERCOM_DATA;

    // Define hardware error flags
    uint32_t errorMsk = SERCOM_USART_INT_STATUS_FERR_Msk |
                        SERCOM_USART_INT_STATUS_PERR_Msk |
                        SERCOM_USART_INT_STATUS_BUFOVF_Msk;

    // Extract active errors
    uint32_t errors = stats & errorMsk;

    if (errors != 0U) {

      // Count this erroneous reception
      rxErrorCount += 1;

      // Clear the hardware status error flags
      UART_REGS.SERCOM_STATUS = errors;

      // Acknowledge the USART error interrupt flag
      UART_REGS.SERCOM_INTFLAG = SERCOM_USART_INT_INTFLAG_ERROR_Msk;

    } else {

      // Store valid byte in software buffer
      if (!ring_buffer_push(&uartRxBuffer, data)) {
        overFlowCount += 1;
      }
    }
  }
}