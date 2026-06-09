#include <zephyr/shell/shell.h>
#include "testbench_pwmIN.h"
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <stdlib.h>
#include <stdint.h>

#define PWM_CHANNEL1 1
#define PWM_CHANNEL2 2

#define SUCCESS 0
#define INVALID_CHANNEL 1
#define DEVICE_NOT_READY 2
#define ERROR_CAPTURING 3
#define ERROR_BUSY 4

static const struct pwm_dt_spec pwm_ch1 = PWM_DT_SPEC_GET(DT_ALIAS(pwmin1));
static const struct pwm_dt_spec pwm_ch2 = PWM_DT_SPEC_GET(DT_ALIAS(pwmin2));

/*
 * The Test Bench provides pwm input features on PB4 and PB5 (timer3 Ch1,Ch2)
 *
 *
 * This Driver provide the following APIs
 * pwm_in get_dutyCycle channel_number
 * Describtion
 *  this Command calculate PWM duty Cycle for the Channel_num
 * Example
 *  pwm_in get_dutyCycle 1       (gets channel 1 dutyCycle Value)
 *
 * pwm_in get_frequency channel_number
 * Description
 *  this Command calculate PWM Frequency for the Channel_num
 * Example
 *  pwm_in get_frequency 1 (gets channel 1 dutyCycle Value)
 *
 * pwm_in read channel_number
 * Description
 *  this Command read both the frequency and dutyCycle for Channel_number
 *
 */

