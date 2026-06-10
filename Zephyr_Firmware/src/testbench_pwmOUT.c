#include <zephyr/shell/shell.h>
#include "testbench_pwmOUT.h"
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * The Test Bench provides pwm output features on PA0 and PA1 (timer2 Ch1,Ch2)
 *
 *
 * This Driver provide the following APIs
 * pwm_out set channel_number dutyCycle%
 * Describtion
 *  this Command generate PWM signal for the Channel_num of dutyCycle% with default frequency 1000 Hz
 * Example
 *  pwm_out set 1 50       (Sets channel 1 to 50% duty at default freq)
 *
 * pwm_out freq channel_number frequency_Value dutyCycle%
 * Description
 *  this Command generate PWM Signal for the Channel_num of dutyCycle% and frequency of frequency_Value
 * Example
 *  pwm_out freq 1 1000 25 (Sets channel 1 to 1kHz at 25% duty)
 *
 * pwm_out stop channel_number
 * Description
 *  this Command stop the PWM Signal for the Channel_number (dutyCycle = 0)
 *
*/


#define MIN_FREQUENCY 1
#define MAX_FREQUENCY 1000000ul
#define DEFAULT_FREQUENCY 1000 // 1000 Hz

#define PWM_CHANNEL1 1
#define PWM_CHANNEL2 2

#define SUCCESS 0
#define INVALID_CHANNEL 1
#define INVALID_DUTYCYCLE 2
#define INVALID_FREQUENCY 3
#define DEVICE_NOT_READY 4




static const struct pwm_dt_spec pwm_ch1 = PWM_DT_SPEC_GET(DT_ALIAS(pwmout1));
static const struct pwm_dt_spec pwm_ch2 = PWM_DT_SPEC_GET(DT_ALIAS(pwmout2));

// Commands implementations
// argv[1] is channel, argv[2] is duty cycle
int cmd_pwm_out_set(const struct shell *sh, size_t argc, char **argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);

    char *endptr;
    float dutyCycle = (float)strtof(argv[2], &endptr);
    if (dutyCycle < 0 || dutyCycle > 100)
    {
        shell_print(sh, "Invalid Duty Cycle\n available DutyCycle 0~100");
        return -INVALID_DUTYCYCLE;
    }

    uint32_t period_ns = 1000000000ull / DEFAULT_FREQUENCY;
    uint32_t pulse_ns = (uint32_t)((dutyCycle / 100.0f) * period_ns);

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!pwm_is_ready_dt(&pwm_ch1)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}

        // pwm_set_dt (const struct pwm_dt_spec *spec, uint32_t period, uint32_t pulse)
        pwm_set_dt(&pwm_ch1,period_ns,pulse_ns);
        break;
    case PWM_CHANNEL2:
        if (!pwm_is_ready_dt(&pwm_ch2)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}
        pwm_set_dt(&pwm_ch2,period_ns,pulse_ns);
        break;
    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    return SUCCESS; // Success
}

// argv[1] is channel, argv[2] is frequency, argv[3] is duty cycle
int cmd_pwm_out_freq(const struct shell *sh, size_t argc, char** argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);

    uint32_t freq = (uint32_t)atoi(argv[2]);
    if (freq < MIN_FREQUENCY || freq > MAX_FREQUENCY)
    {
        shell_print(sh, "Invalid Frequency\n available Frequency 1~1MHz");
        return -INVALID_FREQUENCY;
    }

    char *endptr;
    float dutyCycle = (float)strtof(argv[3], &endptr);
    if (dutyCycle < 0 || dutyCycle > 100)
    {
        shell_print(sh, "Invalid Duty Cycle\n available DutyCycle 0~100");
        return -INVALID_DUTYCYCLE;
    }

    uint32_t period_ns = 1000000000ull / freq;
    uint32_t pulse_ns = (uint32_t)((dutyCycle / 100.0f) * period_ns);

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!pwm_is_ready_dt(&pwm_ch1)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}

        // pwm_set_dt (const struct pwm_dt_spec *spec, uint32_t period, uint32_t pulse)
        pwm_set_dt(&pwm_ch1,period_ns,pulse_ns);
        break;
    case PWM_CHANNEL2:
        if (!pwm_is_ready_dt(&pwm_ch2)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}
        pwm_set_dt(&pwm_ch2,period_ns,pulse_ns);
        break;
    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    return SUCCESS; // Success
}

int cmd_pwm_stop(const struct shell *sh, size_t argc, char** argv)
{
    // arguments validation
    uint8_t pwmChannel = (uint8_t)atoi(argv[1]);
    
    // To stop the PWM, we just set the pulse (duty cycle) to 0. 
    // The period doesn't matter, we'll just use the default.
    uint32_t period_ns = 1000000000ull / DEFAULT_FREQUENCY;
    uint32_t pulse_ns = 0;

    switch (pwmChannel)
    {
    case PWM_CHANNEL1:
        if (!pwm_is_ready_dt(&pwm_ch1)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}

        // pwm_set_dt (const struct pwm_dt_spec *spec, uint32_t period, uint32_t pulse)
        pwm_set_dt(&pwm_ch1,period_ns,pulse_ns);
        break;
    case PWM_CHANNEL2:
        if (!pwm_is_ready_dt(&pwm_ch2)) {
            shell_print(sh, "ERR Device Not Ready\n");
			return -DEVICE_NOT_READY;
		}
        pwm_set_dt(&pwm_ch2,period_ns,pulse_ns);
        break;
    default:
        shell_print(sh, "Invalid PWM Channel\n Available Channels are 1 and 2");
        return -INVALID_CHANNEL;
        break;
    }

    return SUCCESS; // Success
}


/*
 * Create The Subcommands
 *
 *
 */

SHELL_STATIC_SUBCMD_SET_CREATE(
    pwm_out_subcmds,
    SHELL_CMD_ARG(set, NULL, "this Command generate PWM signal for the Channel_num of dutyCycle% with default frequency 1000 Hz",
              cmd_pwm_out_set, 3, 0),

    SHELL_CMD_ARG(freq, NULL, "this Command generate PWM Signal for the Channel_num of dutyCycle% and frequency of frequency_Value",
              cmd_pwm_out_freq, 4, 0),

    SHELL_CMD_ARG(stop, NULL, "this Command stop the PWM Signal for the Channel_number (dutyCycle = 0)",
              cmd_pwm_stop, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(pwm_out, &pwm_out_subcmds, "PWM Output Commands", NULL);