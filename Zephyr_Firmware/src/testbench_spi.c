#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/spi.h>         /* Include Zephyr SPI driver APIs */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */
#include <errno.h>                      /* Include standard error codes */
#include <stdlib.h>                     /* Include strtol for string to number conversion */

#include "testbench_spi.h"              /* Include SPI module interface */


/* Get SPI1 node from devicetree */
#define SPI_NODE DT_NODELABEL(spi1)

/* Set SPI transfer frequency to 1 MHz */
#define SPI_FREQUENCY_HZ 1000000U

/* Define default test byte */
#define SPI_TEST_BYTE 0xA5


/* Get SPI1 device from devicetree */
static const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);


/* Create SPI configuration */
static struct spi_config spi_cfg = {
    .frequency = SPI_FREQUENCY_HZ,                      /* Set SPI clock frequency */
    .operation = SPI_OP_MODE_MASTER |                    /* Configure SPI as master */
                 SPI_TRANSFER_MSB |                      /* Transfer most significant bit first */
                 SPI_WORD_SET(8),                        /* Use 8-bit word size */
    .slave = 0,                                          /* Select slave index 0 */
};


/**
 * @brief Initialize SPI module.
 *
 * This function checks if SPI1 is ready.
 *
 * Current hardware connection:
 * - PA5 -> SPI1_SCK
 * - PA6 -> SPI1_MISO
 * - PA7 -> SPI1_MOSI
 *
 * Loopback wire:
 * - PA7 / MOSI ---- PA6 / MISO
 *
 * @return 0 on success, negative error code on failure.
 */
int testBench_spi_Init(void)
{
    if (!device_is_ready(spi_dev)) {                     /* Check if SPI1 device is ready */
        return -ENODEV;                                  /* Return error if SPI1 is not ready */
    }

    return 0;                                            /* Return success */
}


/**
 * @brief Transfer one byte over SPI.
 *
 * @param tx_byte Byte to transmit.
 * @param rx_byte Pointer to store received byte.
 *
 * @return 0 on success, negative error code on failure.
 */
static int spi_transfer_byte(uint8_t tx_byte, uint8_t *rx_byte)
{
    uint8_t tx_buffer_data = tx_byte;                     /* Create TX byte buffer */
    uint8_t rx_buffer_data = 0;                           /* Create RX byte buffer */

    const struct spi_buf tx_buf = {                       /* Create Zephyr TX buffer */
        .buf = &tx_buffer_data,                           /* Point to TX data */
        .len = sizeof(tx_buffer_data)                     /* Set TX length to one byte */
    };

    struct spi_buf rx_buf = {                             /* Create Zephyr RX buffer */
        .buf = &rx_buffer_data,                           /* Point to RX data */
        .len = sizeof(rx_buffer_data)                     /* Set RX length to one byte */
    };

    const struct spi_buf_set tx_buf_set = {               /* Create TX buffer set */
        .buffers = &tx_buf,                               /* Point to TX buffer */
        .count = 1                                        /* Use one TX buffer */
    };

    const struct spi_buf_set rx_buf_set = {               /* Create RX buffer set */
        .buffers = &rx_buf,                               /* Point to RX buffer */
        .count = 1                                        /* Use one RX buffer */
    };

    int ret = spi_transceive(spi_dev, &spi_cfg, &tx_buf_set, &rx_buf_set); /* Send and receive one byte */

    if (ret < 0) {                                        /* Check if SPI transfer failed */
        return ret;                                       /* Return SPI driver error */
    }

    *rx_byte = rx_buffer_data;                            /* Copy received byte to caller */

    return 0;                                             /* Return success */
}


/**
 * @brief Handle tb_spi_transfer shell command.
 *
 * Command format:
 * tb_spi_transfer <byte>
 *
 * Examples:
 * tb_spi_transfer 0xA5
 * tb_spi_transfer 165
 *
 * Response format:
 * SPI_TRANSFER TX 0xA5 RX 0xA5
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_spi_transfer(const struct shell *sh, size_t argc, char **argv)
{
    if (argc != 2) {                                      /* Check that exactly one argument is provided */
        shell_error(sh, "Usage: tb_spi_transfer <byte>"); /* Print command usage */
        return -EINVAL;                                   /* Return invalid argument error */
    }

    long value = strtol(argv[1], NULL, 0);                /* Convert input string to number, base 0 supports hex */

    if ((value < 0) || (value > 255)) {                   /* Check if value fits in one byte */
        shell_error(sh, "Error: byte must be between 0x00 and 0xFF"); /* Print invalid byte error */
        return -EINVAL;                                   /* Return invalid argument error */
    }

    uint8_t tx_byte = (uint8_t)value;                     /* Convert value to unsigned 8-bit byte */
    uint8_t rx_byte = 0;                                  /* Create variable to store received byte */

    int ret = spi_transfer_byte(tx_byte, &rx_byte);       /* Transfer one byte over SPI */

    if (ret < 0) {                                        /* Check if SPI transfer failed */
        shell_error(sh, "SPI_TRANSFER ERROR %d", ret);    /* Print SPI transfer error */
        return ret;                                       /* Return SPI driver error */
    }

    shell_print(sh, "SPI_TRANSFER TX 0x%02X RX 0x%02X", tx_byte, rx_byte); /* Print transfer result */

    return 0;                                             /* Return success */
}


/**
 * @brief Handle tb_spi_test shell command.
 *
 * This command sends 0xA5 and expects to receive 0xA5.
 *
 * Expected hardware connection:
 * PA7 / MOSI ---- PA6 / MISO
 *
 * @param sh Shell instance used to print responses.
 * @param argc Number of command arguments.
 * @param argv Command arguments.
 *
 * @return 0 on success, negative error code on failure.
 */
static int cmd_tb_spi_test(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                                     /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                     /* Mark argv as unused to avoid compiler warning */

    uint8_t tx_byte = SPI_TEST_BYTE;                      /* Set default SPI test byte */
    uint8_t rx_byte = 0;                                  /* Create variable to store received byte */

    int ret = spi_transfer_byte(tx_byte, &rx_byte);       /* Transfer default byte over SPI */

    if (ret < 0) {                                        /* Check if SPI transfer failed */
        shell_error(sh, "SPI_TEST ERROR %d", ret);        /* Print SPI test error */
        return ret;                                       /* Return SPI driver error */
    }

    if (rx_byte == tx_byte) {                             /* Check if received byte matches transmitted byte */
        shell_print(sh, "SPI_TEST PASS TX 0x%02X RX 0x%02X", tx_byte, rx_byte); /* Print PASS result */
        return 0;                                         /* Return success */
    }

    shell_error(sh, "SPI_TEST FAIL TX 0x%02X RX 0x%02X", tx_byte, rx_byte); /* Print FAIL result */

    return -EIO;                                          /* Return input/output error */
}


/* Register SPI shell commands */
SHELL_CMD_REGISTER(tb_spi_transfer, NULL, "Transfer one byte over SPI", cmd_tb_spi_transfer);
SHELL_CMD_REGISTER(tb_spi_test, NULL, "Run SPI loopback test", cmd_tb_spi_test);
