import serial
import time
import re
import csv
import sys

# ==========================================
# CONFIGURATION
# ==========================================
PORT = '/dev/ttyUSB0'  # Change if your Blackpill is on a different port
BAUDRATE = 115200
TIMEOUT = 2

# Frequencies to test (From 15 Hz up to 100 kHz)
TEST_FREQS = [15, 20, 50, 100, 500, 1000, 5000, 10000, 20000, 50000, 100000]
# Duty cycles to test at each frequency
TEST_DUTIES = [25, 50, 75]
# Number of readings to take per configuration
SAMPLES_PER_CONFIG = 10  # Set to 300 if you want ~10,000 total readings!

def setup_serial():
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)
        time.sleep(1) # Wait for connection to settle
        return ser
    except Exception as e:
        print(f"Error opening serial port {PORT}: {e}")
        sys.exit(1)

def send_command(ser, cmd):
    ser.write((cmd + '\n').encode('utf-8'))
    time.sleep(0.05)

def read_pwm_in(ser):
    # Flush input buffer so we don't read old messages
    ser.reset_input_buffer()
    
    # Send the read command
    send_command(ser, "pwm_in read 1")
    
    start_time = time.time()
    response = ""
    
    # Wait until we see "Duty:" or "ERR"
    while time.time() - start_time < TIMEOUT:
        if ser.in_waiting:
            chunk = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            response += chunk
            if "Duty:" in response or "ERR" in response:
                break
        time.sleep(0.01)
        
    # Parse the response
    if "ERR" in response:
        # Extract just the error message line
        err_msg = [line for line in response.split('\n') if "ERR" in line]
        err_str = err_msg[0] if err_msg else response.strip()
        return None, None, err_str
        
    # Use Regex to extract the numbers from "Freq: 1000 Hz, Duty: 25.00"
    match = re.search(r"Freq:\s*(\d+)\s*Hz,\s*Duty:\s*([\d.]+)", response)
    if match:
        freq = int(match.group(1))
        duty = float(match.group(2))
        return freq, duty, None
    else:
        return None, None, f"Parse Error: {response.strip()}"

def run_statistics():
    ser = setup_serial()
    print(f"Connected to {PORT}. Starting PWM Statistics Gathering...")
    print(f"Testing {len(TEST_FREQS)} frequencies, {len(TEST_DUTIES)} duty cycles, {SAMPLES_PER_CONFIG} samples each.")
    
    results = []
    total_tests = len(TEST_FREQS) * len(TEST_DUTIES)
    current_test = 0

    for freq in TEST_FREQS:
        for duty in TEST_DUTIES:
            current_test += 1
            print(f"\n[{current_test}/{total_tests}] Testing Target: {freq} Hz @ {duty}% Duty Cycle")
            
            # Set the PWM Out
            send_command(ser, f"pwm_out freq 1 {freq} {duty}")
            time.sleep(0.2) # Wait a moment for hardware PWM to stabilize
            
            valid_readings = 0
            freq_errors = []
            duty_errors = []
            
            for i in range(SAMPLES_PER_CONFIG):
                m_freq, m_duty, err = read_pwm_in(ser)
                
                if err:
                    print(f"  Sample {i+1}: FAILED ({err.strip()})")
                else:
                    # Calculate Percentage Error: |Measured - Expected| / Expected * 100
                    f_err = abs(m_freq - freq) / freq * 100.0
                    d_err = abs(m_duty - duty) / duty * 100.0
                    
                    freq_errors.append(f_err)
                    duty_errors.append(d_err)
                    valid_readings += 1
            
            if valid_readings > 0:
                avg_f_err = sum(freq_errors) / valid_readings
                max_f_err = max(freq_errors)
                avg_d_err = sum(duty_errors) / valid_readings
                
                print(f"  -> Valid Readings: {valid_readings}/{SAMPLES_PER_CONFIG}")
                print(f"  -> Avg Freq Error: {avg_f_err:.2f}% (Max: {max_f_err:.2f}%)")
                print(f"  -> Avg Duty Error: {avg_d_err:.2f}%")
                
                results.append({
                    "Target Freq (Hz)": freq,
                    "Target Duty (%)": duty,
                    "Avg Freq Error (%)": round(avg_f_err, 2),
                    "Max Freq Error (%)": round(max_f_err, 2),
                    "Avg Duty Error (%)": round(avg_d_err, 2)
                })
            else:
                print("  -> ALL READINGS FAILED (Likely Out of Range)")
                results.append({
                    "Target Freq (Hz)": freq,
                    "Target Duty (%)": duty,
                    "Avg Freq Error (%)": "FAIL",
                    "Max Freq Error (%)": "FAIL",
                    "Avg Duty Error (%)": "FAIL"
                })

    # Save to CSV
    csv_file = "pwm_statistics_report.csv"
    with open(csv_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=["Target Freq (Hz)", "Target Duty (%)", "Avg Freq Error (%)", "Max Freq Error (%)", "Avg Duty Error (%)"])
        writer.writeheader()
        writer.writerows(results)
        
    print(f"\n==================================================")
    print(f"Finished! Full statistical report saved to {csv_file}")
    print(f"==================================================")
    
    # Turn off PWM safely when done
    send_command(ser, "pwm_out stop 1")
    ser.close()

if __name__ == "__main__":
    run_statistics()
