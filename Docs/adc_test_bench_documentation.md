# ADC Test Bench Documentation

## 1. Overview

This document explains the ADC module in the Test Bench project.

The ADC test reads an analog voltage from a potentiometer connected to the STM32F401 Blackpill board. The firmware exposes a Zephyr Shell command called `tb_adc_read`. The host side can test this command manually using `picocom`, using a Python script, or automatically using Robot Framework.

The ADC module is written as an independent module, separate from DIO and separate from `main.c`. This keeps the project clean and makes it easy to add more modules later, such as SPI, PWM, and UART.

---

## 2. ADC Test Purpose

The ADC test checks that:

1. The board can read an analog voltage from the potentiometer.
2. The ADC raw value is inside the valid 12-bit range.
3. The converted voltage value is inside the valid voltage range.
4. Robot Framework can automatically validate the ADC result.

The expected ADC range is:

| Value Type | Minimum | Maximum |
|---|---:|---:|
| RAW ADC value | 0 | 4095 |
| Voltage | 0 mV | 3300 mV |

The ADC is configured as a 12-bit ADC.

```text
12-bit ADC range = 0 to 4095
Voltage range    = 0 mV to 3300 mV
```

The conversion equation is:

```text
millivolts = raw_value * 3300 / 4095
```

---

## 3. Hardware Used

| Item | Description |
|---|---|
| Board | STM32F401 Blackpill |
| Debugger | ST-Link |
| UART Adapter | USB-to-TTL adapter |
| Analog Input | Potentiometer |
| Firmware | Zephyr RTOS |
| Host Automation | Python + Robot Framework |

---

## 4. Pin Mapping

| Function | STM32 Pin | Description |
|---|---|---|
| ADC Input | PA1 | ADC1_IN1 connected to potentiometer middle pin |
| UART TX | PA9 | USART1 TX to USB-to-TTL RX |
| UART RX | PA10 | USART1 RX to USB-to-TTL TX |
| SWDIO | PA13 | ST-Link SWDIO |
| SWCLK | PA14 | ST-Link SWCLK |
| Power | 3V3 | Potentiometer side pin |
| Ground | GND | Potentiometer side pin |

Do not use PA13 and PA14 for normal testing because they are used for ST-Link flashing and debugging.

---

## 5. Potentiometer Wiring

Connect the potentiometer as follows:

```text
Potentiometer side pin   -> 3V3
Potentiometer middle pin -> PA1 / ADC1_IN1
Potentiometer side pin   -> GND
```

ASCII diagram:

```text
              STM32F401 Blackpill
             +--------------------+
             |                    |
             | PA1 / ADC1_IN1 <---+-------- Middle Pin
             |                    |          of Potentiometer
             | 3V3 --------------+-------- Pot Side Pin
             | GND --------------+-------- Pot Side Pin
             |                    |
             +--------------------+

                   Potentiometer

                         +--------+
              3V3 -------|        |
                         |        |------ PA1 / ADC1_IN1
              GND -------|        |
                         +--------+
```

When the potentiometer is rotated, the voltage on PA1 changes between 0V and 3.3V.

```text
Pot near GND  -> RAW near 0    -> Voltage near 0 mV
Pot in middle -> RAW near 2048 -> Voltage near 1650 mV
Pot near 3V3  -> RAW near 4095 -> Voltage near 3300 mV
```

---

## 6. Full System Flow

The ADC test flow starts from Robot Framework or manual terminal input and ends at the physical ADC pin.

```text
Robot Framework Test
        |
        | calls keyword: Send Shell Command
        v
Python Robot Library
        |
        | sends text: tb_adc_read\r\n
        v
/dev/ttyUSB0
        |
        v
USB-to-TTL Adapter
        |
        v
USART1 PA9 / PA10
        |
        v
Zephyr Shell
        |
        | finds command: tb_adc_read
        v
cmd_tb_adc_read()
        |
        v
ADC Service Module
        |
        | calls adc_read()
        v
ADC1 Peripheral
        |
        v
PA1 / ADC1_IN1
        |
        v
Potentiometer Voltage
        |
        v
ADC RAW value
        |
        v
Voltage Calculation in mV
        |
        v
Shell Response:
ADC_READ RAW <raw_value> MV <millivolts>
        |
        v
Python reads response
        |
        v
Robot validates range
        |
        v
PASS or FAIL
```

