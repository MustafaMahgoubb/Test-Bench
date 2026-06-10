import serial
import time

class TestBenchLibrary:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        # This runs automatically when Robot starts the test
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(1) # Let connection settle

# ##################################################
#                   PWM-OUT
# ##################################################
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

    def set_pwm_freq_and_duty_cycle(self, channel, freq ,duty_cycle):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_out freq {channel} {freq} {duty_cycle}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")


    def stop_pwm(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_out stop {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
        
# ##################################################
#                   PWM-IN
# ##################################################
    def get_pwm_dutyCycle(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_in get_dutyCycle {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    
    def get_pwm_frequency(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_in get_frequency {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    
    def get_pwm_frequency_and_dutyCycle(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"pwm_in read {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    
# ##################################################
#                   UART
# ##################################################    
    def uart_send(self, channel ,message):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"uart send {channel} {message}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    
    def uart_read(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"uart read {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
            
        import re
        # Strip ANSI escape codes (Zephyr shell uses VT100 colors by default)
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        response = ansi_escape.sub('', response)
            
        # Clean up the response to remove the command echo and Zephyr shell prompt
        lines = [line.strip() for line in response.split('\n') if line.strip()]
        
        if lines and "uart read" in lines[0]:
            lines.pop(0)  # Remove the command echo
            
        if lines and "blackpill:~$" in lines[-1]:
            lines.pop(-1) # Remove the shell prompt
            
        return '\n'.join(lines).strip()
        
    
    def uart_clear(self, channel):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"uart clear {channel}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    
    def uart_set_baudrate(self, channel ,baudrate):
        """ This Python function becomes a Robot Keyword! """
        # 1. Format the string exactly as Zephyr expects it
        command = f"uart set_baud {channel} {baudrate}\n"
        
        # 2. Send it to the Blackpill
        self.ser.write(command.encode('utf-8'))
        time.sleep(0.1)
        
        # 3. Read the response (Optional: check if it says 'ERR' or 'Invalid')
        response = self.ser.read_all().decode('utf-8', errors='ignore')
        if "Invalid" in response or "ERR" in response:
            raise Exception(f"Test Bench rejected command! Response: {response}")
        
    

    def close_connection(self):
        self.ser.close()