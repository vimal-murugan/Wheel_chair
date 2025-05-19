#include <ESP32Servo.h>  // Include the ESP32Servo library

Servo myServo;  // Create a servo object

void setup() {
  myServo.attach(13);  // Attach the servo to GPIO 13 (or another PWM-capable pin)
}

void loop() {
  // Slowly move the servo from 0 to 40 degrees
  for (int pos = 0; pos <= 30; pos++) {
    myServo.write(pos);  // Move the servo to the current position
    delay(30);  // Delay to control the speed (increase to slow down)
  }
  delay(1000);  // Wait for 1 second

  // Slowly move the servo from 40 to 0 degrees
  for (int pos = 30; pos >= 0; pos--) {
    myServo.write(pos);  // Move the servo to the current position
    delay(30);  // Delay to control the speed (increase to slow down)
  }
  delay(1000);  // Wait for 1 second
}
