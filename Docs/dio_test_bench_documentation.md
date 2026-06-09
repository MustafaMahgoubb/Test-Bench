# DIO Test Bench Documentation

## 1. Overview

This document explains the DIO test bench implementation using:

- STM32F401 Blackpill
- Zephyr RTOS
- Zephyr Shell
- Python Host scripts
- Robot Framework

The goal of this phase is to test a Digital Input / Output loopback.

The test uses two GPIO pins:

```text
PB0 -> Digital Output
PB1 -> Digital Input
```

The two pins must be connected together:

```text
PB0 ---- PB1
```

When PB0 is set HIGH, PB1 should read HIGH.

When PB0 is set LOW, PB1 should read LOW.

---

## 2. Hardware Connections

### 2.1 DIO Loopback Connection

Connect PB0 to PB1 using a jumper wire:

```text
STM32F401 Blackpill

PB0  ----------------  PB1
OUT                    IN
```

### 2.2 UART Shell Connection

The Zephyr Shell runs over USART1.

Connect the USB-to-TTL adapter like this:

```text
USB-TTL RX  -> PA9  USART1_TX
USB-TTL TX  -> PA10 USART1_RX
USB-TTL GND -> GND
```

Default baud rate:

```text
115200
```

### 2.3 Debug and Flash Pins

Use SWD for flashing:

```text
ST-Link SWDIO -> PA13
ST-Link SWCLK -> PA14
ST-Link GND   -> GND
ST-Link 3V3   -> 3V3
```

Do not use PA13 or PA14 for test pins because they are used for flashing and debugging.

---

## 3. Full System Flow

```text
+----------------------+
| Robot Framework      |
| 01_dio.robot         |
+----------+-----------+
           |
           | Calls keywords
           v
+----------------------+
| Python Robot Library |
| testbench_serial.py  |
+----------+-----------+
           |
           | Sends shell command using pyserial
           v
+----------------------+
| Linux Serial Port    |
| /dev/ttyUSB0         |
+----------+-----------+
           |
           | USB-to-TTL UART
           v
+----------------------+
| STM32 USART1         |
| PA9 TX / PA10 RX     |
+----------+-----------+
           |
           | Zephyr Shell receives command
           v
+----------------------+
| Zephyr Shell         |
| tb_dio_test          |
+----------+-----------+
           |
           | Calls C handler function
           v
+----------------------+
| Zephyr GPIO API      |
| gpio_pin_set_dt()    |
| gpio_pin_get_dt()    |
+----------+-----------+
           |
           | Controls GPIO pins
           v
+----------------------+
| PB0 ---- PB1         |
| OUT      IN          |
+----------------------+
```

---

## 4. Project Files Used in DIO Phase

```text
Test-Bench
├── Zephyr_Firmware
│   ├── boards
│   │   └── blackpill_f401cc.overlay
│   ├── src
│   │   └── main.c
│   ├── prj.conf
│   └── CMakeLists.txt
│
├── Host
│   └── dio_test.py
│
└── Robot_Framework
    ├── resources
    │   └── testbench_serial.py
    └── tests
        └── 01_dio.robot
```

---

## 5. Zephyr Devicetree Overlay

File:

```text
Zephyr_Firmware/boards/blackpill_f401cc.overlay
```

Purpose:

- Define PB0 as DIO output.
- Define PB1 as DIO input.
- Create aliases so the C code can access the pins easily.

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>  /* Include GPIO flag definitions */

