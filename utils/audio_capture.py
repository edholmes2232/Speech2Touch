import serial
import time

# --- Configuration ---
# Update this to your STM32's serial port
SERIAL_PORT = '/dev/ttyACM1'  # For Windows, or '/dev/ttyACM0' for Linux/macOS
BAUD_RATE = 921600

# How many seconds of audio to capture
CAPTURE_DURATION_S = 5

# The name of the raw binary file to save
OUTPUT_FILENAME = 'capture.bin'
# --- End Configuration ---

print(f"Attempting to open {SERIAL_PORT} at {BAUD_RATE} bps...")

try:
    # Open the serial port
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Port opened successfully. Capturing for {CAPTURE_DURATION_S} seconds...")
        
        all_data = bytearray()
        start_time = time.time()
        
        # Read data from the serial port for the specified duration
        while time.time() - start_time < CAPTURE_DURATION_S:
            # Read all available bytes in the input buffer
            bytes_to_read = ser.in_waiting
            if bytes_to_read > 0:
                data = ser.read(bytes_to_read)
                all_data.extend(data)
        
        print(f"Capture complete. Received {len(all_data)} bytes.")

        # Write all the captured bytes to a binary file
        with open(OUTPUT_FILENAME, 'wb') as f:
            f.write(all_data)
        
        print(f"Data saved to '{OUTPUT_FILENAME}'")

except serial.SerialException as e:
    print(f"Error: {e}")
    print("Please check that the COM port is correct and not in use by another program.")