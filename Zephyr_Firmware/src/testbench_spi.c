#include <zephyr/kernel.h>              /* Include Zephyr kernel APIs */
#include <zephyr/device.h>              /* Include Zephyr device model APIs */
#include <zephyr/drivers/spi.h>         /* Include Zephyr SPI driver APIs */
#include <zephyr/drivers/gpio.h>        /* Include Zephyr GPIO driver APIs */
#include <zephyr/shell/shell.h>         /* Include Zephyr shell APIs */
#include <errno.h>                      /* Include standard error codes */
#include <stdlib.h>                     /* Include strtol for string to number conversion */

#include "testbench_spi.h"              /* Include SPI module interface */

/* Get SPI1 node from devicetree */
#define SPI_NODE DT_NODELABEL(spi1)

/* Get SPI chip-select alias from devicetree */
#define SPI_CS_NODE DT_ALIAS(spi_cs)

/* Set SPI transfer frequency to 100 kHz for external board testing */
#define SPI_FREQUENCY_HZ 500000U
/* Define default loopback test byte */
#define SPI_LOOPBACK_TEST_BYTE 0xA5

/* Define byte sent to the external DUT slave */
#define SPI_SLAVE_PING_TX_BYTE 0xA5

/* Define expected response from the external DUT slave */
#define SPI_SLAVE_PING_EXPECTED_RX_BYTE 0x5A

/* Get SPI1 device from devicetree */
static const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);

/* Get SPI CS GPIO from devicetree */
static const struct gpio_dt_spec spi_cs = GPIO_DT_SPEC_GET(SPI_CS_NODE, gpios);

/* SPI configuration for loopback test without chip select */
static struct spi_config spi_cfg_loopback = {
.frequency = SPI_FREQUENCY_HZ,                       /* Set SPI clock frequency */
.operation = SPI_OP_MODE_MASTER |                     /* Configure SPI as master */
SPI_TRANSFER_MSB |                       /* Transfer most significant bit first */
SPI_WORD_SET(8) |                        /* Use 8-bit word size */
SPI_LINES_SINGLE,                        /* Use standard MOSI/MISO lines */
.slave = 0                                            /* Select slave index 0 */
};

/*

* SPI configuration for external DUT slave test.
*
* Chip select is controlled manually using GPIO PA4.
* Do not use .cs here.
  */
  static struct spi_config spi_cfg_slave = {
  .frequency = SPI_FREQUENCY_HZ,                       /* Set SPI clock frequency */
  .operation = SPI_OP_MODE_MASTER |                     /* Configure SPI as master */
  SPI_TRANSFER_MSB |                       /* Transfer most significant bit first */
  SPI_WORD_SET(8) |                        /* Use 8-bit word size */
  SPI_LINES_SINGLE,                        /* Use standard MOSI/MISO lines */
  .slave = 0                                            /* Select slave index 0 */
  };

/**

* @brief Initialize SPI module.
*
* This function checks:
* * SPI1 device is ready
* * CS GPIO is ready
*
* Hardware connections:
*
* Loopback test:
* * PA7 / MOSI ---- PA6 / MISO
*
* External DUT test:
* * Board 1 PA4 / CS   -> Board 2 PA4 / NSS
* * Board 1 PA5 / SCK  -> Board 2 PA5 / SCK
* * Board 1 PA7 / MOSI -> Board 2 PA7 / MOSI
* * Board 1 PA6 / MISO <- Board 2 PA6 / MISO
* * Board 1 GND        -> Board 2 GND
*
* @return 0 on success, negative error code on failure.
  */
  int testBench_spi_Init(void)
  {
  if (!device_is_ready(spi_dev)) {                      /* Check if SPI1 device is ready */
  return -ENODEV;                                   /* Return error if SPI1 is not ready */
  }

  if (!gpio_is_ready_dt(&spi_cs)) {                     /* Check if CS GPIO port is ready */
  return -ENODEV;                                   /* Return error if CS GPIO port is not ready */
  }

  int ret = gpio_pin_configure_dt(&spi_cs, GPIO_OUTPUT_INACTIVE); /* Configure CS inactive */

  if (ret < 0) {                                        /* Check if CS configuration failed */
  return ret;                                       /* Return GPIO driver error */
  }

  return 0;                                             /* Return success */
  }