---

## 7. Project Structure

The ADC module is separated from `main.c`.

```text
Zephyr_Firmware
├── CMakeLists.txt
├── prj.conf
├── boards
│   └── blackpill_f401cc.overlay
└── src
    ├── main.c
    └── modules
        ├── dio
        │   ├── dio_service.c
        │   └── dio_service.h
        └── adc
            ├── adc_service.c
            └── adc_service.h
```

Robot Framework files:

```text
Robot_Framework
├── resources
│   └── testbench_serial.py
└── tests
    ├── 01_dio.robot
    └── 02_adc.robot
```

Host-side Python test:

```text
Host
└── adc_test.py
```

---

## 8. Zephyr Configuration

File:

```text
Zephyr_Firmware/prj.conf
```

Required configuration:

```conf
CONFIG_GPIO=y
CONFIG_ADC=y

CONFIG_SERIAL=y
CONFIG_SHELL=y
CONFIG_SHELL_BACKEND_SERIAL=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y
```

Explanation:

| Config | Purpose |
|---|---|
| `CONFIG_GPIO=y` | Enables GPIO support. Required because other modules such as DIO use GPIO. |
| `CONFIG_ADC=y` | Enables Zephyr ADC driver support. |
| `CONFIG_SERIAL=y` | Enables serial driver support. |
| `CONFIG_SHELL=y` | Enables Zephyr Shell. |
| `CONFIG_SHELL_BACKEND_SERIAL=y` | Runs the shell over UART serial. |
| `CONFIG_UART_CONSOLE=y` | Enables console over UART. |
| `CONFIG_PRINTK=y` | Enables `printk()` debug messages. |

---

## 9. Devicetree Overlay

File:

```text
Zephyr_Firmware/boards/blackpill_f401cc.overlay
```

ADC part:

```dts
&adc1 {                         /* Select ADC1 peripheral node */
    status = "okay";            /* Enable ADC1 peripheral */
};
```

The generated devicetree showed that the board already has ADC1 configured on PA1:

```text
adc1_in1_pa1
adc1: adc@40012000
pinctrl-0 = < &adc1_in1_pa1 >
```

This means:

```text
PA1 = ADC1_IN1
```

---

## 10. ADC Module Header

File:

```text
Zephyr_Firmware/src/modules/adc/adc_service.h
```

```c
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
```

Purpose of this file:

```text
adc_service.h exposes adc_service_init() to main.c.
main.c does not need to know ADC internal details.
```

---

## 11. ADC Module Source

File:

```text
Zephyr_Firmware/src/modules/adc/adc_service.c
```

Main responsibilities:

1. Get ADC1 device from devicetree.
2. Configure ADC channel 1.
3. Register `tb_adc_read` shell command.
4. Read ADC sample.
5. Convert raw value to millivolts.
6. Print the result to Zephyr Shell.

Important definitions:

```c
#define ADC_NODE DT_NODELABEL(adc1)      /* Get the devicetree node label for ADC1 */
#define ADC_CHANNEL_ID 1                 /* Select ADC1 channel 1, which maps to PA1 */
#define ADC_RESOLUTION 12                /* Use 12-bit ADC resolution */
#define ADC_VREF_MV 3300                 /* Use 3300 mV as the ADC reference voltage */
```

ADC device:

```c
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE); /* Get ADC1 device from devicetree */
```

ADC channel configuration:

```c
static const struct adc_channel_cfg adc_channel_cfg = {
    .gain = ADC_GAIN_1,                          /* Use gain 1, so the input voltage is not scaled */
    .reference = ADC_REF_INTERNAL,               /* Use the ADC internal reference setting */
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,    /* Use the default ADC acquisition time */
    .channel_id = ADC_CHANNEL_ID,                /* Select ADC channel 1 */
    .differential = 0                            /* Use single-ended mode, not differential mode */
};
```

ADC initialization:

```c
int adc_service_init(void)
{
    if (!device_is_ready(adc_dev)) {                     /* Check if ADC1 device is ready */
        return -ENODEV;                                  /* Return error if ADC1 is not ready */
    }                                                    /* End ADC readiness check */

    int ret = adc_channel_setup(adc_dev, &adc_channel_cfg); /* Configure ADC channel 1 */

    if (ret < 0) {                                       /* Check if ADC channel setup failed */
        return ret;                                      /* Return the ADC setup error */
    }                                                    /* End ADC setup check */

    return 0;                                            /* Return success */
}
```

