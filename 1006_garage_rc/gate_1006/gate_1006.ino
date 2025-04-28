#include <Servo.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define RED_LED 2
#define YELLOW_LED 3
#define GREEN_LED 4

#define TRIG1 6
#define ECHO1 5

#define TRIG2 8
#define ECHO2 9

#define SERVO_PIN 12
#define BUZZER_PIN 13

// Constants
#define MAX_CARS 3
#define DISTANCE_THRESHOLD 10  // cm

Servo gateServo;
int carCount = 0;
bool gateOpen = false;
bool oledWorking = false; // New flag

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting setup..."));

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed. Continuing without display."));
    oledWorking = false;
  } else {
    oledWorking = true;
    display.clearDisplay();
    display.display();
    Serial.println(F("OLED display initialized successfully."));
  }

  // Pin modes
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);
  closeGate();

  updateLEDs();
  updateDisplay();

  Serial.println(F("Setup complete."));
}

void loop() {
  long distance1 = readUltrasonic(TRIG1, ECHO1);
  long distance2 = readUltrasonic(TRIG2, ECHO2);

  Serial.print(F("Distance1: "));
  Serial.print(distance1);
  Serial.print(F(" cm | Distance2: "));
  Serial.print(distance2);
  Serial.println(F(" cm"));

  // Detect car at entry
  if (distance1 < DISTANCE_THRESHOLD && !gateOpen) {
    Serial.println(F("Car detected at entry."));
    if (carCount < MAX_CARS) {
      welcomeSound();
      openGate();
      delay(2000); // Wait for car to pass
      carCount++;
      Serial.print(F("Car entered. Current count: "));
      Serial.println(carCount);
      closeGate();
      updateLEDs();
      updateDisplay();
    } else {
      Serial.println(F("Garage full! Cannot allow entry."));
      warningSound();
    }
    delay(1000); // Small delay to avoid double detection
  }

  // Detect car at exit
  if (distance2 < DISTANCE_THRESHOLD && !gateOpen) {
    Serial.println(F("Car detected at exit."));
    if (carCount > 0) {
      openGate();
      delay(2000); // Wait for car to pass
      carCount--;
      Serial.print(F("Car exited. Current count: "));
      Serial.println(carCount);
      closeGate();
      updateLEDs();
      updateDisplay();
    } else {
      Serial.println(F("No cars inside to exit."));
    }
    delay(1000);
  }
}

void openGate() {
  Serial.println(F("Opening gate slowly..."));
  for (int pos = 0; pos <= 90; pos++) { // move from 0° to 90°
    gateServo.write(pos);
    delay(15); // adjust speed here (smaller delay = faster)
  }
  gateOpen = true;
}

void closeGate() {
  Serial.println(F("Closing gate slowly..."));
  for (int pos = 90; pos >= 0; pos--) { // move from 90° to 0°
    gateServo.write(pos);
    delay(15); // same speed
  }
  gateOpen = false;
}

void welcomeSound() {
  Serial.println(F("Playing welcome sound..."));
  tone(BUZZER_PIN, 1000); // Start tone
  delay(200);
  noTone(BUZZER_PIN); // Stop tone
  delay(50);
  tone(BUZZER_PIN, 1500);
  delay(200);
  noTone(BUZZER_PIN);
}

void warningSound() {
  Serial.println(F("Playing warning sound..."));
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 400);
    delay(300);
    noTone(BUZZER_PIN);
    delay(100);
  }
}

void updateLEDs() {
  Serial.println(F("Updating LEDs..."));
  if (carCount >= MAX_CARS) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    Serial.println(F("LED Status: RED ON"));
  } else if (carCount == MAX_CARS - 1) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Serial.println(F("LED Status: YELLOW ON"));
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println(F("LED Status: GREEN ON"));
  }
}

void updateDisplay() {
  if (!oledWorking) {
    Serial.println(F("Skipping OLED update (OLED not working)."));
    return;
  }

  Serial.println(F("Updating OLED display..."));
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print(F("Cars: "));
  display.print(carCount);
  display.display();
}

long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout after 30ms
  if (duration == 0) {
    Serial.println(F("Ultrasonic timeout."));
    return 999; // If timeout, return a big distance
  }
  
  long distance = duration * 0.034 / 2;
  return distance;
}