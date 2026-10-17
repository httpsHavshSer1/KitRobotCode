#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

const int SERVO_PIN = 25;

void setup() {
  myServo.attach(SERVO_PIN);

  // Start at the center position
  myServo.write(90);
  delay(1000);
}

void loop() {
  // Sweep from 0 to 180 degrees
  for (int angle = 0; angle <= 180; angle+=5) {
    myServo.write(angle);
    delay(15);
  }

  // Sweep back from 180 to 0 degrees
  for (int angle = 180; angle >= 0; angle-=5) {
    myServo.write(angle);
    delay(15);
  }
}