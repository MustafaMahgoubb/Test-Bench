#ifndef DUT_DIO_H
#define DUT_DIO_H

/**
 * @brief Initialize DUT DIO mirror pins.
 *
 * This configures:
 * - PB8 as input
 * - PB9 as output
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_dio_Init(void);

/**
 * @brief Process one DIO mirror cycle.
 *
 * This reads PB8 and writes the same value to PB9.
 */
void dut_dio_Process(void);

#endif
