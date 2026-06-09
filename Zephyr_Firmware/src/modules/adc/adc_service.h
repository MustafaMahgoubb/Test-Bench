#ifndef ADC_SERVICE_H                    /* Prevent this header file from being included more than once */
#define ADC_SERVICE_H                    /* Define the include guard macro */

/**
 * @brief Initialize the ADC service.
 *
 * This function configures ADC1 channel 1.
 *
 * Hardware pin:
 * - PA1 -> ADC1_IN1
 *
 * @return 0 on success, negative error code on failure.
 */
int adc_service_init(void);              /* Declare the ADC service initialization function */

#endif                                   /* End of include guard */