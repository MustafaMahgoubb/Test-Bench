import serial  # Import pyserial to communicate with the STM32 UART shell
import time    # Import time to add delays while waiting for responses
import sys     # Import sys to exit the script with success or failure status


PORT = "/dev/ttyUSB0"  # Define the serial port connected to the STM32 board
BAUDRATE = 115200      # Define the baud rate used by the Zephyr shell
TIMEOUT = 0.1          # Define a short timeout for serial read operations


def open_serial_port():
    # Open the serial port in exclusive mode to prevent picocom from using it at the same time
    ser = serial.Serial(
        port=PORT,          # Select the UART port path
        baudrate=BAUDRATE,  # Set the UART baud rate
        timeout=TIMEOUT,    # Set the read timeout
        exclusive=True      # Lock the serial port on Linux
    )

    # Wait for the UART connection to become stable
    time.sleep(1.0)

    # Return the opened serial object
    return ser


def read_response(ser, read_time):
    # Create an empty string to store the received response
    response = ""

    # Save the current time before starting the read loop
    start_time = time.time()

    # Keep reading until the timeout expires
    while time.time() - start_time < read_time:
        # Check if there are received bytes in the serial buffer
        if ser.in_waiting > 0:
            # Read all available bytes from the serial buffer
            data = ser.read(ser.in_waiting)

            # Convert received bytes to text
            text = data.decode("utf-8", errors="ignore")

            # Add the received text to the full response
            response += text

        # Wait a short time before checking again
        time.sleep(0.05)

    # Return the full received response
    return response


def send_shell_command(ser, command, read_time=2.0):
    # Clear any old received data before sending a new command
    ser.reset_input_buffer()

    # Clear any old transmitted data before sending a new command
    ser.reset_output_buffer()

    # Send Enter to wake up the Zephyr shell
    ser.write(b"\r\n")

    # Force the Enter key to be sent immediately
    ser.flush()

    # Wait a short time for the shell prompt
    time.sleep(0.2)

    # Clear the shell prompt from the receive buffer
    ser.reset_input_buffer()

    # Create the full command line with CRLF ending
    command_line = f"{command}\r\n"

    # Send the command to the STM32 board
    ser.write(command_line.encode("utf-8"))

    # Force the command to be sent immediately
    ser.flush()

    # Read the response from the STM32 board
    response = read_response(ser, read_time)

    # Print the sent command
    print(f"Command: {command}")

    # Print the received response
    print("Response:")
    print(response)

    # Return the received response to the caller
    return response


try:
    # Open the UART serial port
    ser = open_serial_port()

except serial.SerialException as error:
    # Print the error if the port cannot be opened
    print(f"Could not open serial port {PORT}: {error}")

    # Exit with failure because communication cannot continue
    sys.exit(1)


# Send ping command to check that communication is working
ping_response = send_shell_command(ser, "tb_ping")

# Send DIO loopback test command to test PB0 and PB1
dio_response = send_shell_command(ser, "tb_dio_test")

# Close the serial port after finishing the test
ser.close()


# Check if the ping response contains PONG
ping_ok = "PONG" in ping_response

# Check if the DIO response contains the PASS result
dio_ok = "DIO_TEST PASS HIGH 1 LOW 0" in dio_response


# Check if both tests passed
if ping_ok and dio_ok:
    # Print final PASS result
    print("DIO Test: PASS")

    # Exit with success status
    sys.exit(0)

else:
    # Print final FAIL result
    print("DIO Test: FAIL")

    # Exit with failure status
    sys.exit(1)

    