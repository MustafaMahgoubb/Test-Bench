#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/adc.h>         /* Include Zephyr ADC driver APIs */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */
#include <zephyr/sys/util.h>            /* Include BIT macro */
#include <errno.h>                      /* Include standard error codes */

#include "testbench_adc.h"              /* Include ADC module interface */


/* Get ADC1 node from devicetree */
#define ADC_NODE DT_NODELABEL(adc1)

/* Select ADC channel 8, mapped to PB0 / ADC1_IN8 */
#define ADC_CHANNEL_ID 8

/* Use 12-bit ADC resolution */
#define ADC_RESOLUTION 12

/* Use 3300 mV as the ADC reference voltage */
#define ADC_VREF_MV 3300


/* Get ADC device from devicetree */
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);


/* Create ADC channel configuration */
static const struct adc_channel_cfg adc_channel_cfg = {
    .gain = ADC_GAIN_1,                 /* Use gain 1, so input voltage is not scaled */
    .reference = ADC_REF_INTERNAL,      /* Use internal ADC reference configuration */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT, /* Use default acquisition time */
    .channel_id = ADC_CHANNEL_ID,       /* Select ADC channel */
    .differential = 0                   /* Use single-ended mode */
};


/**
 * @brief Initialize ADC module.
 *
 * This function checks if ADC1 is ready and configures the selected ADC channel.
 *
 * Current hardware connection:
 * - Potentiometer side pin   -> 3V3
 * - Potentiometer middle pin -> PB0 / ADC1_IN8
 * - Potentiometer side pin   -> GND
 *
 * @return 0 on success, negative error code on failure.
 */
int testBench_adc_Init(void)
{
    if (!device_is_ready(adc_dev)) {    /* Check if ADC device is ready */
        return -ENODEV;                 /* Return error if ADC device is not ready */
    }

    int ret = adc_channel_setup(adc_dev, &adc_channel_cfg); /* Configure ADC channel */

    if (ret < 0) {                      /* Check if ADC channel setup failed */
        return ret;                     /* Return ADC driver error */
    }

    return 0;                           /* Return success */
}


/**
 * @brief Handle tb_adc_read shell command.
 *
 * Command format:
 * tb_adc_read
 *
 * Response format:
 * ADC_READ RAW <raw_value> MV <millivolts>
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_adc_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                   /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                   /* Mark argv as unused to avoid compiler warning */

    int16_t sample = 0;                 /* Create variable to store raw ADC sample */

    struct adc_sequence sequence = {    /* Create ADC read sequence */
        .channels = BIT(ADC_CHANNEL_ID), /* Select ADC channel */
        .buffer = &sample,              /* Store ADC result in sample variable */
        .buffer_size = sizeof(sample),  /* Set ADC buffer size */
        .resolution = ADC_RESOLUTION    /* Set ADC resolution */
    };

    int ret = adc_read(adc_dev, &sequence); /* Read one ADC sample */

    if (ret < 0) {                      /* Check if ADC read failed */
        shell_error(sh, "ADC_READ ERROR %d", ret); /* Print ADC read error */
        return ret;                     /* Return ADC driver error */
    }

    int32_t raw = sample;               /* Convert ADC sample to 32-bit integer */

    int32_t millivolts = (raw * ADC_VREF_MV) / ((1 << ADC_RESOLUTION) - 1); /* Convert raw value to millivolts */

    shell_print(sh, "ADC_READ RAW %d MV %d", raw, millivolts); /* Print ADC result */

    return 0;                           /* Return success */
}


/* Register ADC shell command */
SHELL_CMD_REGISTER(tb_adc_read, NULL, "Read ADC potentiometer value", cmd_tb_adc_read);
