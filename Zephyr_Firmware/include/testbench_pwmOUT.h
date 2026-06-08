#ifndef TEST_BENCH_PWM_OUT_HEADER
#define TEST_BENCH_PWM_OUT_HEADER

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
/* 
 * The Test Bench provides pwm output features on PA0 and PA1 (timer2 Ch1,Ch2)
 * 
 * 
 * This Driver provide the following APIs
 * pwm_out set channel_number dutyCycle%
 * Describtion 
 *  this Command generate PWM signal for the Channel_num of dutyCycle% with default frequency 1000 Hz
 * Example
 *  pwm set 1 50       (Sets channel 1 to 50% duty at default freq)
 * 
 * pwm_out freq channel_number frequency_Value dutyCycle% 
 * Description
 *  this Command generate PWM Signal for the Channel_num of dutyCycle% and frequency of frequency_Value
 * Example
 *  pwm freq 1 1000 25 (Sets channel 1 to 1kHz at 25% duty)
 * 
 * pwm_out stop channel_number
 * Description
 *  this Command stop the PWM Signal for the Channel_number (dutyCycle = 0)
 * 
*/

// argv[1] is channel, argv[2] is duty cycle
int cmd_pwm_out_set(const struct shell *sh, size_t argc, char **argv);

// argv[1] is channel, argv[2] is frequency, argv[3] is duty cycle
int cmd_pwm_out_freq(const struct shell *sh, size_t argc, char** argv);

// argv[1] is channel
int cmd_pwm_stop(const struct shell *sh, size_t argc, char** argv);


#endif