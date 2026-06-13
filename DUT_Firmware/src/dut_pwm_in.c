#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device APIs */
#include <zephyr/drivers/pwm.h>         /* Include Zephyr PWM APIs */
#include <zephyr/sys/printk.h>          /* Include printk for debug messages */

#include <errno.h>                      /* Include standard error codes */
#include <stdbool.h>                    /* Include bool type */
#include <stdint.h>                     /* Include integer types */

#include "dut_pwm_in.h"                 /* Include DUT PWM input interface */

/*

* DUT PWM input expected signals:
*
* CH1 expected signal:
* * Frequency: around 1000 Hz
* * Duty cycle: around 50%
*
* CH2 expected signal:
* * Frequency: around 1000 Hz
* * Duty cycle: around 25%
*
* These limits are used to ignore floating/noise readings.
  */
  #define DUT_PWM_MIN_FREQ_HZ        900U
  #define DUT_PWM_MAX_FREQ_HZ        1100U

#define DUT_PWM_CH1_MIN_DUTY_X100  4500U   /* 45.00% */
#define DUT_PWM_CH1_MAX_DUTY_X100  5500U   /* 55.00% */

#define DUT_PWM_CH2_MIN_DUTY_X100  2000U   /* 20.00% */
#define DUT_PWM_CH2_MAX_DUTY_X100  3000U   /* 30.00% */

/*

* Avoid printing the same valid value again and again.
*
* Frequency tolerance:
* * If the new frequency is very close to the last printed frequency,
* it is considered the same reading.
*
* Duty tolerance:
* * Duty is stored as duty * 100.
* * 20 means 0.20%.
    */
    #define DUT_PWM_FREQ_PRINT_DELTA_HZ    2U
    #define DUT_PWM_DUTY_PRINT_DELTA_X100  20U

/* Get DUT PWM input channel 1 from devicetree alias */
static const struct pwm_dt_spec dut_pwm_in_ch1 =
PWM_DT_SPEC_GET(DT_ALIAS(dut_pwmin1));

/* Get DUT PWM input channel 2 from devicetree alias */
static const struct pwm_dt_spec dut_pwm_in_ch2 =
PWM_DT_SPEC_GET(DT_ALIAS(dut_pwmin2));

/* Store the last printed frequency for each channel */
static uint32_t last_freq_hz[3] = {0U, 0U, 0U};

/* Store the last printed duty cycle for each channel */
static uint32_t last_duty_x100[3] = {0U, 0U, 0U};

/* Store whether each channel printed a valid value before */
static bool channel_printed_before[3] = {false, false, false};

/**

* @brief Calculate absolute difference between two unsigned values.
*
* @param a First value.
* @param b Second value.
*
* @return Absolute difference.
  */
  static uint32_t abs_diff_u32(uint32_t a, uint32_t b)
  {
  if (a > b) {                         /* Check if first value is larger */
  return a - b;                    /* Return positive difference */
  }

  return b - a;                        /* Return positive difference */
  }

/**

* @brief Check if captured PWM value is valid for a specific channel.
*
* @param channel_number Logical PWM input channel number.
* @param frequency_hz Captured frequency in Hz.
* @param duty_x100 Captured duty cycle multiplied by 100.
*
* @return true if the value is valid, false otherwise.
  */
  static bool dut_pwm_in_is_valid_reading(
  uint8_t channel_number,
  uint32_t frequency_hz,
  uint32_t duty_x100
  )
  {
  if (frequency_hz < DUT_PWM_MIN_FREQ_HZ) {             /* Reject low frequency noise */
  return false;
  }

  if (frequency_hz > DUT_PWM_MAX_FREQ_HZ) {             /* Reject high frequency noise */
  return false;
  }

  if (channel_number == 1U) {                           /* Validate channel 1 duty cycle */
  if (duty_x100 < DUT_PWM_CH1_MIN_DUTY_X100) {
  return false;
  }

   if (duty_x100 > DUT_PWM_CH1_MAX_DUTY_X100) {
       return false;
   }

   return true;

  }

  if (channel_number == 2U) {                           /* Validate channel 2 duty cycle */
  if (duty_x100 < DUT_PWM_CH2_MIN_DUTY_X100) {
  return false;
  }

   if (duty_x100 > DUT_PWM_CH2_MAX_DUTY_X100) {
       return false;
   }

   return true;

  }

  return false;                                         /* Reject invalid channel number */
  }

/**

* @brief Check if the reading is almost the same as the last printed reading.
*
* @param channel_number Logical PWM input channel number.
* @param frequency_hz Captured frequency in Hz.
* @param duty_x100 Captured duty cycle multiplied by 100.
*
* @return true if the reading is repeated, false otherwise.
  */
  static bool dut_pwm_in_is_repeated_reading(
  uint8_t channel_number,
  uint32_t frequency_hz,
  uint32_t duty_x100
  )
  {
  uint32_t freq_diff;                                    /* Store frequency difference */
  uint32_t duty_diff;                                    /* Store duty cycle difference */

  if (!channel_printed_before[channel_number]) {         /* First valid reading should be printed */
  return false;
  }

  freq_diff = abs_diff_u32(last_freq_hz[channel_number], frequency_hz);
  duty_diff = abs_diff_u32(last_duty_x100[channel_number], duty_x100);

  if ((freq_diff <= DUT_PWM_FREQ_PRINT_DELTA_HZ) &&
  (duty_diff <= DUT_PWM_DUTY_PRINT_DELTA_X100)) {
  return true;                                      /* Reading is close enough to the last one */
  }

  return false;                                         /* Reading changed enough to print */
  }

