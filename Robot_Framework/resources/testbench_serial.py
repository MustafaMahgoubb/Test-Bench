import serial
import time
import re
from robot.api import logger


class testbench_serial:
    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self, port="/dev/ttyUSB0", baudrate=115200, timeout=0.1):
        self.port = port
        self.baudrate = int(baudrate)
        self.timeout = float(timeout)

        self.ser = None

        # DUT serial object for tests that need to read DUT logs
        self.dut_ser = None
        self.dut_port = None

    def open_connection(self):
        if self.ser is not None and self.ser.is_open:
            return

        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            timeout=self.timeout,
            exclusive=True
        )

        time.sleep(1.0)

    def close_connection(self):
        if self.ser is not None:
            if self.ser.is_open:
                self.ser.close()

            self.ser = None

    def open_dut_connection(self, dut_port="/dev/ttyUSB0"):
        self.dut_port = dut_port

        if self.dut_ser is not None and self.dut_ser.is_open:
            return

        self.dut_ser = serial.Serial(
            port=self.dut_port,
            baudrate=self.baudrate,
            timeout=self.timeout,
            exclusive=True
        )

        time.sleep(1.0)

    def close_dut_connection(self):
        if self.dut_ser is not None:
            if self.dut_ser.is_open:
                self.dut_ser.close()

            self.dut_ser = None

    def open_testbench_and_dut_connections(self, dut_port="/dev/ttyUSB0"):
        self.open_connection()
        self.open_dut_connection(dut_port)

    def close_all_connections(self):
        self.close_connection()
        self.close_dut_connection()

    def read_response(self, read_time=2.0):
        read_time = float(read_time)
        response = ""
        start_time = time.time()

        while time.time() - start_time < read_time:
            if self.ser.in_waiting > 0:
                data = self.ser.read(self.ser.in_waiting)
                text = data.decode("utf-8", errors="ignore")
                response += text

            time.sleep(0.05)

        return response

    def send_shell_command(self, command, read_time=2.0):
        if self.ser is None or not self.ser.is_open:
            self.open_connection()

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        self.ser.write(b"\r\n")
        self.ser.flush()
        time.sleep(0.2)

        self.ser.reset_input_buffer()

        command_line = f"{command}\r\n"
        self.ser.write(command_line.encode("utf-8"))
        self.ser.flush()

        response = self.read_response(read_time)

        return response

    def response_should_contain(self, response, expected_text):
        if expected_text not in response:
            raise AssertionError(f"Expected '{expected_text}' but got:\n{response}")

    def adc_response_should_be_valid(self, response):
        match = re.search(r"ADC_READ RAW\s+(\d+)\s+MV\s+(\d+)", response)

        if match is None:
            raise AssertionError(f"Invalid ADC response format:\n{response}")

        raw_value = int(match.group(1))
        mv_value = int(match.group(2))

        logger.console("\nADC Reading:")
        logger.console(f"  RAW     = {raw_value}")
        logger.console(f"  Voltage = {mv_value} mV")

        if raw_value < 0 or raw_value > 4095:
            raise AssertionError(f"ADC raw value out of range: {raw_value}")

        if mv_value < 0 or mv_value > 3300:
            raise AssertionError(f"ADC millivolt value out of range: {mv_value}")

    def spi_slave_ping_response_should_pass(self, response):
        match = re.search(
            r"SPI_SLAVE_PING PASS TX 0x([0-9A-Fa-f]{2}) RX 0x([0-9A-Fa-f]{2})",
            response
        )

        if match is None:
            raise AssertionError(f"Invalid SPI slave ping response:\n{response}")

        tx_text = match.group(1).upper()
        rx_text = match.group(2).upper()

        logger.console("\nSPI External DUT Result:")
        logger.console(f"  TX = 0x{tx_text}")
        logger.console(f"  RX = 0x{rx_text}")

        if tx_text != "A5":
            raise AssertionError(f"Unexpected SPI TX byte: 0x{tx_text}")

        if rx_text != "5A":
            raise AssertionError(f"Unexpected SPI RX byte: 0x{rx_text}")

    def dio_test_response_should_pass(self, response):
        match = re.search(
            r"DIO_TEST PASS HIGH\s+([01])\s+LOW\s+([01])",
            response
        )

        if match is None:
            raise AssertionError(f"Invalid DIO test response:\n{response}")

        high_value = match.group(1)
        low_value = match.group(2)

        logger.console("\nDIO External DUT Result:")
        logger.console(f"  HIGH Read = {high_value}")
        logger.console(f"  LOW Read  = {low_value}")

        if high_value != "1":
            raise AssertionError(f"Expected HIGH read to be 1, got {high_value}")

        if low_value != "0":
            raise AssertionError(f"Expected LOW read to be 0, got {low_value}")

    def clear_dut_log_buffer(self):
        if self.dut_ser is None or not self.dut_ser.is_open:
            self.open_dut_connection()

        self.dut_ser.reset_input_buffer()
        self.dut_ser.reset_output_buffer()

    def read_dut_logs(self, read_time=5.0):
        if self.dut_ser is None or not self.dut_ser.is_open:
            self.open_dut_connection()

        read_time = float(read_time)
        logs = ""
        start_time = time.time()

        while time.time() - start_time < read_time:
            if self.dut_ser.in_waiting > 0:
                data = self.dut_ser.read(self.dut_ser.in_waiting)
                text = data.decode("utf-8", errors="ignore")
                logs += text

            time.sleep(0.05)

        logger.console("\nDUT Logs:")
        logger.console(logs)

        return logs

    def dut_pwm_output_should_match(self, logs):
        ch1_matches = re.findall(
            r"DUT_PWM_IN CH1 Freq:\s*(\d+)\s*Hz,\s*Duty:\s*(\d+)\.(\d+)",
            logs
        )

        ch2_matches = re.findall(
            r"DUT_PWM_IN CH2 Freq:\s*(\d+)\s*Hz,\s*Duty:\s*(\d+)\.(\d+)",
            logs
        )

        if not ch1_matches:
            raise AssertionError(f"CH1 PWM reading was not found in DUT logs:\n{logs}")

        if not ch2_matches:
            raise AssertionError(f"CH2 PWM reading was not found in DUT logs:\n{logs}")

        ch1_freq = None
        ch1_duty = None
        ch2_freq = None
        ch2_duty = None

        for match in ch1_matches:
            freq = int(match[0])
            duty = float(f"{match[1]}.{match[2]}")

            if 900 <= freq <= 1100 and 45.0 <= duty <= 55.0:
                ch1_freq = freq
                ch1_duty = duty
                break

        for match in ch2_matches:
            freq = int(match[0])
            duty = float(f"{match[1]}.{match[2]}")

            if 900 <= freq <= 1100 and 20.0 <= duty <= 30.0:
                ch2_freq = freq
                ch2_duty = duty
                break

        if ch1_freq is None:
            raise AssertionError(f"Valid CH1 PWM reading was not found in DUT logs:\n{logs}")

        if ch2_freq is None:
            raise AssertionError(f"Valid CH2 PWM reading was not found in DUT logs:\n{logs}")

        logger.console("\nPWM OUT External DUT Result:")
        logger.console(f"  CH1 Frequency = {ch1_freq} Hz")
        logger.console(f"  CH1 Duty      = {ch1_duty:.2f}%")
        logger.console(f"  CH2 Frequency = {ch2_freq} Hz")
        logger.console(f"  CH2 Duty      = {ch2_duty:.2f}%")
