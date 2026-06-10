# SPI External DUT Test Documentation

## 1. Overview

This document describes the external SPI test used in the Test Bench project.

The test validates SPI communication between two STM32F401 Blackpill boards:

- **Board 1:** Test Bench Controller / SPI Master
- **Board 2:** DUT Firmware / SPI Slave Responder

Robot Framework communicates with Board 1 over UART. Board 1 sends an SPI command to Board 2, receives a response, and prints the result through the Zephyr shell.

---

## 2. Test Architecture

```text
PC / Robot Framework
        |
        | UART
        v
Board 1: Test Bench Controller
        |
        | SPI Master
        v
Board 2: DUT / SPI Slave Responder
```

---

## 3. Hardware Used

- STM32F401 Blackpill board as Test Bench Controller
- STM32F401 Blackpill board as DUT / SPI Slave
- USB-to-TTL converter for Board 1 shell communication
- Optional USB-to-TTL converter for Board 2 debug logs
- Jumper wires

---

## 4. SPI Wiring

```text
Board 1 / Test Bench Master        Board 2 / DUT Slave
---------------------------        --------------------
GND                                GND
PA4 / CS      ------------------>  PA4 / NSS
PA5 / SCK     ------------------>  PA5 / SCK
PA7 / MOSI    ------------------>  PA7 / MOSI
PA6 / MISO    <------------------  PA6 / MISO
```

Important notes:

- Both boards must share a common GND.
- MOSI connects to MOSI.
- MISO connects to MISO.
- CS/NSS is active low.
- The previous loopback wire `PA7 ---- PA6` must be removed during the external DUT test.

---

## 5. Board Roles

### Board 1: Test Bench Controller

Board 1 runs the main Test Bench firmware.

Main responsibilities:

- Receive shell commands from the PC
- Control SPI as master
- Manually control CS on PA4
- Send command byte `0xA5`
- Receive response byte from the DUT
- Print PASS or FAIL result

### Board 2: DUT Firmware

Board 2 runs the DUT firmware.

Main responsibilities:

- Configure SPI1 as slave
- Wait for SPI transfer from Board 1
- Receive command byte `0xA5`
- Respond with fixed byte `0x5A`
- Print debug message through UART if connected

---

## 6. SPI Protocol

The current SPI protocol is a simple fixed-response ping.

```text
Master TX: 0xA5
Slave RX : 0xA5

Slave TX : 0x5A
Master RX: 0x5A
```

Expected PASS condition:

```text
TX == 0xA5
RX == 0x5A
```

---

## 7. Board 1 Shell Command

### Command

```text
tb_spi_slave_ping
```

### Expected Output

```text
SPI_SLAVE_PING PASS TX 0xA5 RX 0x5A
```

### Failure Example

```text
SPI_SLAVE_PING FAIL TX 0xA5 RX 0x00 EXPECTED 0x5A
```

---

## 8. DUT Debug Output

If Board 2 UART debug is connected, the expected output is:

```text
DUT Firmware Started
DUT SPI slave thread started
DUT SPI RX 0xA5 TX 0x5A
```

This confirms that the DUT received the SPI command and transmitted the expected response.

---

## 9. Important Implementation Notes

### Manual CS Control

The SPI external DUT test uses manual CS control on Board 1.

The CS pin is PA4 and is configured as GPIO active low.

```text
CS LOW  -> DUT selected
CS HIGH -> DUT deselected
```

The transfer sequence is:

```text
PA4 CS LOW
SPI transfer
PA4 CS HIGH
```

This avoids issues with automatic chip-select handling and makes the SPI transaction explicit.

### SPI Frequency

The SPI frequency was set to:

```text
500 kHz
```

A frequency of `100 kHz` was rejected by the STM32 SPI driver because it was below the supported minimum frequency.

---

## 10. Robot Framework Test

Robot Framework test file:

```text
Robot_Framework/tests/04_spi_external.robot
```

Main test cases:

```text
Communication Ping Test
SPI External DUT Slave Ping Test
SPI External DUT Slave Ping Repeat Test
```

Expected Robot output:

```text
SPI External DUT Result:
  TX = 0xA5
  RX = 0x5A

3 tests, 3 passed, 0 failed
```

---

## 11. Troubleshooting

### RX is 0x00

Possible causes:

- DUT board is not powered
- GND is not shared
- MISO wire is disconnected
- CS/NSS is not connected correctly
- DUT firmware is not running
- SPI slave is not selected

### RX is 0xFF

Possible causes:

- MISO line is floating or pulled high
- DUT MISO is not driving the line
- MISO wire is disconnected

### DUT shows timeout errors

Possible causes:

- Master is not sending SPI clock
- CS/NSS is not asserted
- SCK or MOSI wiring is wrong

### Robot test fails but manual shell command passes

Possible causes:

- Wrong serial port
- Another terminal is using the serial port
- Robot library file has indentation or keyword issues
- Board was not reset after flashing

---

## 12. Current Verified Result

Manual shell result:

```text
SPI_SLAVE_PING PASS TX 0xA5 RX 0x5A
```

Robot Framework result:

```text
04 Spi External | PASS |
3 tests, 3 passed, 0 failed
```

DUT debug result:

```text
DUT SPI RX 0xA5 TX 0x5A
```

The external SPI DUT test is verified successfully.