ADC shell command handler:

```c
static int cmd_tb_adc_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);                                    /* Mark argc as unused to avoid compiler warning */
    ARG_UNUSED(argv);                                    /* Mark argv as unused to avoid compiler warning */

    int16_t sample = 0;                                  /* Create a variable to store the raw ADC sample */

    struct adc_sequence sequence = {                     /* Create ADC read sequence configuration */
        .channels = BIT(ADC_CHANNEL_ID),                 /* Select ADC channel 1 */
        .buffer = &sample,                               /* Store the ADC result in sample */
        .buffer_size = sizeof(sample),                   /* Set the buffer size */
        .resolution = ADC_RESOLUTION                     /* Set ADC resolution to 12 bits */
    };

    int ret = adc_read(adc_dev, &sequence);              /* Read one sample from ADC1 channel 1 */

    if (ret < 0) {                                       /* Check if ADC read failed */
        shell_error(sh, "ADC_READ ERROR %d", ret);       /* Print ADC read error */
        return ret;                                      /* Return the ADC driver error */
    }                                                    /* End ADC read error check */

    int32_t raw = sample;                                /* Convert raw ADC sample to 32-bit integer */

    int32_t millivolts = (raw * ADC_VREF_MV) / ((1 << ADC_RESOLUTION) - 1); /* Convert raw value to millivolts */

    shell_print(sh, "ADC_READ RAW %d MV %d", raw, millivolts); /* Print ADC raw value and voltage */

    return 0;                                            /* Return success */
}
```

Shell command registration:

```c
SHELL_CMD_REGISTER(tb_adc_read, NULL, "Read ADC potentiometer value", cmd_tb_adc_read); /* Register tb_adc_read shell command */
```

---

## 12. `main.c` Integration

File:

```text
Zephyr_Firmware/src/main.c
```

The ADC module is included in `main.c`:

```c
#include "modules/adc/adc_service.h"    /* Include the ADC service interface */
```

The ADC service is initialized from `main()`:

```c
ret = adc_service_init();                              /* Initialize the ADC service */

if (ret < 0) {                                         /* Check if ADC service initialization failed */
    printk("ADC service initialization failed: %d\n", ret); /* Print the ADC initialization error */
    return ret;                                        /* Stop the application with the error code */
}                                                      /* End ADC service initialization check */
```

This keeps `main.c` clean.

```text
main.c only initializes services.
adc_service.c contains all ADC details.
```

---

## 13. CMake Integration

File:

```text
Zephyr_Firmware/CMakeLists.txt
```

Add the ADC source file:

```cmake
target_sources(app PRIVATE                   # Add source files to the Zephyr application target
    src/main.c                               # Add the main application source file
    src/modules/dio/dio_service.c            # Add the DIO service source file
    src/modules/adc/adc_service.c            # Add the ADC service source file
)
```

Without this line, Zephyr will not compile `adc_service.c`, and the `tb_adc_read` command will not exist.

---

## 14. Build and Flash

From the firmware directory:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Zephyr_Firmware
west build -b blackpill_f401cc -p always .
west flash
```

After flashing, reset the board.

This project showed that reset after flash is important because the shell may not become ready until the board restarts cleanly.

---

## 15. Manual Test Using Picocom

Open the UART shell:

```bash
picocom -b 115200 /dev/ttyUSB0
```

Press reset on the board. You should see the shell prompt:

```text
uart:~$
```

Test communication:

```text
tb_ping
```

Expected output:

```text
PONG
```

Test ADC:

```text
tb_adc_read
```

Expected output example:

```text
ADC_READ RAW 2480 MV 1998
```

Rotate the potentiometer and run the command again:

```text
tb_adc_read
```

Expected behavior:

```text
ADC_READ RAW 1 MV 0
ADC_READ RAW 2480 MV 1998
ADC_READ RAW 4095 MV 3300
```

These values prove that the ADC input changes when the potentiometer changes.

---

## 16. Host Python ADC Test

File:

```text
Host/adc_test.py
```

Purpose:

1. Open `/dev/ttyUSB0` using PySerial.
2. Send `tb_ping`.
3. Send `tb_adc_read`.
4. Extract `RAW` and `MV` using regular expression.
5. Validate ranges.
6. Exit with status code `0` for PASS or `1` for FAIL.

Main validation logic:

```python
match = re.search(r"ADC_READ RAW\s+(\d+)\s+MV\s+(\d+)", response)
```

This extracts:

```text
RAW value
MV value
```

Valid ranges:

```python
raw_ok = 0 <= raw_value <= 4095
mv_ok = 0 <= mv_value <= 3300
```

Run the test:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Host
python3 adc_test.py
```

