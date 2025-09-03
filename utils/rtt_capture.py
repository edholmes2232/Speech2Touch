#!/usr/bin/env python3
"""
Simple Segger RTT Telnet client for viewing RTT output with timestamps.

Instructions:
1. Start J-Link Commander or RTT Viewer as described in the script header.
2. Run this script to connect to RTT via telnet (default: localhost:19021).
3. Output will be printed to stdout with timestamps.
"""
import socket
import time
import sys
from datetime import datetime

# --- Configuration ---
RTT_HOST = "127.0.0.1"
RTT_PORT = 19021  # Default RTT Telnet port
CAPTURE_DURATION_S = 0  # 0 = run forever, >0 = stop after N seconds
PRINT_BINARY = False     # Set True to print raw binary, False for line mode
# --- End Configuration ---


def is_preamble(line):
    # Remove lines that are SEGGER/J-Link preamble or empty
    if not line.strip():
        return True
    preambles = [
        'SEGGER',
        'Process: JLink',
        'J-Link',
        'JLink',
        'INFO main : Startup',
    ]
    for p in preambles:
        if line.startswith(p):
            return True
    return False

def print_line(line_bytes):
    try:
        line = line_bytes.decode(errors='replace').rstrip('\r\n')
    except Exception:
        line = str(line_bytes)
    if is_preamble(line):
        return
    print(line)

def main():
    print(f"Connecting to RTT at {RTT_HOST}:{RTT_PORT} ...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((RTT_HOST, RTT_PORT))
    except Exception as e:
        print(f"Failed to connect: {e}")
        sys.exit(1)
    print("Connected. Press Ctrl+C to quit.")
    s.settimeout(0.5)
    start_time = time.time()
    buf = b''
    try:
        while True:
            if CAPTURE_DURATION_S > 0 and (time.time() - start_time) > CAPTURE_DURATION_S:
                print("Capture duration reached. Exiting.")
                break
            try:
                data = s.recv(4096)
                if not data:
                    time.sleep(0.05)
                    continue
                if PRINT_BINARY:
                    sys.stdout.buffer.write(data)
                    sys.stdout.buffer.flush()
                else:
                    buf += data
                    while b'\n' in buf:
                        line, buf = buf.split(b'\n', 1)
                        print_line(line)
            except socket.timeout:
                continue
            except KeyboardInterrupt:
                print("\nInterrupted by user.")
                break
    finally:
        s.close()
        print("Disconnected.")

if __name__ == "__main__":
    main()
