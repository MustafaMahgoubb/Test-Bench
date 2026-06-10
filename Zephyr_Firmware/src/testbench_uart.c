#include "testbench_uart.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/ring_buffer.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define UART_CHANNEL1 1
#define UART_CHANNEL2 2

#define SUCCESS 0
#define INVALID_CHANNEL 1
#define DEVICE_NOT_READY 2

// Get Device using Alias configured in the overlay
static const struct device *const uart_ch1 = DEVICE_DT_GET(DT_ALIAS(uart_ch1));
static const struct device *const uart_ch2 = DEVICE_DT_GET(DT_ALIAS(uart_ch2));

// Declare Ring Buffers 2 for each Channel
#define RING_BUF_SIZE 256

RING_BUF_DECLARE(uart_ch1_rx_ringbuf, RING_BUF_SIZE);
RING_BUF_DECLARE(uart_ch1_tx_ringbuf, RING_BUF_SIZE);

RING_BUF_DECLARE(uart_ch2_rx_ringbuf, RING_BUF_SIZE);
RING_BUF_DECLARE(uart_ch2_tx_ringbuf, RING_BUF_SIZE);

// Define CallBack Function for each Channel ISR
static void uart_ch1_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    // 1. Always update the IRQ status first
    if (!uart_irq_update(dev))
    {
        return;
    }

    if (uart_irq_rx_ready(dev))
    {
        // static int 	uart_fifo_read (const struct device *dev, uint8_t *rx_data, const int size)
        while (uart_fifo_read(dev, &c, 1) == 1)
        {
            ring_buf_put(&uart_ch1_rx_ringbuf, &c, 1);
        }
    }

    if (uart_irq_tx_ready(dev))
    {
        uint8_t c;
        // The hardware FIFO on STM32 is only 1 byte deep!
        // We must pull exactly 1 byte at a time so we don't lose data.
        if (ring_buf_get(&uart_ch1_tx_ringbuf, &c, 1) > 0)
        {
            // Push 1 byte to the hardware
            uart_fifo_fill(dev, &c, 1);
        }
        else
        {
            // CRITICAL: Nothing left to send, stop the TX interrupts!
            uart_irq_tx_disable(dev);
            // The uart tx Enable will be called inside send cmd
        }
    }
}

static void uart_ch2_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    // 1. Always update the IRQ status first
    if (!uart_irq_update(dev))
    {
        return;
    }

    if (uart_irq_rx_ready(dev))
    {
        // static int 	uart_fifo_read (const struct device *dev, uint8_t *rx_data, const int size)
        while (uart_fifo_read(dev, &c, 1) == 1)
        {
            ring_buf_put(&uart_ch2_rx_ringbuf, &c, 1);
        }
    }

    if (uart_irq_tx_ready(dev))
    {
        uint8_t c;
        // The hardware FIFO on STM32 is only 1 byte deep!
        // We must pull exactly 1 byte at a time so we don't lose data.
        if (ring_buf_get(&uart_ch2_tx_ringbuf, &c, 1) > 0)
        {
            // Push 1 byte to the hardware
            uart_fifo_fill(dev, &c, 1);
        }
        else
        {
            // CRITICAL: Nothing left to send, stop the TX interrupts!
            uart_irq_tx_disable(dev);
            // The uart tx Enable will be called inside send cmd
        }
    }
}

int testBench_uart_Init()
{
    // Check If the Device is ready
    if (!device_is_ready(uart_ch1))
    {
        return -DEVICE_NOT_READY;
    }

    if (!device_is_ready(uart_ch2))
    {
        return -DEVICE_NOT_READY;
    }

    // Assign The CallBack Functions to the devices
    // uart_irq_callback_user_data_set (const struct device *dev, uart_irq_callback_user_data_t cb, void *user_data)
    uart_irq_callback_user_data_set(uart_ch1, uart_ch1_cb, NULL);
    uart_irq_callback_user_data_set(uart_ch2, uart_ch2_cb, NULL);

    // Enable RX Interrupts
    // uart_irq_rx_enable (const struct device *dev)
    uart_irq_rx_enable(uart_ch1);
    uart_irq_rx_enable(uart_ch2);

    return SUCCESS;
}

