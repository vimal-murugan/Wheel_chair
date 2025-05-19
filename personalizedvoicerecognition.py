import sounddevice as sd
import numpy as np
import librosa
import scipy.io.wavfile as wav
from scipy.spatial.distance import cosine
import speech_recognition as sr

# Predefined wheelchair commands
VALID_COMMANDS = ["go forward", "go backward", "turn right", "please stop"]

# Record audio
def record_voice(filename, duration=5, fs=44100):
    print(f"Recording... Please speak for {duration} seconds.")
    try:
        recording = sd.rec(int(duration * fs), samplerate=fs, channels=1, dtype='float32')
        sd.wait()
        wav.write(filename, fs, (recording * 32767).astype(np.int16))  # Save as WAV file
        print(f"Recording saved as {filename}")
    except Exception as e:
        print(f"Error during recording: {e}")

# Extract MFCC features
def extract_features(filename):
    try:
        y, sr = librosa.load(filename, sr=None)
        mfcc = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=13)
        mfcc = (mfcc - np.mean(mfcc)) / np.std(mfcc)  # Normalize features
        return np.mean(mfcc.T, axis=0)
    except Exception as e:
        print(f"Error extracting features: {e}")
        return None

# Recognize speech command
def recognize_command(filename):
    recognizer = sr.Recognizer()
    try:
        with sr.AudioFile(filename) as source:
            print("Recognizing command...")
            audio = recognizer.record(source)
            command = recognizer.recognize_google(audio).lower()
            print(f"Recognized command: {command}")
            return command
    except sr.UnknownValueError:
        print("Could not understand the command.")
        return None
    except sr.RequestError as e:
        print(f"Speech recognition error: {e}")
        return None

# Check if audio is silent
def is_silent(filename):
    try:
        y, _ = librosa.load(filename, sr=None)
        rms = np.sqrt(np.mean(y**2))
        print(f"RMS value of the audio: {rms}")
        return rms < 0.01  # Threshold for silence
    except Exception as e:
        print(f"Error checking silence: {e}")
        return True

# Main script
if __name__ == "__main__":
    try:
        # Step 1: Record authorized user's voice profile
        print("Step 1: Recording voice profile...")
        record_voice("authorized_user.wav")

        # Step 2: Extract features for the authorized user
        print("Step 2: Extracting features...")
        authorized_features = extract_features("authorized_user.wav")
        if authorized_features is None:
            print("Failed to extract features from the authorized user's voice.")
            exit(1)
        print("Authorized user features extracted.")

        # Main loop
        while True:
            print("Step 3: Waiting for a new voice command...")
            record_voice("test_command.wav")  # Record a new command
            
            # Check for silence
            if is_silent("test_command.wav"):
                print("Silent or invalid input detected. Please speak clearly.")
                continue

            # Step 4: Extract features for the new command
            test_features = extract_features("test_command.wav")
            if test_features is None:
                print("Failed to extract features from the new command.")
                continue

            # Step 5: Compare features using cosine similarity
            similarity = 1 - cosine(authorized_features, test_features)
            threshold = 0.8  # Set a threshold for verification
            print(f"Similarity: {similarity:.2f}")

            if similarity > threshold:
                print("Voice verified!")

                # Step 6: Recognize the command
                command = recognize_command("test_command.wav")
                if command and command in VALID_COMMANDS:
                    print(f"Authorized command recognized: {command}")
                else:
                    print("Command not recognized or invalid.")
            else:
                print("Voice not recognized.")
            
            # Prompt the user to continue or exit
            choice = input("\nDo you want to test another voice command? (y/n): ").strip().lower()
            if choice != 'y':
                print("Exiting the program. Goodbye!")
                break

    except KeyboardInterrupt:
        print("\nProgram interrupted by the user.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
