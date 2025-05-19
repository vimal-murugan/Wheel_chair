#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP32Servo.h>

// Wi-Fi credentials
#define WIFI_SSID "police"
#define WIFI_PASSWORD "123456789"

// Firebase configuration
#define API_KEY "AIzaSyDu47T1FhjtpTtz_tVeOUDKM9LXwoXHGus"
#define DATABASE_URL "https://voicewheelchair-default-rtdb.firebaseio.com/"
#define USER_EMAIL "vimalm1292003@gmail.com"
#define USER_PASSWORD "Vimal@2003"
#define PROXIMITY_SENSOR_PIN 4 // Pin connected to the proximity sensor

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Motor control pins (connected to L298 motor driver)
#define MOTOR1_IN1 5  // Motor 1 input 1
#define MOTOR1_IN2 18 // Motor 1 input 2
#define MOTOR2_IN1 19 // Motor 2 input 1
#define MOTOR2_IN2 21 // Motor 2 input 2
#define MOTOR_ENABLE1 12 // PWM pin for motor 1 speed control
#define MOTOR_ENABLE2 13 // PWM pin for motor 2 speed control

// Servo motor pins
#define SERVO1_PIN 27
#define SERVO11_PIN 25
#define SERVO2_PIN 23

Servo servo1;
Servo servo11;
Servo servo2;

// Global servo angles
int servo1Angle = 0;  // Default neutral position
int servo11Angle = 0; // Default neutral position
int servo2Angle = 0;  // Default neutral position

// Motor speed variables
int motorSpeed1 = 50; // Default speed for motor 1
int motorSpeed2 = 50; // Default speed for motor 2

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize motor pins
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT);
  pinMode(MOTOR2_IN2, OUTPUT);
  pinMode(MOTOR_ENABLE1, OUTPUT);
  pinMode(MOTOR_ENABLE2, OUTPUT);
  pinMode(PROXIMITY_SENSOR_PIN, INPUT_PULLUP); // Set the proximity sensor pin as an input

  stopMotors();

  // Attach servos
  servo1.attach(SERVO1_PIN);
  servo11.attach(SERVO11_PIN);
  servo2.attach(SERVO2_PIN);

  // Set initial servo positions
  servo1.write(servo1Angle);
  servo11.write(servo11Angle);
  servo2.write(servo2Angle);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi Connected!");
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Initialize Firebase
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);

  if (Firebase.ready()) {
    Serial.println("Firebase connected!");
  } else {
    Serial.println("Firebase connection failed, retrying...");
    delay(1000); // Wait before trying again
    Firebase.begin(&config, &auth);
  }
}

void stopMotors() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, LOW);
  analogWrite(MOTOR_ENABLE1, 0);
  analogWrite(MOTOR_ENABLE2, 0);
  Serial.println("Stopping Motors");
}

void moveForward() {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, HIGH);
  digitalWrite(MOTOR2_IN2, LOW);
  analogWrite(MOTOR_ENABLE1, motorSpeed1);
  analogWrite(MOTOR_ENABLE2, motorSpeed2);
  Serial.println("Moving Forward");
}

void moveBackward() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, HIGH);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, HIGH);
  analogWrite(MOTOR_ENABLE1, motorSpeed1);
  analogWrite(MOTOR_ENABLE2, motorSpeed2);
  Serial.println("Moving Backward");
}

void moveLeft() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, HIGH);
  digitalWrite(MOTOR2_IN1, HIGH);
  digitalWrite(MOTOR2_IN2, LOW);
  analogWrite(MOTOR_ENABLE1, motorSpeed1);
  analogWrite(MOTOR_ENABLE2, motorSpeed2);
  Serial.println("Turning Left");
}

void moveRight() {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, HIGH);
  analogWrite(MOTOR_ENABLE1, motorSpeed1);
  analogWrite(MOTOR_ENABLE2, motorSpeed2);
  Serial.println("Turning Right");
}

void updateServoPositions(int servo1Angle, int servo11Angle, int servo2Angle) {
  servo1.write(servo1Angle);
  servo11.write(servo11Angle);
  servo2.write(servo2Angle);
}

void loop() {
  int proximitySensorValue = digitalRead(PROXIMITY_SENSOR_PIN);
  Serial.print("Proximity Sensor Value: ");
  Serial.println(proximitySensorValue);

  // Create JSON object to send accident flag
  FirebaseJson commandData;
  commandData.set("accident", (proximitySensorValue == LOW)); // True if an accident is detected

  // Send accident data to Firebase under "/wheelchair/commands"
  if (Firebase.updateNode(fbdo, "/wheelchair/commands", commandData)) {
    Serial.println("Accident data updated in Firebase");
  } else {
    Serial.println("Failed to update accident data in Firebase");
    Serial.println(fbdo.errorReason());
  }

  // Existing code to get commands from Firebase
  if (Firebase.ready()) {
    if (Firebase.getJSON(fbdo, "/wheelchair/commands")) {
      FirebaseJson &json = fbdo.jsonObject();
      FirebaseJsonData jsonData;

      // Motor flags
      bool moveForwardFlag = false;
      bool moveBackwardFlag = false;
      bool moveLeftFlag = false;
      bool moveRightFlag = false;
      bool stopFlag = false;

      // Read motor commands
      if (json.get(jsonData, "forward") && jsonData.type == "boolean") {
        moveForwardFlag = jsonData.boolValue;
        if (moveForwardFlag) {
          moveForward();
        }
      }
      if (json.get(jsonData, "back") && jsonData.type == "boolean") {
        moveBackwardFlag = jsonData.boolValue;
        if (moveBackwardFlag) {
          moveBackward();
        }
      }
      if (json.get(jsonData, "left") && jsonData.type == "boolean") {
        moveLeftFlag = jsonData.boolValue;
        if (moveLeftFlag) {
          moveLeft();
        }
      }
      if (json.get(jsonData, "right") && jsonData.type == "boolean") {
        moveRightFlag = jsonData.boolValue;
        if (moveRightFlag) {
          moveRight();
        }
      }

      if (json.get(jsonData, "stop") && jsonData.type == "boolean") {
        stopFlag = jsonData.boolValue;
        if (stopFlag) {
          stopMotors();
        }
      }

      // Stop motors if all commands are false
      if (!moveForwardFlag && !moveBackwardFlag && !moveLeftFlag && !moveRightFlag) {
        stopMotors();
      }

      // Read speed values
      if (json.get(jsonData, "speed") && jsonData.type == "int") {
        motorSpeed1 = constrain(jsonData.intValue, 0, 255);
        Serial.printf("Updated Motor1 Speed: %d\n", motorSpeed1);
      }
      if (json.get(jsonData, "speed") && jsonData.type == "int") {
        motorSpeed2 = constrain(jsonData.intValue, 0, 255);
        Serial.printf("Updated Motor2 Speed: %d\n", motorSpeed2);
      }

      // Read servo angles
      if (json.get(jsonData, "servo1") && jsonData.type == "int") {
        servo1Angle = jsonData.intValue;
      }
      if (json.get(jsonData, "servo11") && jsonData.type == "int") {
        servo11Angle = jsonData.intValue;
      }
      if (json.get(jsonData, "servo2") && jsonData.type == "int") {
        servo2Angle = jsonData.intValue;
      }

      // Update servo positions
      updateServoPositions(servo1Angle, servo11Angle, servo2Angle);
    } else {
      Serial.println("Failed to get data from Firebase: " + fbdo.errorReason());
    }
  }
  delay(100); // Add a small delay to avoid excessive Firebase calls
}