/**

* @brief Transfer one byte over SPI using a selected SPI configuration.
*
* @param cfg Pointer to SPI configuration.
* @param tx_byte Byte to transmit.
* @param rx_byte Pointer to store received byte.
*
* @return 0 on success, negative error code on failure.
  */
  static int spi_transfer_byte_with_config(struct spi_config *cfg, uint8_t tx_byte, uint8_t *rx_byte)
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

  int ret = spi_transceive(spi_dev, cfg, &tx_buf_set, &rx_buf_set); /* Send and receive */

  if (ret < 0) {                                        /* Check if SPI transfer failed */
  return ret;                                       /* Return SPI driver error */
  }

  *rx_byte = rx_buffer_data;                            /* Copy received byte to caller */

  return 0;                                             /* Return success */
  }

/**

* @brief Transfer one byte over SPI using loopback configuration.
*
* This function is used for bring-up testing:
* PA7 / MOSI ---- PA6 / MISO
*
* @param tx_byte Byte to transmit.
* @param rx_byte Pointer to store received byte.
*
* @return 0 on success, negative error code on failure.
  */
  static int spi_loopback_transfer_byte(uint8_t tx_byte, uint8_t *rx_byte)
  {
  return spi_transfer_byte_with_config(&spi_cfg_loopback, tx_byte, rx_byte);
  }

/**

* @brief Transfer one byte over SPI using external DUT slave configuration.
*
* This function manually controls CS on PA4.
*
* GPIO_ACTIVE_LOW behavior:
* * gpio_pin_set_dt(&spi_cs, 1) drives physical LOW  -> slave selected
* * gpio_pin_set_dt(&spi_cs, 0) drives physical HIGH -> slave deselected
*
* @param tx_byte Byte to transmit.
* @param rx_byte Pointer to store received byte.
*
* @return 0 on success, negative error code on failure.
  */
  static int spi_slave_transfer_byte(uint8_t tx_byte, uint8_t *rx_byte)
  {
  int ret;
  int cs_ret;

  ret = gpio_pin_set_dt(&spi_cs, 1);                    /* Select slave: CS LOW */
  if (ret < 0) {                                        /* Check if CS assert failed */
  return ret;                                       /* Return GPIO driver error */
  }

  k_busy_wait(50);                                      /* Wait before SPI clock starts */

  ret = spi_transfer_byte_with_config(&spi_cfg_slave, tx_byte, rx_byte); /* Transfer byte */

  k_busy_wait(50);                                      /* Wait before releasing CS */

  cs_ret = gpio_pin_set_dt(&spi_cs, 0);                 /* Deselect slave: CS HIGH */

  if (ret < 0) {                                        /* Check if SPI transfer failed */
  return ret;                                       /* Return SPI driver error */
  }

  if (cs_ret < 0) {                                     /* Check if CS deassert failed */
  return cs_ret;                                    /* Return GPIO driver error */
  }

  return 0;                                             /* Return success */
  }