Expected output:

```text
Command: tb_ping
Response:
tb_ping
PONG
uart:~$

Command: tb_adc_read
Response:
tb_adc_read
ADC_READ RAW 2480 MV 1998
uart:~$

Extracted RAW: 2480
Extracted MV: 1998
ADC Test: PASS
```

---

## 17. Robot Framework ADC Test

File:

```text
Robot_Framework/tests/02_adc.robot
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

ADC Potentiometer Read Test
    ${response}=    Send Shell Command    tb_adc_read
    Response Should Contain    ${response}    ADC_READ RAW
    ADC Response Should Be Valid    ${response}
```

Run the test:

```bash
cd ~/zephyrproject/iti_applications/Test-Bench/Robot_Framework/tests
robot 02_adc.robot
```

Expected PASS example:

```text
==============================================================================
02 Adc
==============================================================================
Communication Ping Test                                               | PASS |
------------------------------------------------------------------------------
ADC Potentiometer Read Test                                           ..
---------------- ADC Reading ----------------
RAW Value : 2480
Voltage   : 1998 mV
---------------------------------------------
ADC Potentiometer Read Test                                           | PASS |
------------------------------------------------------------------------------
02 Adc                                                                | PASS |
2 tests, 2 passed, 0 failed
==============================================================================
```

---

## 18. Robot ADC Validation Keyword

File:

```text
Robot_Framework/resources/testbench_serial.py
```

Required imports:

```python
import re  # Import re to extract values from shell response text
from robot.api import logger  # Import Robot logger to print messages in Robot console and log
```

ADC validation keyword:

```python
    def adc_response_should_be_valid(self, response):
        # Search for ADC response format: ADC_READ RAW <number> MV <number>
        match = re.search(r"ADC_READ RAW\s+(\d+)\s+MV\s+(\d+)", response)

        # Check if the ADC response format was not found
        if match is None:
            # Raise an error so Robot Framework marks the test as failed
            raise AssertionError(f"Invalid ADC response format:\n{response}")

        # Extract the raw ADC value from the response
        raw_value = int(match.group(1))

        # Extract the millivolt value from the response
        mv_value = int(match.group(2))

        # Print ADC values in Robot terminal output in a clean block
        logger.console("\n---------------- ADC Reading ----------------")
        logger.console(f"RAW Value : {raw_value}")
        logger.console(f"Voltage   : {mv_value} mV")
        logger.console("---------------------------------------------")

        # Save ADC values in Robot log.html
        logger.info(f"ADC RAW value = {raw_value}")

        # Save ADC voltage in Robot log.html
        logger.info(f"ADC voltage = {mv_value} mV")

        # Check if the raw ADC value is outside the 12-bit range
        if raw_value < 0 or raw_value > 4095:
            # Raise an error if the raw value is invalid
            raise AssertionError(f"ADC raw value out of range: {raw_value}")

        # Check if the millivolt value is outside the expected voltage range
        if mv_value < 0 or mv_value > 3300:
            # Raise an error if the millivolt value is invalid
            raise AssertionError(f"ADC millivolt value out of range: {mv_value}")

        # Check if the ADC value is too close to 0V
        if raw_value <= 50:
            # Raise an error because the ADC input may be connected to GND or stuck LOW
            raise AssertionError(f"ADC value is too low or stuck near GND: RAW={raw_value}, MV={mv_value}")

        # Check if the ADC value is too close to 3.3V
        if raw_value >= 4045:
            # Raise an error because the ADC input may be connected to 3V3 or stuck HIGH
            raise AssertionError(f"ADC value is too high or stuck near 3V3: RAW={raw_value}, MV={mv_value}")
```

This keyword fails the test if:

