import serial  # Import pyserial to communicate with the STM32 UART shell
import time    # Import time to add delays while waiting for UART responses


class testbench_serial:
    # Tell Robot Framework to use one library instance for the whole test suite
    ROBOT_LIBRARY_SCOPE = "SUITE"

    # Create the library object and receive arguments from the Robot file
    def __init__(self, port="/dev/ttyUSB0", baudrate=115200, timeout=0.1):
        # Store the UART port path, for example /dev/ttyUSB0
        self.port = port

        # Convert the baudrate argument to integer because Robot may pass it as text
        self.baudrate = int(baudrate)

        # Convert the timeout argument to float because Robot may pass it as text
        self.timeout = float(timeout)

        # Store the serial connection object, and keep it empty before opening the port
        self.ser = None

    # Open the UART connection to the STM32 board
    def open_connection(self):
        # Check if the serial port is already open
        if self.ser is not None and self.ser.is_open:
            # Return directly because the connection is already open
            return

        # Open the UART serial port
        self.ser = serial.Serial(
            port=self.port,          # Select the UART port path
            baudrate=self.baudrate,  # Set the UART baud rate
            timeout=self.timeout,    # Set the UART read timeout
            exclusive=True           # Prevent another program from using the same port on Linux
        )

        # Wait for the UART connection to become stable
        time.sleep(1.0)

    # Close the UART connection
    def close_connection(self):
        # Check if the serial object exists
        if self.ser is not None:
            # Check if the serial port is open
            if self.ser.is_open:
                # Close the UART serial port
                self.ser.close()

            # Clear the serial object after closing the port
            self.ser = None

    # Read data from the STM32 for a specific time
    def read_response(self, read_time=2.0):
        # Convert read_time to float because Robot may pass it as text
        read_time = float(read_time)

        # Create an empty string to store the full response
        response = ""

        # Save the current time before starting the read loop
        start_time = time.time()

        # Keep reading until the read time expires
        while time.time() - start_time < read_time:
            # Check if the UART received any bytes
            if self.ser.in_waiting > 0:
                # Read all available bytes from the UART buffer
                data = self.ser.read(self.ser.in_waiting)

                # Convert the received bytes to text
                text = data.decode("utf-8", errors="ignore")

                # Add the received text to the full response
                response += text

            # Wait a short time before checking again
            time.sleep(0.05)

        # Return the full response text
        return response

    # Send one Zephyr shell command and return the response
    def send_shell_command(self, command, read_time=2.0):
        # Open the connection automatically if it was not opened before
        if self.ser is None or not self.ser.is_open:
            # Open the UART connection
            self.open_connection()

        # Clear old received data before sending a new command
        self.ser.reset_input_buffer()

        # Clear old transmitted data before sending a new command
        self.ser.reset_output_buffer()

        # Send Enter to wake up the Zephyr shell
        self.ser.write(b"\r\n")

        # Force the Enter command to be sent immediately
        self.ser.flush()

        # Wait for the Zephyr shell prompt
        time.sleep(0.2)

        # Clear the shell prompt from the receive buffer
        self.ser.reset_input_buffer()

        # Add CRLF to the command because Zephyr shell expects Enter
        command_line = f"{command}\r\n"

        # Send the command to the STM32 board
        self.ser.write(command_line.encode("utf-8"))

        # Force the command to be sent immediately
        self.ser.flush()

        # Read the STM32 response
        response = self.read_response(read_time)

        # Return the response to Robot Framework
        return response

    # Check that the response contains the expected text
    def response_should_contain(self, response, expected_text):
        # Check if the expected text does not exist in the response
        if expected_text not in response:
            # Raise an error so Robot Framework marks the test as failed
            raise AssertionError(f"Expected '{expected_text}' but got:\n{response}")