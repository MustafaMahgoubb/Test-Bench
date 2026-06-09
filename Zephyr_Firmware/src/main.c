#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/sys/printk.h>          /* Include printk for boot messages */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */

#include "modules/dio/dio_service.h"    /* Include the DIO service interface */
#include "modules/adc/adc_service.h"    /* Include the ADC service interface */


/**
 * @brief Handle the tb_ping shell command.
 *
 * This command is used to verify communication between the host and the board.
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 always.
 */
static int cmd_tb_ping(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                                      /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                      /* Mark argv as unused to avoid compiler warning */

    shell_print(sh, "PONG");                              /* Print PONG as the shell response */

    return 0;                                              /* Return success */
}


SHELL_CMD_REGISTER(tb_ping, NULL, "Test Bench ping command", cmd_tb_ping); /* Register tb_ping shell command */


/**
 * @brief Main application entry point.
 *
 * This function initializes all test bench services.
 *
 * @return 0 on success, negative error code on failure.
 */
int main(void)
{
    int ret = dio_service_init();                          /* Initialize the DIO service */

    if (ret < 0) {                                         /* Check if DIO service initialization failed */
        printk("DIO service initialization failed: %d\n", ret); /* Print the DIO initialization error */
        return ret;                                        /* Stop the application with the error code */
    }                                                      /* End DIO service initialization check */

    ret = adc_service_init();                              /* Initialize the ADC service */

    if (ret < 0) {                                         /* Check if ADC service initialization failed */
        printk("ADC service initialization failed: %d\n", ret); /* Print the ADC initialization error */
        return ret;                                        /* Stop the application with the error code */
    }                                                      /* End ADC service initialization check */

    printk("Test Bench Firmware Started\n");               /* Print firmware startup message */

    return 0;                                              /* Return success from main */
}
