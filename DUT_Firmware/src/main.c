#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/sys/printk.h>          /* Include printk for debug messages */

#include "dut_dio.h"                    /* Include DUT DIO module interface */
#include "dut_spi.h"                    /* Include DUT SPI module interface */
#include "dut_uart.h"                   /* Include DUT UART module interface */
#include "dut_pwm_out.h"                /* Include DUT PWM output interface */


/**
 * @brief Main entry point for DUT firmware.
 *
 * This firmware runs on Board 2 / DUT.
 *
 * It acts as:
 * - DIO mirror responder
 * - SPI slave responder
 * - UART echo responder
 *
 * @return 0 on success, negative error code on failure.
 */
int main(void)
{
    int ret;                                             /* Store return status */

    printk("DUT Firmware Started\n");                    /* Print firmware startup message */

    ret = dut_dio_Init();                                /* Initialize DUT DIO mirror */

    if (ret < 0) {                                       /* Check if DUT DIO initialization failed */
        printk("DUT DIO initialization failed: %d\n", ret);
        return ret;                                      /* Stop application with error */
    }

    printk("DUT DIO initialized\n");                     /* Print DIO success message */

    ret = dut_spi_Init();                                /* Initialize DUT SPI slave */

    if (ret < 0) {                                       /* Check if DUT SPI initialization failed */
        printk("DUT SPI initialization failed: %d\n", ret);
        return ret;                                      /* Stop application with error */
    }

    printk("DUT SPI initialized\n");                     /* Print SPI success message */

    ret = dut_uart_Init();                               /* Initialize DUT UART echo */

    if (ret < 0) {                                       /* Check if DUT UART initialization failed */
        printk("DUT UART initialization failed: %d\n", ret);
        return ret;                                      /* Stop application with error */
    }

    printk("DUT UART initialized\n");                    /* Print UART success message */

    ret = dut_pwm_out_Init();

    if (ret < 0) {
        printk("DUT PWM OUT initialization failed: %d\n", ret);
        return ret;
    }

    printk("DUT PWM OUT initialized\n");

    while (1) {                                          /* Run forever */
        dut_dio_Process();                               /* Mirror DIO input to output */
        k_sleep(K_MSEC(1));                              /* Small delay to reduce CPU usage */
    }

    return 0;                                            /* This line should never be reached */
}