/**
 *  Commands Implementations
 */

// argv[1] is uart channel , argv[2] is the string to be sent.
int cmd_uart_send(const struct shell *sh, size_t argc, char **argv)
{
    // arguments validation
    uint8_t uartChannel = (uint8_t)atoi(argv[1]);
    char *str = argv[2];

    switch (uartChannel)
    {
    case UART_CHANNEL1:
        if (!device_is_ready(uart_ch1))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }
        ring_buf_put(&uart_ch1_tx_ringbuf, (uint8_t *)str, strlen(str));

        // Enable Tx Interrupt
        uart_irq_tx_enable(uart_ch1);
        break;

    case UART_CHANNEL2:
        if (!device_is_ready(uart_ch2))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }
        ring_buf_put(&uart_ch2_tx_ringbuf, (uint8_t *)str, strlen(str));

        // Enable Tx Interrupt
        uart_irq_tx_enable(uart_ch2);
        break;

    default:
        shell_print(sh, "Invalid UART Channel. Available are 1 and 2");
        return -INVALID_CHANNEL;
    }
    return SUCCESS;
}

int cmd_uart_read(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t uartChannel = (uint8_t)atoi(argv[1]);
    uint8_t buffer[64];
    int len;

    switch (uartChannel)
    {
    case UART_CHANNEL1:
        while ((len = ring_buf_get(&uart_ch1_rx_ringbuf, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[len] = '\0';
            shell_print(sh, "%s", buffer);
        }
        break;

    case UART_CHANNEL2:
        while ((len = ring_buf_get(&uart_ch2_rx_ringbuf, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[len] = '\0';
            shell_print(sh, "%s", buffer);
        }
        break;

    default:
        shell_print(sh, "Invalid UART Channel. Available are 1 and 2");
        return -INVALID_CHANNEL;
    }
    return SUCCESS;
}

int cmd_uart_clear(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t uartChannel = (uint8_t)atoi(argv[1]);

    switch (uartChannel)
    {
    case UART_CHANNEL1:
        ring_buf_reset(&uart_ch1_rx_ringbuf);
        shell_print(sh, "UART1 RX Buffer Cleared");
        break;

    case UART_CHANNEL2:
        ring_buf_reset(&uart_ch2_rx_ringbuf);
        shell_print(sh, "UART2 RX Buffer Cleared");
        break;

    default:
        shell_print(sh, "Invalid UART Channel.");
        return -INVALID_CHANNEL;
    }
    return SUCCESS;
}

int cmd_uart_set_baud(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t uartChannel = (uint8_t)atoi(argv[1]);
    uint32_t baudrate = (uint32_t)atoi(argv[2]);
    struct uart_config cfg;
    const struct device *dev;

    switch (uartChannel)
    {
    case UART_CHANNEL1:
        dev = uart_ch1;
        break;
    case UART_CHANNEL2:
        dev = uart_ch2;
        break;
    default:
        return -INVALID_CHANNEL;
    }

    if (uart_config_get(dev, &cfg) == 0)
    {
        cfg.baudrate = baudrate;
        if (uart_configure(dev, &cfg) == 0)
        {
            shell_print(sh, "Baudrate set to %u", baudrate);
            return SUCCESS;
        }
    }

    shell_print(sh, "Failed to set baudrate");
    return -1;
}

/* Shell Subcommands Registration */
SHELL_STATIC_SUBCMD_SET_CREATE(
    uart_subcmds,
    SHELL_CMD_ARG(send, NULL, "Send string over UART", cmd_uart_send, 3, 0),
    SHELL_CMD_ARG(read, NULL, "Read received string from UART buffer", cmd_uart_read, 2, 0),
    SHELL_CMD_ARG(clear, NULL, "Clear the RX buffer", cmd_uart_clear, 2, 0),
    SHELL_CMD_ARG(set_baud, NULL, "Change baudrate", cmd_uart_set_baud, 3, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(uart, &uart_subcmds, "UART Commands", NULL);