#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/gpio.h>        /* Include Zephyr GPIO driver APIs */
#include <errno.h>                      /* Include standard error codes */

#include "dut_dio.h"                    /* Include DUT DIO module interface */


/* Get DUT DIO input alias from devicetree */
#define DUT_DIO_IN_NODE  DT_ALIAS(dut_dio_in)

/* Get DUT DIO output alias from devicetree */
#define DUT_DIO_OUT_NODE DT_ALIAS(dut_dio_out)


/* Create GPIO specification for PB8 input */
static const struct gpio_dt_spec dut_dio_in = GPIO_DT_SPEC_GET(DUT_DIO_IN_NODE, gpios);

/* Create GPIO specification for PB9 output */
static const struct gpio_dt_spec dut_dio_out = GPIO_DT_SPEC_GET(DUT_DIO_OUT_NODE, gpios);


/**
 * @brief Initialize DUT DIO mirror pins.
 *
 * Hardware connection:
 * - Board 1 PB8 -> Board 2 PB8
 * - Board 2 PB9 -> Board 1 PB9
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_dio_Init(void)
{
    if (!gpio_is_ready_dt(&dut_dio_in)) {                         /* Check if input GPIO port is ready */
        return -ENODEV;                                           /* Return error if input GPIO port is not ready */
    }

    if (!gpio_is_ready_dt(&dut_dio_out)) {                        /* Check if output GPIO port is ready */
        return -ENODEV;                                           /* Return error if output GPIO port is not ready */
    }

    int ret = gpio_pin_configure_dt(&dut_dio_in, GPIO_INPUT);     /* Configure PB8 as input */

    if (ret < 0) {                                                /* Check if input configuration failed */
        return ret;                                               /* Return GPIO driver error */
    }

    ret = gpio_pin_configure_dt(&dut_dio_out, GPIO_OUTPUT_INACTIVE); /* Configure PB9 as output and start LOW */

    if (ret < 0) {                                                /* Check if output configuration failed */
        return ret;                                               /* Return GPIO driver error */
    }

    return 0;                                                     /* Return success */
}


/**
 * @brief Process one DIO mirror cycle.
 *
 * This function reads PB8 and writes the same value to PB9.
 */
void dut_dio_Process(void)
{
    int value = gpio_pin_get_dt(&dut_dio_in);                     /* Read PB8 input value */

    if (value < 0) {                                              /* Check if GPIO read failed */
        return;                                                   /* Ignore this cycle if read failed */
    }

    gpio_pin_set_dt(&dut_dio_out, value);                         /* Mirror PB8 value to PB9 */
}