/**

* @brief Capture and print one PWM input channel.
*
* This function ignores invalid/noise readings.
*
* @param pwm_spec PWM device specification.
* @param channel_number Logical channel number.
  */
  static void dut_pwm_in_capture_and_print(
  const struct pwm_dt_spec *pwm_spec,
  uint8_t channel_number
  )
  {
  uint64_t period_us = 0U;                              /* Store captured period in microseconds */
  uint64_t pulse_us = 0U;                               /* Store captured pulse width in microseconds */
  uint32_t frequency_hz = 0U;                           /* Store calculated frequency */
  uint32_t duty_x100 = 0U;                              /* Store duty cycle multiplied by 100 */
  int ret;                                             /* Store return value */

  ret = pwm_capture_usec(
  pwm_spec->dev,                                   /* PWM device */
  pwm_spec->channel,                               /* PWM channel */
  PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE, /* Capture period and pulse once */
  &period_us,                                      /* Output period */
  &pulse_us,                                       /* Output pulse width */
  K_MSEC(500)                                      /* Capture timeout */
  );

  if (ret == -EBUSY) {                                 /* Check if PWM capture channel is busy */
  pwm_disable_capture(pwm_spec->dev, pwm_spec->channel);
  return;                                          /* Keep silent */
  }

  if (ret < 0) {                                       /* No valid PWM signal */
  return;                                          /* Keep silent */
  }

  pwm_disable_capture(pwm_spec->dev, pwm_spec->channel); /* Disable capture after one reading */

  if (period_us == 0U) {                               /* Protect against division by zero */
  return;                                          /* Keep silent */
  }

  frequency_hz = (uint32_t)(1000000ULL / period_us);    /* Convert period in us to Hz */

  duty_x100 = (uint32_t)((pulse_us * 10000ULL) / period_us); /* Calculate duty * 100 */

  if (!dut_pwm_in_is_valid_reading(channel_number, frequency_hz, duty_x100)) {
  return;                                          /* Ignore noise or unexpected signal */
  }

  if (dut_pwm_in_is_repeated_reading(channel_number, frequency_hz, duty_x100)) {
  return;                                          /* Avoid printing repeated stable values */
  }

  last_freq_hz[channel_number] = frequency_hz;          /* Save last printed frequency */
  last_duty_x100[channel_number] = duty_x100;           /* Save last printed duty */
  channel_printed_before[channel_number] = true;        /* Mark channel as printed before */

  printk(
  "DUT_PWM_IN CH%d Freq: %u Hz, Duty: %u.%02u\n",
  channel_number,
  frequency_hz,
  duty_x100 / 100U,
  duty_x100 % 100U
  );
  }

/**

* @brief DUT PWM input monitor thread.
*
* This thread measures PWM input channels periodically.
* It prints only valid and useful readings.
  */
  static void dut_pwm_in_thread(void *arg1, void *arg2, void *arg3)
  {
  ARG_UNUSED(arg1);                                     /* Mark arg1 as unused */
  ARG_UNUSED(arg2);                                     /* Mark arg2 as unused */
  ARG_UNUSED(arg3);                                     /* Mark arg3 as unused */

  printk("DUT PWM IN monitor thread started\n");        /* Print thread start message */

  while (1) {                                           /* Run forever */
  dut_pwm_in_capture_and_print(&dut_pwm_in_ch1, 1U); /* Capture PWM input channel 1 */
  dut_pwm_in_capture_and_print(&dut_pwm_in_ch2, 2U); /* Capture PWM input channel 2 */

   k_sleep(K_SECONDS(1));                            /* Wait before next measurement */

  }
  }

/* Define DUT PWM input monitor thread */
K_THREAD_DEFINE(
dut_pwm_in_thread_id,                                 /* Thread ID */
1024,                                                 /* Stack size */
dut_pwm_in_thread,                                    /* Thread function */
NULL,                                                 /* First argument */
NULL,                                                 /* Second argument */
NULL,                                                 /* Third argument */
7,                                                    /* Thread priority */
0,                                                    /* Thread options */
0                                                     /* Start immediately */
);

/**

* @brief Initialize DUT PWM input capture module.
*
* @return 0 on success, negative error code on failure.
  */
  int dut_pwm_in_Init(void)
  {
  if (!pwm_is_ready_dt(&dut_pwm_in_ch1)) {              /* Check if PWM input channel 1 is ready */
  printk("DUT PWM IN CH1 device not ready\n");
  return -ENODEV;
  }

  if (!pwm_is_ready_dt(&dut_pwm_in_ch2)) {              /* Check if PWM input channel 2 is ready */
  printk("DUT PWM IN CH2 device not ready\n");
  return -ENODEV;
  }

  printk("DUT PWM IN initialized\n");                   /* Print initialization message */

  return 0;                                             /* Return success */
  }
