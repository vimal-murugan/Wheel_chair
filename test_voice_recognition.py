import speech_recognition as sr

# Initialize recognizer
recognizer = sr.Recognizer()

# Use microphone as source
with sr.Microphone() as source:
    print("Please speak into the microphone...")
    audio = recognizer.listen(source)
    try:
        # Recognize speech
        text = recognizer.recognize_google(audio)
        print(f"You said: {text}")
    except sr.UnknownValueError:
        print("Sorry, could not understand the audio.")
    except sr.RequestError as e:
        print(f"Error with the recognition service: {e}")
