#ifndef DIO_SERVICE_H                    /* Prevent this header file from being included more than once */
#define DIO_SERVICE_H                    /* Define the include guard macro */

/**
 * @brief Initialize the DIO service pins.
 *
 * This function configures:
 * - PB0 as DIO output
 * - PB1 as DIO input
 *
 * @return 0 on success, negative error code on failure.
 */
int dio_service_init(void);              /* Declare the DIO service initialization function */

#endif                                   /* End of include guard */