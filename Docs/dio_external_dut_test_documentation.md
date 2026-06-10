# DIO External DUT Test Documentation

## 1. Overview

This document describes the external DIO test used in the Test Bench project.

The test validates digital input/output communication between two STM32F401 Blackpill boards:

- **Board 1:** Test Bench Controller
- **Board 2:** DUT Firmware / DIO Mirror Responder

Robot Framework communicates with Board 1 over UART. Board 1 drives a digital output pin, Board 2 reads it, mirrors the value to another pin, and Board 1 reads the mirrored value back.

---

## 2. Test Architecture

```text
PC / Robot Framework
        |
        | UART
        v
Board 1: Test Bench Controller
        |
        | DIO signals
        v
Board 2: DUT / DIO Mirror Responder
```

---

## 3. Hardware Used

- STM32F401 Blackpill board as Test Bench Controller
- STM32F401 Blackpill board as DUT / DIO Mirror
- USB-to-TTL converter for Board 1 shell communication
- Jumper wires

---

## 4. DIO Wiring

```text
Board 1 / Test Bench              Board 2 / DUT
--------------------              -------------
GND           ------------------  GND
PB8 / DIO_OUT -----------------> PB8 / DUT_DIO_IN
PB9 / DIO_IN  <----------------- PB9 / DUT_DIO_OUT
```

Important notes:

- Both boards must share a common GND.
- Board 1 PB8 is the output under test.
- Board 1 PB9 is the input that reads the DUT response.
- Board 2 PB8 reads the signal from Board 1.
- Board 2 PB9 mirrors the received value back to Board 1.

---

## 5. Board Roles

### Board 1: Test Bench Controller

Board 1 runs the main Test Bench firmware.

Main responsibilities:

- Receive shell commands from the PC
- Drive PB8 as digital output
- Read PB9 as digital input
- Validate HIGH and LOW states
- Print PASS or FAIL result

### Board 2: DUT Firmware

Board 2 runs the DUT firmware.

Main responsibilities:

- Configure PB8 as input
- Configure PB9 as output
- Continuously read PB8
- Write the same value to PB9

This creates a simple DIO mirror.

---

## 6. DIO Test Logic

The DIO external test works as follows:

```text
1. Board 1 drives PB8 HIGH.
2. Board 2 reads HIGH on PB8.
3. Board 2 mirrors HIGH to PB9.
4. Board 1 reads HIGH on PB9.

5. Board 1 drives PB8 LOW.
6. Board 2 reads LOW on PB8.
7. Board 2 mirrors LOW to PB9.
8. Board 1 reads LOW on PB9.
```

PASS condition:

```text
HIGH read == 1
LOW read  == 0
```

---

## 7. Board 1 Shell Commands

### Full DIO Test

```text
tb_dio_test
```

Expected output:

```text
DIO_TEST PASS HIGH 1 LOW 0
```

### Write HIGH

```text
tb_dio_write 1
```

Expected output:

```text
DIO_WRITE OK VALUE 1
```

### Read Input

```text
tb_dio_read
```

Expected output after writing HIGH:

```text
DIO_READ VALUE 1
```

### Write LOW

```text
tb_dio_write 0
```

Expected output:

```text
DIO_WRITE OK VALUE 0
```

### Read Input

Expected output after writing LOW:

```text
DIO_READ VALUE 0
```

---

## 8. DUT Firmware Behavior

The DUT firmware continuously executes the following logic:

```text
read PB8
write same value to PB9
```

This allows Board 1 to validate both output and input paths using an external board instead of a local loopback jumper.

---

## 9. Robot Framework Test

Robot Framework test file:

```text
Robot_Framework/tests/05_dio_external.robot
```

Main test cases:

```text
Communication Ping Test
DIO External DUT Mirror Test
DIO External Write High And Read Test
DIO External Write Low And Read Test
```

Expected Robot output:

```text
DIO External DUT Result:
  HIGH Read = 1
  LOW Read  = 0

4 tests, 4 passed, 0 failed
```

---

## 10. Troubleshooting

### DIO_TEST returns HIGH 0 LOW 0

Possible causes:

- Board 1 PB8 is not connected to Board 2 PB8
- Board 2 firmware is not running
- Board 2 PB9 is not connected to Board 1 PB9
- GND is not shared

### DIO_TEST returns HIGH 1 LOW 1

Possible causes:

- Board 1 PB9 is stuck HIGH
- Board 2 PB9 is stuck HIGH
- Incorrect wiring
- Pull-up or external short to 3V3

### Manual write works but Robot fails

Possible causes:

- Wrong serial port
- Serial port already opened by picocom
- Robot test file indentation issue
- Robot library keyword issue

---

## 11. Current Verified Result

Manual shell result:

```text
DIO_TEST PASS HIGH 1 LOW 0
DIO_WRITE OK VALUE 1
DIO_READ VALUE 1
```

This confirms that the external DIO mirror test is working correctly between the Test Bench board and the DUT board.

---

## 12. Notes

The external DIO test is more realistic than a local loopback because the signal leaves Board 1, is processed by Board 2, and returns to Board 1.

This validates:

- Board 1 digital output
- Board 2 digital input
- Board 2 digital output
- Board 1 digital input
- Shared ground and wiring integrity