int cmd_pwm_in_read(const struct shell *sh, size_t argc, char **argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);
    uint64_t period_us;
    uint64_t pulse_us;

    float dutyCycle = 0;
    uint32_t freq = 0;

    int ret;

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!device_is_ready(pwm_ch1.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }

        // Calculate DutyCycle and Frequency
        // pwm_capture_usec (const struct device *dev, uint32_t channel, pwm_flags_t flags, uint64_t *period, uint64_t *pulse, k_timeout_t timeout)

        /* Flags Infos

        PWM capture configuration flags

        #define 	PWM_CAPTURE_TYPE_PERIOD   (1U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures period.
        #define 	PWM_CAPTURE_TYPE_PULSE   (2U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures pulse width.
        #define 	PWM_CAPTURE_TYPE_BOTH
            PWM pin capture captures both period and pulse width.
        #define 	PWM_CAPTURE_MODE_SINGLE   (0U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures a single period/pulse width.
        #define 	PWM_CAPTURE_MODE_CONTINUOUS   (1U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures period/pulse width continuously.
*/
        ret = pwm_capture_usec(pwm_ch1.dev, pwm_ch1.channel, (PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch1.dev, pwm_ch1.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;
    case PWM_CHANNEL2:
        if (!device_is_ready(pwm_ch2.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }

        ret = pwm_capture_usec(pwm_ch2.dev, pwm_ch2.channel, (PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch2.dev, pwm_ch2.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;

    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    freq = (uint32_t)(1000000 / period_us);
    dutyCycle = (float)(((double)pulse_us / period_us) * 100);

    shell_print(sh, "Freq: %u Hz, Duty: %.2f", freq, dutyCycle);
    return SUCCESS; // Success
}


int cmd_pwm_in_get_frequency(const struct shell *sh, size_t argc, char** argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);
    uint64_t period_us;
    uint64_t pulse_us;

    uint32_t freq = 0;

    int ret;

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!device_is_ready(pwm_ch1.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }


        // Calculate DutyCycle and Frequency
        // pwm_capture_usec (const struct device *dev, uint32_t channel, pwm_flags_t flags, uint64_t *period, uint64_t *pulse, k_timeout_t timeout)

        /* Flags Infos

        PWM capture configuration flags

        #define 	PWM_CAPTURE_TYPE_PERIOD   (1U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures period.
        #define 	PWM_CAPTURE_TYPE_PULSE   (2U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures pulse width.
        #define 	PWM_CAPTURE_TYPE_BOTH
            PWM pin capture captures both period and pulse width.
        #define 	PWM_CAPTURE_MODE_SINGLE   (0U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures a single period/pulse width.
        #define 	PWM_CAPTURE_MODE_CONTINUOUS   (1U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures period/pulse width continuously.
*/
        ret = pwm_capture_usec(pwm_ch1.dev, pwm_ch1.channel, (PWM_CAPTURE_TYPE_PERIOD | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch1.dev, pwm_ch1.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;
    case PWM_CHANNEL2:
        if (!device_is_ready(pwm_ch2.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }

        ret = pwm_capture_usec(pwm_ch2.dev, pwm_ch2.channel, (PWM_CAPTURE_TYPE_PERIOD | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch2.dev, pwm_ch2.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;

    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    freq = (uint32_t)(1000000 / period_us);

    shell_print(sh, "Freq: %u Hz", freq);
    return SUCCESS; // Success
}

int cmd_pwm_in_get_dutyCycle(const struct shell *sh, size_t argc, char **argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);
    uint64_t period_us;
    uint64_t pulse_us;

    float dutyCycle = 0;

    int ret;

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!device_is_ready(pwm_ch1.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }


        // Calculate DutyCycle and Frequency
        // pwm_capture_usec (const struct device *dev, uint32_t channel, pwm_flags_t flags, uint64_t *period, uint64_t *pulse, k_timeout_t timeout)

        /* Flags Infos

        PWM capture configuration flags

        #define 	PWM_CAPTURE_TYPE_PERIOD   (1U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures period.
        #define 	PWM_CAPTURE_TYPE_PULSE   (2U << PWM_CAPTURE_TYPE_SHIFT)
            PWM pin capture captures pulse width.
        #define 	PWM_CAPTURE_TYPE_BOTH
            PWM pin capture captures both period and pulse width.
        #define 	PWM_CAPTURE_MODE_SINGLE   (0U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures a single period/pulse width.
        #define 	PWM_CAPTURE_MODE_CONTINUOUS   (1U << PWM_CAPTURE_MODE_SHIFT)
            PWM pin capture captures period/pulse width continuously.
*/
        ret = pwm_capture_usec(pwm_ch1.dev, pwm_ch1.channel, (PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch1.dev, pwm_ch1.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;
    case PWM_CHANNEL2:
        if (!device_is_ready(pwm_ch2.dev))
        {
            shell_print(sh, "ERR Device Not Ready\n");
            return -DEVICE_NOT_READY;
        }

        ret = pwm_capture_usec(pwm_ch2.dev, pwm_ch2.channel, (PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE),
                               &period_us, &pulse_us, Z_TIMEOUT_MS(500));

        if (ret == -EBUSY)
        {
            shell_print(sh, "ERR Channel Is Busy\n");
            pwm_disable_capture(pwm_ch2.dev, pwm_ch2.channel);
            return -ERROR_BUSY;
        }

        if (ret)
        {
            shell_print(sh, "ERR Capturing Failed (%d)\n", ret);
            return -ERROR_CAPTURING;
        }


        break;

    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    dutyCycle = (float)(((double)pulse_us / period_us) * 100);

    shell_print(sh, "Duty: %.2f", dutyCycle);
    return SUCCESS; // Success
}

/*
 * Create The Subcommands
 *
 *
 */

SHELL_STATIC_SUBCMD_SET_CREATE(
    pwm_in_subcmds,
    SHELL_CMD_ARG(read, NULL, "this Command read both the frequency and dutyCycle for Channel_number",
              cmd_pwm_in_read, 2, 0),

    SHELL_CMD_ARG(get_dutyCycle, NULL, "this Command calculate PWM duty Cycle for the Channel_num",
              cmd_pwm_in_get_dutyCycle, 2, 0),

    SHELL_CMD_ARG(get_frequency, NULL, "this Command calculate PWM Frequency for the Channel_num",
              cmd_pwm_in_get_frequency, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(pwm_in, &pwm_in_subcmds, "PWM Capture Input Commands", NULL);