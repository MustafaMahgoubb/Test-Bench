#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/sys/printk.h>          /* Include printk for startup messages */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */

#include "testbench_dio.h"              /* Include DIO module interface */
#include "testbench_adc.h"              /* Include ADC module interface */
#include "testbench_spi.h"              /* Include SPI module interface */


/**
 * @brief Handle the tb_ping shell command.
 *
 * This command is used to verify UART communication between the host and the board.
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success.
 */
static int cmd_tb_ping(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                   /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                   /* Mark argv as unused to avoid compiler warning */

    shell_print(sh, "PONG");           /* Print PONG as the shell response */

    return 0;                           /* Return success */
}


/* Register tb_ping shell command */
SHELL_CMD_REGISTER(tb_ping, NULL, "Test Bench ping command", cmd_tb_ping);


/**
 * @brief Main application entry point.
 *
 * This function initializes all enabled Test Bench modules.
 *
 * @return 0 on success, negative error code on failure.
 */
int main(void)
{
    int ret = testBench_dio_Init();     /* Initialize DIO module */

    if (ret < 0) {                      /* Check if DIO initialization failed */
        printk("DIO initialization failed: %d\n", ret); /* Print DIO initialization error */
        return ret;                     /* Stop application with error code */
    }

    ret = testBench_adc_Init();         /* Initialize ADC module */

    if (ret < 0) {                      /* Check if ADC initialization failed */
        printk("ADC initialization failed: %d\n", ret); /* Print ADC initialization error */
        return ret;                     /* Stop application with error code */
    }

    ret = testBench_spi_Init();

    if (ret < 0) {
        printk("SPI initialization failed: %d\n", ret);
        return ret;
    }

    printk("Test Bench Firmware Started\n"); /* Print firmware startup message */

    return 0;                           /* Return success */
}
