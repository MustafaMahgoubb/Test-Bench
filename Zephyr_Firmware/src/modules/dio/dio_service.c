#include <zephyr/kernel.h>               /* Include Zephyr kernel APIs such as k_sleep */
#include <zephyr/device.h>               /* Include Zephyr device model APIs */
#include <zephyr/drivers/gpio.h>         /* Include Zephyr GPIO driver APIs */
#include <zephyr/shell/shell.h>          /* Include Zephyr shell APIs */
#include <errno.h>                       /* Include standard error codes such as EINVAL and EIO */
#include <stdlib.h>                      /* Include strtol for string to number conversion */

#include "dio_service.h"                 /* Include the DIO service public header */


#define DIO_OUT_NODE DT_ALIAS(dio_out)   /* Get the devicetree alias for the DIO output pin */
#define DIO_IN_NODE  DT_ALIAS(dio_in)    /* Get the devicetree alias for the DIO input pin */


static const struct gpio_dt_spec dio_out = GPIO_DT_SPEC_GET(DIO_OUT_NODE, gpios); /* Create GPIO spec for PB0 output */
static const struct gpio_dt_spec dio_in  = GPIO_DT_SPEC_GET(DIO_IN_NODE, gpios);  /* Create GPIO spec for PB1 input */


/**
 * @brief Initialize the DIO service pins.
 *
 * This function checks if the GPIO devices are ready, then configures:
 * - PB0 as output and starts it LOW
 * - PB1 as input
 *
 * @return 0 on success, negative error code on failure.
 */
int dio_service_init(void)
{
    if (!gpio_is_ready_dt(&dio_out)) {                         /* Check if the output GPIO port is ready */
        return -ENODEV;                                        /* Return error if output GPIO port is not ready */
    }                                                          /* End output GPIO readiness check */

    if (!gpio_is_ready_dt(&dio_in)) {                          /* Check if the input GPIO port is ready */
        return -ENODEV;                                        /* Return error if input GPIO port is not ready */
    }                                                          /* End input GPIO readiness check */

    int ret = gpio_pin_configure_dt(&dio_out, GPIO_OUTPUT_INACTIVE); /* Configure PB0 as output and set it LOW */

    if (ret < 0) {                                             /* Check if output pin configuration failed */
        return ret;                                            /* Return the GPIO configuration error */
    }                                                          /* End output configuration check */

    ret = gpio_pin_configure_dt(&dio_in, GPIO_INPUT);          /* Configure PB1 as input */

    if (ret < 0) {                                             /* Check if input pin configuration failed */
        return ret;                                            /* Return the GPIO configuration error */
    }                                                          /* End input configuration check */

    return 0;                                                  /* Return success if all DIO initialization steps passed */
}


/**
 * @brief Handle the tb_dio_write shell command.
 *
 * Command format:
 * tb_dio_write <0|1>
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_dio_write(const struct shell *sh, size_t argc, char **argv)
{
    if (argc != 2) {                                           /* Check that the command has exactly one argument after the command name */
        shell_error(sh, "Usage: tb_dio_write <0|1>");          /* Print correct command usage */
        return -EINVAL;                                        /* Return invalid argument error */
    }                                                          /* End argument count check */

    long value = strtol(argv[1], NULL, 10);                    /* Convert the input argument from string to number */

    if ((value != 0) && (value != 1)) {                        /* Check that the value is only 0 or 1 */
        shell_error(sh, "Error: value must be 0 or 1");        /* Print error message for invalid value */
        return -EINVAL;                                        /* Return invalid argument error */
    }                                                          /* End value validation check */

    int ret = gpio_pin_set_dt(&dio_out, (int)value);           /* Set PB0 output pin to the requested value */

    if (ret < 0) {                                             /* Check if GPIO write operation failed */
        shell_error(sh, "Error: failed to write DIO output");  /* Print GPIO write failure message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End GPIO write check */

    shell_print(sh, "DIO_WRITE OK VALUE %ld", value);          /* Print successful write response */

    return 0;                                                  /* Return success */
}


/**
 * @brief Handle the tb_dio_read shell command.
 *
 * Command format:
 * tb_dio_read
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_dio_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                                          /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                          /* Mark argv as unused to avoid compiler warning */

    int value = gpio_pin_get_dt(&dio_in);                      /* Read PB1 input pin value */

    if (value < 0) {                                           /* Check if GPIO read operation failed */
        shell_error(sh, "Error: failed to read DIO input");    /* Print GPIO read failure message */
        return value;                                          /* Return the GPIO driver error */
    }                                                          /* End GPIO read check */

    shell_print(sh, "DIO_READ VALUE %d", value);               /* Print the input pin value */

    return 0;                                                  /* Return success */
}


/**
 * @brief Handle the tb_dio_test shell command.
 *
 * This command performs a loopback test:
 * - Set PB0 HIGH and read PB1
 * - Set PB0 LOW and read PB1
 *
 * Expected hardware connection:
 * PB0 ---- PB1
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_dio_test(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                                          /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                          /* Mark argv as unused to avoid compiler warning */

    int ret = gpio_pin_set_dt(&dio_out, 1);                    /* Set PB0 output pin HIGH */

    if (ret < 0) {                                             /* Check if setting PB0 HIGH failed */
        shell_error(sh, "Error: failed to set output HIGH");   /* Print error message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End HIGH write check */

    k_sleep(K_MSEC(10));                                       /* Wait 10 ms to allow PB1 input to become stable */

    int high_value = gpio_pin_get_dt(&dio_in);                 /* Read PB1 input after setting PB0 HIGH */

    if (high_value < 0) {                                      /* Check if reading PB1 after HIGH failed */
        shell_error(sh, "Error: failed to read input after HIGH"); /* Print read failure message */
        return high_value;                                     /* Return the GPIO driver error */
    }                                                          /* End HIGH read check */

    ret = gpio_pin_set_dt(&dio_out, 0);                        /* Set PB0 output pin LOW */

    if (ret < 0) {                                             /* Check if setting PB0 LOW failed */
        shell_error(sh, "Error: failed to set output LOW");    /* Print error message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End LOW write check */

    k_sleep(K_MSEC(10));                                       /* Wait 10 ms to allow PB1 input to become stable */

    int low_value = gpio_pin_get_dt(&dio_in);                  /* Read PB1 input after setting PB0 LOW */

    if (low_value < 0) {                                       /* Check if reading PB1 after LOW failed */
        shell_error(sh, "Error: failed to read input after LOW"); /* Print read failure message */
        return low_value;                                      /* Return the GPIO driver error */
    }                                                          /* End LOW read check */

    if ((high_value == 1) && (low_value == 0)) {                /* Check if loopback result is correct */
        shell_print(sh, "DIO_TEST PASS HIGH %d LOW %d", high_value, low_value); /* Print PASS result */
        return 0;                                              /* Return success */
    }                                                          /* End PASS condition check */

    shell_error(sh, "DIO_TEST FAIL HIGH %d LOW %d", high_value, low_value); /* Print FAIL result */

    return -EIO;                                               /* Return input/output error */
}


SHELL_CMD_REGISTER(tb_dio_write, NULL, "Write DIO output pin", cmd_tb_dio_write); /* Register tb_dio_write shell command */
SHELL_CMD_REGISTER(tb_dio_read, NULL, "Read DIO input pin", cmd_tb_dio_read);     /* Register tb_dio_read shell command */
SHELL_CMD_REGISTER(tb_dio_test, NULL, "Run DIO loopback test", cmd_tb_dio_test);  /* Register tb_dio_test shell command */