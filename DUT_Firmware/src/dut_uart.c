#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/uart.h>        /* Include Zephyr UART driver APIs */
#include <zephyr/sys/printk.h>          /* Include printk for debug messages */
#include <errno.h>                      /* Include standard error codes */
#include <stdbool.h>                    /* Include bool type */

#include "dut_uart.h"                   /* Include DUT UART module interface */


/* Get UART test node from devicetree alias */
#define DUT_UART_NODE DT_ALIAS(uart_test)

/* Define UART echo thread stack size */
#define DUT_UART_THREAD_STACK_SIZE 1024

/* Define UART echo thread priority */
#define DUT_UART_THREAD_PRIORITY 7


/* Get UART device from devicetree */
static const struct device *uart_dev = DEVICE_DT_GET(DUT_UART_NODE);

/* Store UART initialization state */
static volatile bool uart_initialized = false;


/**
 * @brief UART echo thread.
 *
 * This thread reads all available bytes from USART6
 * and immediately sends them back.
 */
static void dut_uart_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);                                     /* Mark arg1 as unused */
    ARG_UNUSED(arg2);                                     /* Mark arg2 as unused */
    ARG_UNUSED(arg3);                                     /* Mark arg3 as unused */

    while (!uart_initialized) {                           /* Wait until UART initialization is done */
        k_yield();                                        /* Yield CPU without long sleep */
    }

    printk("DUT UART echo thread started\n");             /* Print UART thread start message */

    while (1) {                                           /* Run forever */
        unsigned char rx_char;                            /* Store received character */

        while (uart_poll_in(uart_dev, &rx_char) == 0) {   /* Drain all received bytes */
            uart_poll_out(uart_dev, rx_char);             /* Echo each byte back immediately */
        }

        k_yield();                                        /* Yield CPU without losing UART bursts */
    }
}


/* Define and start UART echo thread */
K_THREAD_DEFINE(
    dut_uart_thread_id,                                   /* Thread ID */
    DUT_UART_THREAD_STACK_SIZE,                           /* Stack size */
    dut_uart_thread,                                      /* Thread entry function */
    NULL,                                                 /* First argument */
    NULL,                                                 /* Second argument */
    NULL,                                                 /* Third argument */
    DUT_UART_THREAD_PRIORITY,                             /* Thread priority */
    0,                                                    /* Thread options */
    0                                                     /* Start immediately */
);


/**
 * @brief Initialize DUT UART echo module.
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_uart_Init(void)
{
    if (!device_is_ready(uart_dev)) {                     /* Check if UART device is ready */
        return -ENODEV;                                   /* Return error if UART is not ready */
    }

    uart_initialized = true;                              /* Allow UART echo thread to run */

    return 0;                                             /* Return success */
}

