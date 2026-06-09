#include <zephyr/kernel.h>               /* Include Zephyr kernel APIs */
#include <zephyr/device.h>               /* Include Zephyr device model APIs */
#include <zephyr/drivers/adc.h>          /* Include Zephyr ADC driver APIs */
#include <zephyr/shell/shell.h>          /* Include Zephyr shell APIs */
#include <zephyr/sys/util.h>             /* Include BIT macro */
#include <errno.h>                       /* Include standard error codes */

#include "adc_service.h"                 /* Include the ADC service public header */


#define ADC_NODE DT_NODELABEL(adc1)      /* Get the devicetree node label for ADC1 */
#define ADC_CHANNEL_ID 1                 /* Select ADC1 channel 1, which maps to PA1 */
#define ADC_RESOLUTION 12                /* Use 12-bit ADC resolution */
#define ADC_VREF_MV 3300                 /* Use 3300 mV as the ADC reference voltage */


static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE); /* Get ADC1 device from devicetree */


/* Create ADC channel configuration for PA1 / ADC1_IN1 */
static const struct adc_channel_cfg adc_channel_cfg = {
    .gain = ADC_GAIN_1,                          /* Use gain 1, so the input voltage is not scaled */
    .reference = ADC_REF_INTERNAL,               /* Use the ADC internal reference setting */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,    /* Use the default ADC acquisition time */
    .channel_id = ADC_CHANNEL_ID,                /* Select ADC channel 1 */
    .differential = 0                            /* Use single-ended mode, not differential mode */
};


/**
 * @brief Initialize the ADC service.
 *
 * This function checks if ADC1 is ready and configures ADC channel 1.
 *
 * @return 0 on success, negative error code on failure.
 */
int adc_service_init(void)
{
    if (!device_is_ready(adc_dev)) {                     /* Check if ADC1 device is ready */
        return -ENODEV;                                  /* Return error if ADC1 is not ready */
    }                                                    /* End ADC readiness check */

    int ret = adc_channel_setup(adc_dev, &adc_channel_cfg); /* Configure ADC channel 1 */

    if (ret < 0) {                                       /* Check if ADC channel setup failed */
        return ret;                                      /* Return the ADC setup error */
    }                                                    /* End ADC setup check */

    return 0;                                            /* Return success */
}


/**
 * @brief Handle the tb_adc_read shell command.
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
    ARG_UNUSED(argc);                                    /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                    /* Mark argv as unused to avoid compiler warning */

    int16_t sample = 0;                                  /* Create a variable to store the raw ADC sample */

    struct adc_sequence sequence = {                     /* Create ADC read sequence configuration */
        .channels = BIT(ADC_CHANNEL_ID),                 /* Select ADC channel 1 */
        .buffer = &sample,                               /* Store the ADC result in sample */
        .buffer_size = sizeof(sample),                   /* Set the buffer size */
        .resolution = ADC_RESOLUTION                     /* Set ADC resolution to 12 bits */
    };

    int ret = adc_read(adc_dev, &sequence);              /* Read one sample from ADC1 channel 1 */

    if (ret < 0) {                                       /* Check if ADC read failed */
        shell_error(sh, "ADC_READ ERROR %d", ret);       /* Print ADC read error */
        return ret;                                      /* Return the ADC driver error */
    }                                                    /* End ADC read error check */

    int32_t raw = sample;                                /* Convert raw ADC sample to 32-bit integer */

    int32_t millivolts = (raw * ADC_VREF_MV) / ((1 << ADC_RESOLUTION) - 1); /* Convert raw value to millivolts */

    shell_print(sh, "ADC_READ RAW %d MV %d", raw, millivolts); /* Print ADC raw value and voltage */

    return 0;                                            /* Return success */
}


SHELL_CMD_REGISTER(tb_adc_read, NULL, "Read ADC potentiometer value", cmd_tb_adc_read); /* Register tb_adc_read shell command */

