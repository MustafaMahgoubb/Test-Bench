import serial
import time

class TestBenchLibrary:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        # This runs automatically when Robot starts the test
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(1) # Let connection settle
        
    def set_pwm_duty_cycle(self, channel, duty_cycle):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_out set {channel} {duty_cycle}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")

    def close_connection(self):
        self.ser.close()