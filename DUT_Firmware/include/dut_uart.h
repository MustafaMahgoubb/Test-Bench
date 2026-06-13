#ifndef DUT_UART_H
#define DUT_UART_H

/**
 * @brief Initialize DUT UART echo module.
 *
 * This module uses USART6 as UART echo responder.
 *
 * Hardware pins:
 * - PA11 = USART6_TX
 * - PA12 = USART6_RX
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_uart_Init(void);

#endif