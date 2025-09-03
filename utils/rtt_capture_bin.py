#!/usr/bin/env python3
"""
Capture raw binary data from Segger RTT (via telnet) and save to a file, similar to the UART capture script.

Instructions:
1. Start J-Link Commander or RTT Viewer as described previously.
2. Run this script to connect to RTT via telnet (default: localhost:19021).
3. Captures for a fixed duration and saves to a .bin file.
"""
import socket
import time

# --- Configuration ---
RTT_HOST = "127.0.0.1"
RTT_PORT = 19021  # Default RTT Telnet port
CAPTURE_DURATION_S = 5
OUTPUT_FILENAME = 'capture.bin'
# --- End Configuration ---

print(f"Connecting to RTT at {RTT_HOST}:{RTT_PORT} ...")
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.connect((RTT_HOST, RTT_PORT))
except Exception as e:
    print(f"Failed to connect: {e}")
    exit(1)
print(f"Connected. Capturing for {CAPTURE_DURATION_S} seconds...")
s.settimeout(0.5)
all_data = bytearray()
start_time = time.time()
try:
    while time.time() - start_time < CAPTURE_DURATION_S:
        try:
            data = s.recv(4096)
            if data:
                all_data.extend(data)
        except socket.timeout:
            continue
    print(f"Capture complete. Received {len(all_data)} bytes.")
    with open(OUTPUT_FILENAME, 'wb') as f:
        f.write(all_data)
    print(f"Data saved to '{OUTPUT_FILENAME}'")
finally:
    s.close()
