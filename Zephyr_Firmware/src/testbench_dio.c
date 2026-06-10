#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs such as k_sleep */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/gpio.h>        /* Include Zephyr GPIO driver APIs */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */
#include <errno.h>                      /* Include standard error codes */
#include <stdlib.h>                     /* Include strtol for string to number conversion */

#include "testbench_dio.h"              /* Include DIO module interface */


/* Get the devicetree alias for the DIO output pin */
#define DIO_OUT_NODE DT_ALIAS(dio_out)

/* Get the devicetree alias for the DIO input pin */
#define DIO_IN_NODE  DT_ALIAS(dio_in)


/* Create GPIO specification for the DIO output pin */
static const struct gpio_dt_spec dio_out = GPIO_DT_SPEC_GET(DIO_OUT_NODE, gpios);

/* Create GPIO specification for the DIO input pin */
static const struct gpio_dt_spec dio_in = GPIO_DT_SPEC_GET(DIO_IN_NODE, gpios);


/**
 * @brief Initialize DIO pins.
 *
 * This function configures:
 * - DIO output pin as output and starts it LOW
 * - DIO input pin as input
 *
 * Expected hardware connection:
 * DIO output pin ---- DIO input pin
 *
 * @return 0 on success, negative error code on failure.
 */
int testBench_dio_Init(void)
{
    if (!gpio_is_ready_dt(&dio_out)) {  /* Check if output GPIO port is ready */
        return -ENODEV;                 /* Return error if output GPIO port is not ready */
    }

    if (!gpio_is_ready_dt(&dio_in)) {   /* Check if input GPIO port is ready */
        return -ENODEV;                 /* Return error if input GPIO port is not ready */
    }

    int ret = gpio_pin_configure_dt(&dio_out, GPIO_OUTPUT_INACTIVE); /* Configure output pin and set it LOW */

    if (ret < 0) {                      /* Check if output configuration failed */
        return ret;                     /* Return GPIO driver error */
    }

    ret = gpio_pin_configure_dt(&dio_in, GPIO_INPUT); /* Configure input pin */

    if (ret < 0) {                      /* Check if input configuration failed */
        return ret;                     /* Return GPIO driver error */
    }

    return 0;                           /* Return success */
}


/**
 * @brief Handle tb_dio_write shell command.
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
    if (argc != 2) {                    /* Check that exactly one argument is provided */
        shell_error(sh, "Usage: tb_dio_write <0|1>"); /* Print command usage */
        return -EINVAL;                 /* Return invalid argument error */
    }

    long value = strtol(argv[1], NULL, 10); /* Convert input argument from string to number */

    if ((value != 0) && (value != 1)) { /* Check that value is only 0 or 1 */
        shell_error(sh, "Error: value must be 0 or 1"); /* Print invalid value error */
        return -EINVAL;                 /* Return invalid argument error */
    }

    int ret = gpio_pin_set_dt(&dio_out, (int)value); /* Set DIO output pin */

    if (ret < 0) {                      /* Check if GPIO write failed */
        shell_error(sh, "Error: failed to write DIO output"); /* Print write error */
        return ret;                     /* Return GPIO driver error */
    }

    shell_print(sh, "DIO_WRITE OK VALUE %ld", value); /* Print successful write response */

    return 0;                           /* Return success */
}


/**
 * @brief Handle tb_dio_read shell command.
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
    ARG_UNUSED(argc);                   /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                   /* Mark argv as unused to avoid compiler warning */

    int value = gpio_pin_get_dt(&dio_in); /* Read DIO input pin */

    if (value < 0) {                    /* Check if GPIO read failed */
        shell_error(sh, "Error: failed to read DIO input"); /* Print read error */
        return value;                   /* Return GPIO driver error */
    }

    shell_print(sh, "DIO_READ VALUE %d", value); /* Print input pin value */

    return 0;                           /* Return success */
}


/**
 * @brief Initialize DIO pins.
 *
 * This function configures:
 * - PB8 as DIO output and starts it LOW
 * - PB9 as DIO input
 *
 * Expected hardware connection:
 * PB8 ---- PB9
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_dio_test(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                   /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                   /* Mark argv as unused to avoid compiler warning */

    int ret = gpio_pin_set_dt(&dio_out, 1); /* Set DIO output HIGH */

    if (ret < 0) {                      /* Check if setting output HIGH failed */
        shell_error(sh, "Error: failed to set output HIGH"); /* Print HIGH write error */
        return ret;                     /* Return GPIO driver error */
    }

    k_sleep(K_MSEC(10));                /* Wait for input signal to stabilize */

    int high_value = gpio_pin_get_dt(&dio_in); /* Read input after setting output HIGH */

    if (high_value < 0) {               /* Check if HIGH read failed */
        shell_error(sh, "Error: failed to read input after HIGH"); /* Print HIGH read error */
        return high_value;              /* Return GPIO driver error */
    }

    ret = gpio_pin_set_dt(&dio_out, 0); /* Set DIO output LOW */

    if (ret < 0) {                      /* Check if setting output LOW failed */
        shell_error(sh, "Error: failed to set output LOW"); /* Print LOW write error */
        return ret;                     /* Return GPIO driver error */
    }

    k_sleep(K_MSEC(10));                /* Wait for input signal to stabilize */

    int low_value = gpio_pin_get_dt(&dio_in); /* Read input after setting output LOW */

    if (low_value < 0) {                /* Check if LOW read failed */
        shell_error(sh, "Error: failed to read input after LOW"); /* Print LOW read error */
        return low_value;               /* Return GPIO driver error */
    }

    if ((high_value == 1) && (low_value == 0)) { /* Check if loopback result is correct */
        shell_print(sh, "DIO_TEST PASS HIGH %d LOW %d", high_value, low_value); /* Print PASS result */
        return 0;                       /* Return success */
    }

    shell_error(sh, "DIO_TEST FAIL HIGH %d LOW %d", high_value, low_value); /* Print FAIL result */

    return -EIO;                        /* Return input/output error */
}


/* Register DIO shell commands */
SHELL_CMD_REGISTER(tb_dio_write, NULL, "Write DIO output pin", cmd_tb_dio_write);
SHELL_CMD_REGISTER(tb_dio_read, NULL, "Read DIO input pin", cmd_tb_dio_read);
SHELL_CMD_REGISTER(tb_dio_test, NULL, "Run DIO loopback test", cmd_tb_dio_test);
