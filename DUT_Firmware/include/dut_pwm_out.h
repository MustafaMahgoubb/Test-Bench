#ifndef DUT_PWM_OUT_H
#define DUT_PWM_OUT_H

/**
 * @brief Initialize DUT PWM output generator.
 *
 * DUT PWM outputs:
 * - PA0 = PWM output channel 1
 * - PA1 = PWM output channel 2
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_pwm_out_Init(void);

#endif
