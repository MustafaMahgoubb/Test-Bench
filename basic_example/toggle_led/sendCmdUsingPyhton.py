import serial
import time
import sys

# 1. Initialize the serial connection
try:
    # Must use the full path in Linux
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    # Give the serial port a moment to settle
    time.sleep(1)  
except Exception as e:
    print(f"Could not open port: {e}")
    # Exit the script if we can't open the port
    sys.exit(1)

if ser.is_open:
    print("Port opened successfully! Sending command...")
    
    # 2. Send the command
    ser.write(b"blink_led\n")
    
    # 3. IMPORTANT: Wait 100 milliseconds for the board to process and reply
    time.sleep(0.1)
    
    # 4. Read everything the board sent back (Echo + Message + New Prompt)
    if ser.in_waiting > 0:
        # read_all() grabs everything in the buffer at once
        raw_response = ser.read_all().decode('utf-8', errors='ignore')
        
        print("\n--- Raw Data from Blackpill ---")
        print(raw_response)
        print("-------------------------------")
        
        # Optional: Parse out just your specific message
        if "LED Toggled!" in raw_response:
            print("SUCCESS: The board confirmed the LED was toggled!")

    # 5. Always close the port when done
    ser.close()