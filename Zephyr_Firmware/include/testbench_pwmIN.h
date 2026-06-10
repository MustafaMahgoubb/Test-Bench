#ifndef TEST_BENCH_PWM_IN_HEADER
#define TEST_BENCH_PWM_IN_HEADER

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

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

// argv[1] is channel
int cmd_pwm_in_get_dutyCycle(const struct shell *sh, size_t argc, char **argv);

// argv[1] is channel
int cmd_pwm_in_get_frequency(const struct shell *sh, size_t argc, char** argv);

// argv[1] is channel
int cmd_pwm_in_read(const struct shell *sh, size_t argc, char** argv);


#endif