#ifndef DUT_PWM_IN_H
#define DUT_PWM_IN_H

/**
 * @brief Initialize DUT PWM input capture module.
 *
 * DUT PWM inputs:
 * - PB4 = PWM input channel 1
 * - PB5 = PWM input channel 2
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_pwm_in_Init(void);

#endif