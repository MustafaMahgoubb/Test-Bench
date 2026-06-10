#ifndef TESTBENCH_SPI_H
#define TESTBENCH_SPI_H

/**
 * @brief Initialize SPI module.
 *
 * This function checks if SPI1 is ready.
 *
 * Hardware pins:
 * - PA5 -> SPI1_SCK
 * - PA6 -> SPI1_MISO
 * - PA7 -> SPI1_MOSI
 *
 * Loopback connection:
 * - PA7 / MOSI ---- PA6 / MISO
 *
 * @return 0 on success, negative error code on failure.
 */
int testBench_spi_Init(void);

#endif
