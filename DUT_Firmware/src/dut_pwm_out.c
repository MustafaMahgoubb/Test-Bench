#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device APIs */
#include <zephyr/drivers/pwm.h>         /* Include Zephyr PWM driver APIs */
#include <zephyr/sys/printk.h>          /* Include printk for debug messages */
#include <errno.h>                      /* Include standard error codes */

#include "dut_pwm_out.h"                /* Include DUT PWM output interface */


/* Get PWM output channel 1 from devicetree alias */
static const struct pwm_dt_spec dut_pwm_ch1 = PWM_DT_SPEC_GET(DT_ALIAS(dut_pwmout1));

/* Get PWM output channel 2 from devicetree alias */
static const struct pwm_dt_spec dut_pwm_ch2 = PWM_DT_SPEC_GET(DT_ALIAS(dut_pwmout2));


/**
 * @brief Initialize DUT PWM output generator.
 *
 * Channel 1:
 * - Frequency = 1000 Hz
 * - Duty cycle = 50%
 *
 * Channel 2:
 * - Frequency = 1000 Hz
 * - Duty cycle = 25%
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_pwm_out_Init(void)
{
    int ret;                                             /* Store return value */

    const uint32_t period_ns = 1000000U;                 /* 1 kHz period = 1 ms = 1,000,000 ns */

    const uint32_t ch1_pulse_ns = 500000U;               /* 50% duty cycle */
    const uint32_t ch2_pulse_ns = 250000U;               /* 25% duty cycle */

    if (!pwm_is_ready_dt(&dut_pwm_ch1)) {                /* Check if PWM channel 1 is ready */
        printk("DUT PWM CH1 device not ready\n");        /* Print error message */
        return -ENODEV;                                  /* Return device error */
    }

    if (!pwm_is_ready_dt(&dut_pwm_ch2)) {                /* Check if PWM channel 2 is ready */
        printk("DUT PWM CH2 device not ready\n");        /* Print error message */
        return -ENODEV;                                  /* Return device error */
    }

    ret = pwm_set_dt(&dut_pwm_ch1, period_ns, ch1_pulse_ns); /* Start PWM channel 1 */

    if (ret < 0) {                                       /* Check if channel 1 failed */
        printk("DUT PWM CH1 start failed: %d\n", ret);   /* Print error */
        return ret;                                      /* Return error */
    }

    ret = pwm_set_dt(&dut_pwm_ch2, period_ns, ch2_pulse_ns); /* Start PWM channel 2 */

    if (ret < 0) {                                       /* Check if channel 2 failed */
        printk("DUT PWM CH2 start failed: %d\n", ret);   /* Print error */
        return ret;                                      /* Return error */
    }

    printk("DUT PWM OUT started: CH1 1000Hz 50%%, CH2 1000Hz 25%%\n");

    return 0;                                            /* Return success */
}
