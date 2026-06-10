import serial  # Import pyserial to communicate with the UART port
import time    # Import time to add delays while waiting for the board response
import sys     # Import sys to exit the script with PASS or FAIL status


PORT = "/dev/ttyUSB0"  # Store the Linux serial port name
BAUDRATE = 115200      # Store the UART baud rate used by Zephyr shell
TIMEOUT = 0.1          # Store a small serial timeout for non-blocking reads


try:
    # Open the serial port using the selected port, baud rate, and timeout
    ser = serial.Serial(port=PORT, baudrate=BAUDRATE, timeout=TIMEOUT)

    # Wait for the serial connection to become stable
    time.sleep(1.0)

except serial.SerialException as error:
    # Print an error message if the serial port cannot be opened
    print(f"Could not open serial port {PORT}: {error}")

    # Exit the script with failure code
    sys.exit(1)


# Clear any old received data before sending the command
ser.reset_input_buffer()

# Clear any old transmitted data before sending the command
ser.reset_output_buffer()

# Send an Enter key first to wake up the Zephyr shell prompt
ser.write(b"\r\n")

# Wait a short time for the shell to print its prompt
time.sleep(0.2)

# Clear the prompt output so we only read the command response
ser.reset_input_buffer()

# Send the Zephyr shell command with CRLF line ending
ser.write(b"tb_ping\r\n")

# Force pyserial to send the data immediately
ser.flush()


# Create an empty string to store the full response
response = ""

# Store the start time of the read loop
start_time = time.time()

# Keep reading for 2 seconds
while time.time() - start_time < 2.0:
    # Check if there is any received data waiting in the serial buffer
    if ser.in_waiting > 0:
        # Read all currently available bytes from the serial buffer
        received_bytes = ser.read(ser.in_waiting)

        # Convert bytes to text and ignore invalid characters
        received_text = received_bytes.decode("utf-8", errors="ignore")

        # Add the received text to the full response
        response += received_text

    # Wait a little before checking the serial buffer again
    time.sleep(0.05)


# Close the serial port after finishing the test
ser.close()

# Print the full response received from the board
print("Received:")
print(response)


# Check if the expected word PONG exists in the response
if "PONG" in response:
    # Print PASS if the board replied correctly
    print("Communication Test: PASS")

    # Exit the script with success code
    sys.exit(0)

else:
    # Print FAIL if the expected response was not found
    print("Communication Test: FAIL")

    # Exit the script with failure code
    sys.exit(1)

    