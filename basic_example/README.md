# Basic Shell Example (LED Toggle)

This is a simple example demonstrating how to use the Zephyr Shell to control hardware from a Host PC.

## The Flow
1. The STM32 runs Zephyr and exposes a shell interface over its UART port.
2. You register a custom C function in Zephyr as a shell command.
3. A Python script on the PC connects to the UART and "types" the command.
4. The STM32 parses the command, executes the C function (which toggles the LED), and prints a text response back to the PC.

## 1. Connecting the STM32 to the PC
To communicate with the shell, connect your STM32F401CC Blackpill to your PC using a USB-to-TTL (UART) adapter:
* **Adapter RX** -> **Blackpill PA9** (USART1 TX)
* **Adapter TX** -> **Blackpill PA10** (USART1 RX)
* **Adapter GND** -> **Blackpill GND**

*Note: The default baud rate is 115200.*

## 2. Registering a Shell Command (in C)
In `main.c`, you create a custom command by doing two things:

1. **Write the Handler:** Create a C function that performs the hardware action.
   ```c
   static int cmd_led_toggle(const struct shell *sh, size_t argc, char **argv) {
       gpio_pin_toggle_dt(&led);
       shell_print(sh, "LED Toggled!");
       return 0; // Return 0 on success
   }
   ```
2. **Register It:** Use the Zephyr macro to link your function to a text command (e.g., `blink_led`).
   ```c
   SHELL_CMD_REGISTER(blink_led, NULL, "Toggle Built In LED", cmd_led_toggle);
   ```

## 3. Executing from the PC (in Python)
You can test the command manually using a serial terminal like PuTTY. To execute it automatically via script, use `pyserial`:

```python
import serial
import time
import sys

try:
    # 1. Open the serial port (Use 'COM3' etc. on Windows, '/dev/ttyUSB0' on Linux)
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    time.sleep(1) # Let the connection settle
except Exception as e:
    print(f"Could not open port: {e}")
    sys.exit(1)

# 2. Send the shell command (must end with \n)
ser.write(b"blink_led\n")

# 3. Wait for the microcontroller to process it
time.sleep(0.1)

# 4. Read the response back
if ser.in_waiting > 0:
    response = ser.read_all().decode('utf-8', errors='ignore')
    print("Received:")
    print(response)

ser.close()
```
