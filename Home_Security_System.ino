#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
// Pin definitions
const int trigPin1 = A3;   // Ultrasonic sensor 1 - Trigger pin
const int echoPin1 = 6;   // Ultrasonic sensor 1 - Echo pin
const int trigPin2 = A2;   // Ultrasonic sensor 2 - Trigger pin
const int echoPin2 = A1;   // Ultrasonic sensor 2 - Echo pin
const int buzzer = 7;    // Buzzer pin
const int led1 = 11;      // LED 1 pin
const int led2 = 2;      // LED 2 pin (Analog pin A4)

// System states
bool systemOn = false;    // System activated
bool alarmOn = false;     // Alarm active

// Keypad setup
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {12,13,5,10};
byte colPins[COLS] = {4,8,9,11};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// I2C LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD address 0x27

// Passwords
const String PASSWORD_ON = "1234";
const String PASSWORD_ALARM_OFF = "4321";

String inputPassword = ""; // Store keypad input

// Motion detection variables
unsigned long lastMotionTime = 0;
const unsigned long motionDelay = 5000; // 5 seconds delay between detections

void setup() {
  Serial.begin(9600);
  // Initialize pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  // Initialize I2C LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Enter Password");

   // Start Serial Monitor
  Serial.println("System Started");
}

void loop() {
  char key = keypad.getKey();

  // Handle keypad input
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    if (key == '#') { // End of password input
      lcd.clear();
      if (inputPassword == PASSWORD_ON) {
        systemOn = true;
        alarmOn = false;
        lcd.print("System Activated");
        Serial.println("System Activated");
      }
      else if (inputPassword == PASSWORD_ALARM_OFF) {
        stopAlarm();
        lcd.print("Alarm Stopped");
        Serial.println("Alarm Stopped");
      }
      else {
        lcd.print("Wrong Password");
        Serial.println("Wrong Password");
      }
      inputPassword = ""; // Reset input
      delay(2000);
      lcd.clear();
      lcd.print("Enter Password");
    } else {
      if (inputPassword.length() < 4) { // Limit input length
        inputPassword += key; // Append key to password
        lcd.setCursor(0, 1);
        lcd.print(inputPassword);
      }
    }
  }

  // Check motion if the system is ON
  if (systemOn && !alarmOn) {
    bool motion1 = checkMotion(trigPin1, echoPin1);
    bool motion2 = checkMotion(trigPin2, echoPin2);

    if (motion1 || motion2) {
      activateAlarm();
    }
  }

  // Alarm active: Blink LEDs and sound buzzer
  if (alarmOn) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    tone(buzzer, 1000); // Sound buzzer (high-pitched tone)
    delay(250);
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    delay(250);
  }
}

// Function to check motion using ultrasonic sensor
bool checkMotion(int trigPin, int echoPin) {
  long duration;
  int distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm

  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);

  if (distance > 0 && distance < 10) { // Motion detected within 10 cm
    if (millis() - lastMotionTime > motionDelay) {
      lastMotionTime = millis();
      return true;  // Trigger alarm only once every 5 seconds
    }
  }
  return false;
}

// Function to activate the alarm
void activateAlarm() {
  alarmOn = true;
  lcd.clear();
  lcd.print("Motion Detected!");
  Serial.println("Alarm Activated!");
  // Add a unique sound for the alarm activation
  tone(buzzer, 1500); // A lower pitch sound
  delay(500);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 2000); // A higher pitch sound
  delay(500);
  noTone(buzzer);
}

// Function to stop the alarm
void stopAlarm() {
  alarmOn = false;
  noTone(buzzer);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  Serial.println("Alarm Stopped");
}
