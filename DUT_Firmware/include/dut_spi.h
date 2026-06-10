#ifndef DUT_SPI_H
#define DUT_SPI_H

/**
 * @brief Initialize DUT SPI slave responder.
 *
 * This configures SPI1 as a slave responder.
 *
 * @return 0 on success, negative error code on failure.
 */
int dut_spi_Init(void);

#endif