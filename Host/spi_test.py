import serial  # Import pyserial to communicate with the STM32 UART shell
import time    # Import time to add delays while waiting for responses
import sys     # Import sys to exit with success or failure status


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

            # Convert received bytes to text and ignore invalid characters
            text = data.decode("utf-8", errors="ignore")

            # Add received text to the full response
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


try:
    # Open UART serial port
    ser = open_serial_port()

except serial.SerialException as error:
    # Print error if serial port cannot be opened
    print(f"Could not open serial port {PORT}: {error}")

    # Exit with failure
    sys.exit(1)


# Send ping command to verify communication
ping_response = send_shell_command(ser, "tb_ping")

# Run default SPI loopback test
spi_test_response = send_shell_command(ser, "tb_spi_test")

# Run SPI transfer with different test bytes
spi_transfer_55_response = send_shell_command(ser, "tb_spi_transfer 0x55")
spi_transfer_a5_response = send_shell_command(ser, "tb_spi_transfer 0xA5")
spi_transfer_ff_response = send_shell_command(ser, "tb_spi_transfer 0xFF")

# Close UART serial port
ser.close()


# Validate ping response
ping_ok = "PONG" in ping_response

# Validate default SPI loopback response
spi_test_ok = "SPI_TEST PASS TX 0xA5 RX 0xA5" in spi_test_response

# Validate transfer responses
spi_transfer_55_ok = "SPI_TRANSFER TX 0x55 RX 0x55" in spi_transfer_55_response
spi_transfer_a5_ok = "SPI_TRANSFER TX 0xA5 RX 0xA5" in spi_transfer_a5_response
spi_transfer_ff_ok = "SPI_TRANSFER TX 0xFF RX 0xFF" in spi_transfer_ff_response


# Check final test result
if ping_ok and spi_test_ok and spi_transfer_55_ok and spi_transfer_a5_ok and spi_transfer_ff_ok:
    # Print final PASS result
    print("SPI Test: PASS")

    # Exit with success status
    sys.exit(0)

else:
    # Print final FAIL result
    print("SPI Test: FAIL")

    # Exit with failure status
    sys.exit(1)

    