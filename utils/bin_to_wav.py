import wave

# --- Configuration ---
# The path to the raw binary file you exported from the debugger
RAW_CAPTURE_FILE = 'capture.bin'

# The name of the output WAV file you want to create
OUTPUT_WAV_FILE = 'output_audio.wav'

# Audio parameters (must match your project)
SAMPLE_RATE = 16000  # 16 kHz
NUM_CHANNELS = 1     # Mono
SAMPLE_WIDTH = 2     # 2 bytes for int16_t
# --- End Configuration ---

try:
    with open(RAW_CAPTURE_FILE, 'rb') as bin_file:
        # Read the raw audio data from the binary file
        raw_data = bin_file.read()

        # Create a new WAV file and write the data to it
        with wave.open(OUTPUT_WAV_FILE, 'wb') as wav_file:
            wav_file.setnchannels(NUM_CHANNELS)
            wav_file.setsampwidth(SAMPLE_WIDTH)
            wav_file.setframerate(SAMPLE_RATE)
            wav_file.writeframes(raw_data)

        print(f"Successfully created '{OUTPUT_WAV_FILE}'")

except FileNotFoundError:
    print(f"Error: The file '{RAW_CAPTURE_FILE}' was not found.")
    print("Please make sure the file path is correct.")