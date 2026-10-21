#include <Arduino.h>
#include <PestoLink-Receive.h>

bool previousAState = false;

void setup() {
  Serial.begin(115200);
  PestoLink.begin("ESP32 Test");
}

void loop() {
  bool aPressed = PestoLink.buttonHeld(0);

  // Print once when A changes from released to pressed.
  if (aPressed && !previousAState) {
    Serial.println("Button A was pressed");
  }

  previousAState = aPressed;
  delay(10);
}
