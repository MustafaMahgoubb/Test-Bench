import serial  # Import pyserial to communicate with the STM32 UART shell
import time    # Import time to add delays while waiting for responses
import sys     # Import sys to exit with success or failure status
import re      # Import re to extract ADC values from the response text


PORT = "/dev/ttyUSB0"  # Define the serial port connected to the STM32 board
BAUDRATE = 115200      # Define the UART baud rate used by Zephyr shell
TIMEOUT = 0.1          # Define a short timeout for serial read operations


def open_serial_port():
    # Open the serial port in exclusive mode to prevent other tools from using it
    ser = serial.Serial(
        port=PORT,          # Select the UART port path
        baudrate=BAUDRATE,  # Set the UART baud rate
        timeout=TIMEOUT,    # Set the read timeout
        exclusive=True      # Lock the port on Linux
    )

    # Wait for the UART connection to become stable
    time.sleep(1.0)

    # Return the opened serial object
    return ser


def read_response(ser, read_time):
    # Create an empty string to store the full response
    response = ""

    # Save the current time before starting the read loop
    start_time = time.time()

    # Keep reading until the read time expires
    while time.time() - start_time < read_time:
        # Check if there are bytes waiting in the UART receive buffer
        if ser.in_waiting > 0:
            # Read all available bytes from the UART buffer
            data = ser.read(ser.in_waiting)

            # Convert received bytes to text
            text = data.decode("utf-8", errors="ignore")

            # Add the received text to the full response
            response += text

        # Wait a short time before checking again
        time.sleep(0.05)

    # Return the full response text
    return response


def send_shell_command(ser, command, read_time=2.0):
    # Clear old received data before sending a new command
    ser.reset_input_buffer()

    # Clear old transmitted data before sending a new command
    ser.reset_output_buffer()

    # Create the command line with CRLF because Zephyr shell expects Enter
    command_line = f"{command}\r\n"

    # Send the command to the STM32 board
    ser.write(command_line.encode("utf-8"))

    # Force the command to be sent immediately
    ser.flush()

    # Wait a little before reading the response
    time.sleep(0.3)

    # Read the response from the STM32 board
    response = read_response(ser, read_time)

    # Print the command that was sent
    print(f"Command: {command}")

    # Print the full response
    print("Response:")
    print(response)

    # Return the response to the caller
    return response


def validate_adc_response(response):
    # Search for ADC response format: ADC_READ RAW <number> MV <number>
    match = re.search(r"ADC_READ RAW\s+(\d+)\s+MV\s+(\d+)", response)

    # Check if the expected ADC format was found
    if match is None:
        # Print an error if the response format is invalid
        print("ADC response format is invalid")

        # Return failure
        return False

    # Extract the RAW ADC value from the response
    raw_value = int(match.group(1))

    # Extract the millivolt value from the response
    mv_value = int(match.group(2))

    # Print the extracted ADC values
    print(f"Extracted RAW: {raw_value}")
    print(f"Extracted MV: {mv_value}")

    # Check that the raw ADC value is inside the 12-bit range
    raw_ok = 0 <= raw_value <= 4095

    # Check that the millivolt value is inside the expected voltage range
    mv_ok = 0 <= mv_value <= 3300

    # Return True only if both checks passed
    return raw_ok and mv_ok


try:
    # Open the UART serial port
    ser = open_serial_port()

except serial.SerialException as error:
    # Print the error if the serial port cannot be opened
    print(f"Could not open serial port {PORT}: {error}")

    # Exit with failure because communication cannot continue
    sys.exit(1)


# Send ping command to check communication
ping_response = send_shell_command(ser, "tb_ping")

# Send ADC read command to get potentiometer value
adc_response = send_shell_command(ser, "tb_adc_read")

# Close the UART serial port
ser.close()


# Check if ping response contains PONG
ping_ok = "PONG" in ping_response

# Check if ADC response has valid RAW and MV values
adc_ok = validate_adc_response(adc_response)


# Check final test result
if ping_ok and adc_ok:
    # Print final PASS result
    print("ADC Test: PASS")

    # Exit with success status
    sys.exit(0)

else:
    # Print final FAIL result
    print("ADC Test: FAIL")

    # Exit with failure status
    sys.exit(1)

    