1. The response format is invalid.
2. RAW is outside `0..4095`.
3. MV is outside `0..3300`.
4. RAW is too close to `0`, which may mean the input is stuck near GND.
5. RAW is too close to `4095`, which may mean the input is stuck near 3.3V.

---

## 19. PASS and FAIL Examples

### PASS Example

```text
ADC Reading:
  RAW     = 2480
  Voltage = 1998 mV

ADC Potentiometer Read Test | PASS |
```

Reason:

```text
RAW is between 50 and 4045.
The ADC input is not stuck near GND or 3.3V.
```

### FAIL Near GND

```text
ADC Reading:
  RAW     = 1
  Voltage = 0 mV

ADC Potentiometer Read Test | FAIL |
ADC value is too low or stuck near GND: RAW=1, MV=0
```

Reason:

```text
RAW is too close to 0.
The potentiometer may be rotated fully toward GND.
```

### FAIL Near 3.3V

```text
ADC Reading:
  RAW     = 4095
  Voltage = 3300 mV

ADC Potentiometer Read Test | FAIL |
ADC value is too high or stuck near 3V3: RAW=4095, MV=3300
```

Reason:

```text
RAW is too close to 4095.
The potentiometer may be rotated fully toward 3.3V.
```

---

## 20. Important Notes

### Robot Summary Is Normal

Robot always prints a summary at the end:

```text
2 tests, 2 passed, 0 failed
```

or:

```text
2 tests, 1 passed, 1 failed
```

This is normal and should not be removed. It is the final test result.

### Robot Dots Are Normal

Robot may print dots like this:

```text
ADC Potentiometer Read Test                                           ..
```

These dots represent running keywords. They are normal.

### Picocom and Robot Cannot Use the Same Port Together

Close `picocom` before running Python or Robot tests.

Exit `picocom` using:

```text
Ctrl + A
Ctrl + X
```

Check port usage:

```bash
lsof /dev/ttyUSB0
```

Kill a process using the port if needed:

```bash
sudo fuser -k /dev/ttyUSB0
```

---

## 21. Troubleshooting

| Problem | Possible Cause | Solution |
|---|---|---|
| No output in picocom | Board not reset after flash | Press reset button |
| No shell prompt | Firmware not running or wrong UART | Check flash and UART wiring |
| `tb_adc_read` command not found | `adc_service.c` not added to CMake | Add it to `target_sources()` |
| ADC always reads 0 | PA1 connected to GND or wrong wiring | Check potentiometer wiring |
| ADC always reads 4095 | PA1 connected to 3V3 or wrong wiring | Check potentiometer wiring |
| Robot fails with port error | Picocom still open | Close picocom |
| Robot PASS but value not visible | Robot only shows PASS/FAIL by default | Use `logger.console()` |
| Build fails with ADC error | ADC config missing | Check `CONFIG_ADC=y` and overlay |

---

## 22. Success Criteria

The ADC module is considered working when all of the following are true:

1. `tb_adc_read` appears in Zephyr Shell help.
2. `tb_adc_read` returns this format:

```text
ADC_READ RAW <raw_value> MV <millivolts>
```

3. RAW changes when the potentiometer is rotated.
4. RAW stays between `0` and `4095`.
5. MV stays between `0` and `3300`.
6. `02_adc.robot` passes when the potentiometer is not at the extreme low or high ends.

---

## 23. Final ADC Test Flow Summary

```text
Potentiometer changes voltage
        |
        v
PA1 receives analog voltage
        |
        v
ADC1 converts analog voltage to RAW digital value
        |
        v
Firmware converts RAW to millivolts
        |
        v
Zephyr Shell prints ADC_READ response
        |
        v
Python reads the response from UART
        |
        v
Robot Framework validates the result
        |
        v
PASS or FAIL
```

---

## 24. Current Project Status

Completed modules:

| Module | Status | Shell Command | Robot Test |
|---|---|---|---|
| Communication | Done | `tb_ping` | `Communication Ping Test` |
| DIO | Done | `tb_dio_test`, `tb_dio_write`, `tb_dio_read` | `01_dio.robot` |
| ADC | Done | `tb_adc_read` | `02_adc.robot` |

Next possible modules:

1. SPI module
2. PWM module
3. UART module
4. Combined full test suite