/ {                                         /* Start the root devicetree node */

    aliases {                              /* Start the aliases section */
        dio-out = &dio_out_pin;            /* Create alias dio-out that points to PB0 output node */
        dio-in = &dio_in_pin;              /* Create alias dio-in that points to PB1 input node */
    };                                     /* End the aliases section */

    dio_outputs {                          /* Create a container node for output GPIO pins */
        compatible = "gpio-leds";          /* Use Zephyr gpio-leds binding for GPIO output-like nodes */

        dio_out_pin: dio_out_pin {         /* Create the DIO output node label */
            gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>; /* Select PB0 as active-high GPIO output */
            label = "DIO_OUT_PB0";         /* Add a readable name for the output pin */
        };                                 /* End the DIO output node */
    };                                     /* End the output GPIO container */

    dio_inputs {                           /* Create a container node for input GPIO pins */
        compatible = "gpio-keys";          /* Use Zephyr gpio-keys binding for GPIO input-like nodes */

        dio_in_pin: dio_in_pin {           /* Create the DIO input node label */
            gpios = <&gpiob 1 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>; /* Select PB1 as input with pull-down */
            label = "DIO_IN_PB1";          /* Add a readable name for the input pin */
        };                                 /* End the DIO input node */
    };                                     /* End the input GPIO container */
};                                         /* End the root devicetree node */
```

### 5.1 Why Use Aliases?

The overlay creates two aliases:

```text
dio-out -> PB0
dio-in  -> PB1
```

The C code can then use:

```c
#define DIO_OUT_NODE DT_ALIAS(dio_out)
#define DIO_IN_NODE  DT_ALIAS(dio_in)
```

Zephyr converts `dio-out` in devicetree to `dio_out` in C because hyphens are not valid in C macro names.

### 5.2 Why Use `gpio-leds` and `gpio-keys`?

Zephyr needs a known devicetree binding to understand the `gpios` property correctly.

For output pins, `gpio-leds` is a simple and valid binding.

For input pins, `gpio-keys` is a simple and valid binding.

This allows Zephyr to generate the correct GPIO information during build time.

---

## 6. Zephyr Firmware Code

File:

```text
Zephyr_Firmware/src/main.c
```

Purpose:

- Get PB0 and PB1 from devicetree.
- Configure PB0 as output.
- Configure PB1 as input.
- Register shell commands.
- Execute DIO operations when commands are received.

### 6.1 Important Includes

```c
#include <zephyr/kernel.h>          /* Include Zephyr kernel APIs */
#include <zephyr/device.h>          /* Include Zephyr device APIs */
#include <zephyr/drivers/gpio.h>    /* Include Zephyr GPIO driver APIs */
#include <zephyr/shell/shell.h>     /* Include Zephyr shell APIs */
#include <stdlib.h>                 /* Include strtol function for string to number conversion */
```

### 6.2 Get Pins from Devicetree

```c
#define DIO_OUT_NODE DT_ALIAS(dio_out)      /* Get the devicetree alias for the DIO output pin */
#define DIO_IN_NODE  DT_ALIAS(dio_in)       /* Get the devicetree alias for the DIO input pin */
```

These macros connect the C code to the overlay aliases.

### 6.3 Create GPIO Specifications

```c
static const struct gpio_dt_spec dio_out = GPIO_DT_SPEC_GET(DIO_OUT_NODE, gpios); /* Create GPIO spec for PB0 output */
static const struct gpio_dt_spec dio_in  = GPIO_DT_SPEC_GET(DIO_IN_NODE, gpios);  /* Create GPIO spec for PB1 input */
```

Each `gpio_dt_spec` contains:

```text
port  -> GPIO port, for example GPIOB
pin   -> pin number, for example 0 or 1
flags -> GPIO flags, for example active high or pull-down
```

### 6.4 DIO Initialization

```c
static int dio_init(void)                                      /* Create a function to initialize DIO pins */
{
    if (!gpio_is_ready_dt(&dio_out)) {                         /* Check if the output GPIO port is ready */
        return -ENODEV;                                        /* Return error if output GPIO port is not ready */
    }                                                          /* End output GPIO readiness check */

    if (!gpio_is_ready_dt(&dio_in)) {                          /* Check if the input GPIO port is ready */
        return -ENODEV;                                        /* Return error if input GPIO port is not ready */
    }                                                          /* End input GPIO readiness check */

    int ret = gpio_pin_configure_dt(&dio_out, GPIO_OUTPUT_INACTIVE); /* Configure PB0 as output and set it LOW */

    if (ret < 0) {                                             /* Check if output pin configuration failed */
        return ret;                                            /* Return the GPIO configuration error */
    }                                                          /* End output configuration check */

    ret = gpio_pin_configure_dt(&dio_in, GPIO_INPUT);          /* Configure PB1 as input */

    if (ret < 0) {                                             /* Check if input pin configuration failed */
        return ret;                                            /* Return the GPIO configuration error */
    }                                                          /* End input configuration check */

    return 0;                                                  /* Return success if all DIO initialization steps passed */
}
```

The function returns:

```text
0        -> success
negative -> error
```

### 6.5 Shell Command: `tb_ping`

```c
static int cmd_tb_ping(const struct shell *sh, size_t argc, char **argv) /* Create shell command handler for tb_ping */
{
    ARG_UNUSED(argc);                                          /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                          /* Mark argv as unused to avoid compiler warning */

    shell_print(sh, "PONG");                                  /* Print PONG as the shell response */

    return 0;                                                  /* Return success */
}
```

Command:

```text
tb_ping
```

Expected response:

```text
PONG
```

This command confirms that Host-to-STM32 communication is working.

### 6.6 Shell Command: `tb_dio_write`

```c
static int cmd_tb_dio_write(const struct shell *sh, size_t argc, char **argv) /* Create shell command handler for DIO write */
{
    if (argc != 2) {                                           /* Check that the user passed exactly one argument */
        shell_error(sh, "Usage: tb_dio_write <0|1>");          /* Print correct command usage */
        return -EINVAL;                                        /* Return invalid argument error */
    }                                                          /* End argument count check */

    long value = strtol(argv[1], NULL, 10);                    /* Convert the input argument from string to number */

    if ((value != 0) && (value != 1)) {                        /* Check that the value is only 0 or 1 */
        shell_error(sh, "Error: value must be 0 or 1");        /* Print error message for invalid value */
        return -EINVAL;                                        /* Return invalid argument error */
    }                                                          /* End value validation check */

    int ret = gpio_pin_set_dt(&dio_out, (int)value);           /* Set PB0 output pin to the requested value */

    if (ret < 0) {                                             /* Check if GPIO write operation failed */
        shell_error(sh, "Error: failed to write DIO output");  /* Print GPIO write failure message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End GPIO write check */

    shell_print(sh, "DIO_WRITE OK VALUE %ld", value);          /* Print successful write response */

    return 0;                                                  /* Return success */
}
```

Commands:

```text
tb_dio_write 1
tb_dio_write 0
```

Expected responses:

```text
DIO_WRITE OK VALUE 1
DIO_WRITE OK VALUE 0
```

### 6.7 Shell Command: `tb_dio_read`

```c
static int cmd_tb_dio_read(const struct shell *sh, size_t argc, char **argv) /* Create shell command handler for DIO read */
{
    ARG_UNUSED(argc);                                          /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                          /* Mark argv as unused to avoid compiler warning */

    int value = gpio_pin_get_dt(&dio_in);                      /* Read PB1 input pin value */

    if (value < 0) {                                           /* Check if GPIO read operation failed */
        shell_error(sh, "Error: failed to read DIO input");    /* Print GPIO read failure message */
        return value;                                          /* Return the GPIO driver error */
    }                                                          /* End GPIO read check */

    shell_print(sh, "DIO_READ VALUE %d", value);               /* Print the input pin value */

    return 0;                                                  /* Return success */
}
```

Command:

```text
tb_dio_read
```

Expected responses:

```text
DIO_READ VALUE 1
DIO_READ VALUE 0
```

### 6.8 Shell Command: `tb_dio_test`

```c
static int cmd_tb_dio_test(const struct shell *sh, size_t argc, char **argv) /* Create shell command handler for automatic DIO loopback test */
{
    ARG_UNUSED(argc);                                          /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                          /* Mark argv as unused to avoid compiler warning */

    int ret = gpio_pin_set_dt(&dio_out, 1);                    /* Set PB0 output pin HIGH */

    if (ret < 0) {                                             /* Check if setting PB0 HIGH failed */
        shell_error(sh, "Error: failed to set output HIGH");   /* Print error message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End HIGH write check */

    k_sleep(K_MSEC(10));                                       /* Wait 10 ms to allow PB1 input to become stable */

    int high_value = gpio_pin_get_dt(&dio_in);                 /* Read PB1 input after setting PB0 HIGH */

    ret = gpio_pin_set_dt(&dio_out, 0);                        /* Set PB0 output pin LOW */

    if (ret < 0) {                                             /* Check if setting PB0 LOW failed */
        shell_error(sh, "Error: failed to set output LOW");    /* Print error message */
        return ret;                                            /* Return the GPIO driver error */
    }                                                          /* End LOW write check */

    k_sleep(K_MSEC(10));                                       /* Wait 10 ms to allow PB1 input to become stable */

    int low_value = gpio_pin_get_dt(&dio_in);                  /* Read PB1 input after setting PB0 LOW */

    if ((high_value == 1) && (low_value == 0)) {                /* Check if loopback result is correct */
        shell_print(sh, "DIO_TEST PASS HIGH %d LOW %d", high_value, low_value); /* Print PASS result */
        return 0;                                              /* Return success */
    }                                                          /* End PASS condition check */

    shell_error(sh, "DIO_TEST FAIL HIGH %d LOW %d", high_value, low_value); /* Print FAIL result */

    return -EIO;                                               /* Return input/output error */
}
```

Command:

```text
tb_dio_test
```

Expected response:

```text
DIO_TEST PASS HIGH 1 LOW 0
```

### 6.9 Register Shell Commands

```c
SHELL_CMD_REGISTER(tb_ping, NULL, "Test Bench ping command", cmd_tb_ping);           /* Register tb_ping shell command */
SHELL_CMD_REGISTER(tb_dio_write, NULL, "Write DIO output pin", cmd_tb_dio_write);    /* Register tb_dio_write shell command */
SHELL_CMD_REGISTER(tb_dio_read, NULL, "Read DIO input pin", cmd_tb_dio_read);        /* Register tb_dio_read shell command */
SHELL_CMD_REGISTER(tb_dio_test, NULL, "Run DIO loopback test", cmd_tb_dio_test);     /* Register tb_dio_test shell command */
```

These macros tell Zephyr Shell:

```text
If the user types tb_ping, call cmd_tb_ping().
If the user types tb_dio_write, call cmd_tb_dio_write().
If the user types tb_dio_read, call cmd_tb_dio_read().
If the user types tb_dio_test, call cmd_tb_dio_test().
```

### 6.10 Main Function

```c
int main(void)                                               /* Main application entry point */
{
    int ret = dio_init();                                    /* Initialize DIO output and input pins */

    if (ret < 0) {                                           /* Check if DIO initialization failed */
        printk("DIO initialization failed: %d\n", ret);      /* Print DIO initialization error */
        return ret;                                          /* Stop main with error code */
    }                                                        /* End DIO initialization check */

    printk("Test Bench Firmware Started\n");                 /* Print firmware startup message */

    return 0;                                                /* Return success from main */
}
```

Even after `main()` returns, Zephyr Shell continues running because it runs as a Zephyr service/thread.

---

## 7. Zephyr Configuration

File:

```text
Zephyr_Firmware/prj.conf
```

Required options:

```conf
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_SHELL=y
CONFIG_SHELL_BACKEND_SERIAL=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
```

Explanation:

| Option | Purpose |
|---|---|
| `CONFIG_GPIO=y` | Enable GPIO driver support |
| `CONFIG_SERIAL=y` | Enable serial driver support |
| `CONFIG_SHELL=y` | Enable Zephyr Shell |
| `CONFIG_SHELL_BACKEND_SERIAL=y` | Run shell over serial UART |
| `CONFIG_UART_CONSOLE=y` | Use UART as console backend |
| `CONFIG_PRINTK=y` | Enable `printk()` output |

---

## 8. Build and Flash

From the firmware folder:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Zephyr_Firmware
west build -b blackpill_f401cc -p always .
west flash
```

After flashing, reset the board.

Reset is important because the new firmware must start running before testing.

---

## 9. Manual Test Using Picocom

Open the UART shell:

```bash
picocom -b 115200 /dev/ttyUSB0
```

Test communication:

```text
tb_ping
```

Expected response:

```text
PONG
```

Test DIO loopback:

```text
tb_dio_test
```

Expected response:

```text
DIO_TEST PASS HIGH 1 LOW 0
```

Test write and read manually:

```text
tb_dio_write 1
tb_dio_read
```

Expected response:

```text
DIO_WRITE OK VALUE 1
DIO_READ VALUE 1
```

Then test LOW:

```text
tb_dio_write 0
tb_dio_read
```

Expected response:

```text
DIO_WRITE OK VALUE 0
DIO_READ VALUE 0
```

Exit picocom:

```text
Ctrl + A
Ctrl + X
```

---

## 10. Python Host Test

File:

```text
Host/dio_test.py
```

Purpose:

- Open `/dev/ttyUSB0`.
- Send `tb_ping`.
- Check for `PONG`.
- Send `tb_dio_test`.
- Check for `DIO_TEST PASS HIGH 1 LOW 0`.
- Print final PASS or FAIL.

Run:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Host
python3 dio_test.py
```

Expected output:

```text
Command: tb_ping
Response:
tb_ping
PONG
uart:~$

Command: tb_dio_test
Response:
tb_dio_test
DIO_TEST PASS HIGH 1 LOW 0
uart:~$

DIO Test: PASS
```

---

## 11. Robot Framework Python Library

File:

```text
Robot_Framework/resources/testbench_serial.py
```

Purpose:

- Open UART connection.
- Send Zephyr Shell commands.
- Read Zephyr Shell responses.
- Provide keywords to Robot Framework.

Important Robot keywords provided by this Python library:

| Python Method | Robot Keyword | Purpose |
|---|---|---|
| `open_connection()` | `Open Connection` | Open UART port |
| `close_connection()` | `Close Connection` | Close UART port |
| `send_shell_command()` | `Send Shell Command` | Send command to Zephyr Shell |
| `response_should_contain()` | `Response Should Contain` | Validate response text |

Important scope setting:

```python
ROBOT_LIBRARY_SCOPE = "SUITE"
```

This tells Robot Framework to use one Python object for the whole test suite.

This is important because the UART connection is stored in:

```python
self.ser
```

Without suite scope, Robot may create a new object and `self.ser` may become `None`.

---

## 12. Robot Framework Test Suite

File:

```text
Robot_Framework/tests/01_dio.robot
```

Content:

```robot
*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB0    115200
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

DIO Loopback Test
    ${response}=    Send Shell Command    tb_dio_test
    Response Should Contain    ${response}    DIO_TEST PASS HIGH 1 LOW 0

DIO Write High And Read Test
    ${write_response}=    Send Shell Command    tb_dio_write 1
    Response Should Contain    ${write_response}    DIO_WRITE OK VALUE 1
    ${read_response}=    Send Shell Command    tb_dio_read
    Response Should Contain    ${read_response}    DIO_READ VALUE 1

DIO Write Low And Read Test
    ${write_response}=    Send Shell Command    tb_dio_write 0
    Response Should Contain    ${write_response}    DIO_WRITE OK VALUE 0
    ${read_response}=    Send Shell Command    tb_dio_read
    Response Should Contain    ${read_response}    DIO_READ VALUE 0
```

---

## 13. Run Robot Framework Tests

Before running Robot tests, make sure picocom is closed.

If needed, release the UART port:

```bash
sudo fuser -k /dev/ttyUSB0
```

Run the tests:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Robot_Framework
robot tests/01_dio.robot
```

Expected output:

```text
==============================================================================
01 Dio
==============================================================================
Communication Ping Test                                               | PASS |
------------------------------------------------------------------------------
DIO Loopback Test                                                     | PASS |
------------------------------------------------------------------------------
DIO Write High And Read Test                                          | PASS |
------------------------------------------------------------------------------
DIO Write Low And Read Test                                           | PASS |
------------------------------------------------------------------------------
01 Dio                                                                | PASS |
4 tests, 4 passed, 0 failed
==============================================================================
Output:  output.xml
Log:     log.html
Report:  report.html
```

---

## 14. Robot Framework Output Files

Robot Framework generates three main files:

| File | Purpose |
|---|---|
| `output.xml` | Raw test execution data |
| `log.html` | Detailed execution log |
| `report.html` | Summary test report |

Open the detailed log:

```bash
firefox log.html
```

Open the summary report:

```bash
firefox report.html
```

---

## 15. Troubleshooting

### 15.1 Robot Test Returns Empty Response

Possible causes:

- Board was not reset after flashing.
- Wrong serial port.
- Picocom is still using the port.
- UART connection is incorrect.

Fix:

```bash
sudo fuser -k /dev/ttyUSB0
```

Then reset the board and run Robot again.

### 15.2 Picocom Works but Python Fails

Possible cause:

- Picocom and Python are trying to use the same serial port.

Fix:

Exit picocom:

```text
Ctrl + A
Ctrl + X
```

Then run Python again.

### 15.3 Build Succeeds but New Commands Do Not Work

Possible cause:

- The board was not reset after flashing.

Fix:

Press the reset button on the STM32 board after flashing.

### 15.4 DIO Test Fails

Possible causes:

- PB0 is not connected to PB1.
- Wrong pin mapping in overlay.
- Old firmware is still running.

Check connection:

```text
PB0 ---- PB1
```

Then rebuild, flash, reset, and test again.

---

## 16. Final DIO Phase Status

The DIO phase is considered complete when all of the following pass:

```text
Manual Shell Test: PASS
Python Host Test: PASS
Robot Framework Test: PASS
```

Final Robot result:

```text
4 tests, 4 passed, 0 failed
```

This confirms that the DIO test bench is working correctly.