/**

* @brief Handle tb_spi_transfer shell command.
*
* This command is used for loopback bring-up testing.
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

  long value = strtol(argv[1], NULL, 0);                /* Convert input string to number */

  if ((value < 0) || (value > 255)) {                   /* Check if value fits in one byte */
  shell_error(sh, "Error: byte must be between 0x00 and 0xFF");
  return -EINVAL;                                   /* Return invalid argument error */
  }

  uint8_t tx_byte = (uint8_t)value;                     /* Convert value to unsigned 8-bit byte */
  uint8_t rx_byte = 0;                                  /* Create variable to store received byte */

  int ret = spi_loopback_transfer_byte(tx_byte, &rx_byte); /* Transfer one byte using loopback */

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
* This command is a loopback bring-up test.
*
* It sends 0xA5 and expects to receive 0xA5.
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
  ARG_UNUSED(argc);                                     /* Mark argc as unused */
  ARG_UNUSED(argv);                                     /* Mark argv as unused */

  uint8_t tx_byte = SPI_LOOPBACK_TEST_BYTE;             /* Set default loopback test byte */
  uint8_t rx_byte = 0;                                  /* Create variable to store received byte */

  int ret = spi_loopback_transfer_byte(tx_byte, &rx_byte); /* Transfer default byte using loopback */

  if (ret < 0) {                                        /* Check if SPI transfer failed */
  shell_error(sh, "SPI_TEST ERROR %d", ret);        /* Print SPI test error */
  return ret;                                       /* Return SPI driver error */
  }

  if (rx_byte == tx_byte) {                             /* Check if received byte matches transmitted byte */
  shell_print(sh, "SPI_TEST PASS TX 0x%02X RX 0x%02X", tx_byte, rx_byte);
  return 0;                                         /* Return success */
  }

  shell_error(sh, "SPI_TEST FAIL TX 0x%02X RX 0x%02X", tx_byte, rx_byte); /* Print FAIL result */

  return -EIO;                                          /* Return input/output error */
  }

/**

* @brief Handle tb_spi_slave_ping shell command.
*
* This command tests communication with the external DUT board.
*
* Master sends:
* * 0xA5
*
* DUT slave is expected to reply:
* * 0x5A
*
* Expected response:
* SPI_SLAVE_PING PASS TX 0xA5 RX 0x5A
*
* @param sh Shell instance used to print responses.
* @param argc Number of command arguments.
* @param argv Command arguments.
*
* @return 0 on success, negative error code on failure.
  */
  static int cmd_tb_spi_slave_ping(const struct shell *sh, size_t argc, char **argv)
  {
  ARG_UNUSED(argc);                                     /* Mark argc as unused */
  ARG_UNUSED(argv);                                     /* Mark argv as unused */

  uint8_t tx_byte = SPI_SLAVE_PING_TX_BYTE;             /* Set slave ping TX byte */
  uint8_t rx_byte = 0;                                  /* Create variable to store received byte */

  int ret = spi_slave_transfer_byte(tx_byte, &rx_byte); /* Transfer byte using external DUT slave */

  if (ret < 0) {                                        /* Check if SPI transfer failed */
  shell_error(sh, "SPI_SLAVE_PING ERROR %d", ret);  /* Print SPI slave ping error */
  return ret;                                       /* Return SPI driver error */
  }

  if (rx_byte == SPI_SLAVE_PING_EXPECTED_RX_BYTE) {     /* Check if received byte matches expected response */
  shell_print(sh, "SPI_SLAVE_PING PASS TX 0x%02X RX 0x%02X", tx_byte, rx_byte);
  return 0;                                         /* Return success */
  }

  shell_error(sh,
  "SPI_SLAVE_PING FAIL TX 0x%02X RX 0x%02X EXPECTED 0x%02X",
  tx_byte,
  rx_byte,
  SPI_SLAVE_PING_EXPECTED_RX_BYTE);         /* Print FAIL result */

  return -EIO;                                          /* Return input/output error */
  }

/* Register SPI shell commands */
SHELL_CMD_REGISTER(tb_spi_transfer, NULL, "Transfer one byte over SPI loopback", cmd_tb_spi_transfer);
SHELL_CMD_REGISTER(tb_spi_test, NULL, "Run SPI loopback test", cmd_tb_spi_test);
SHELL_CMD_REGISTER(tb_spi_slave_ping, NULL, "Run SPI external DUT slave ping test", cmd_tb_spi_slave_ping);
