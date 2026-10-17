#include <Arduino.h>
#include <ESP32Encoder.h> // https://github.com/madhephaestus/ESP32Encoder.git

#define CLK 32
#define DT 33

// #define CLK 13 // CLK ENCODER 2
// #define DT 14 // DT ENCODER 2
#define ENABLE_BOOST_PIN 5

ESP32Encoder encoder;

void setup () {
  pinMode(ENABLE_BOOST_PIN, OUTPUT);
  digitalWrite(ENABLE_BOOST_PIN, HIGH);
  encoder.attachHalfQuad ( DT, CLK );
  encoder.setCount ( 0 );
  Serial.begin ( 115200 );
}

void loop () {
  digitalWrite(ENABLE_BOOST_PIN, HIGH);
  long newPosition = encoder.getCount() / 2;
  Serial.println(newPosition);
}

// #define ENCODER_A 13
// #define ENCODER_B 14

// volatile long encoderCount = 0;
// volatile int lastA = 0;


// void IRAM_ATTR encoderISR()
// {
//     int A = digitalRead(ENCODER_A);
//     int B = digitalRead(ENCODER_B);

//     if (A != lastA)
//     {
//         if (A == B)
//             encoderCount++;
//         else
//             encoderCount--;
//     }

//     lastA = A;
// }

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     lastA = digitalRead(ENCODER_A);

//     attachInterrupt(
//         digitalPinToInterrupt(ENCODER_A),
//         encoderISR,
//         CHANGE
//     );

//     Serial.println("Quadrature Encoder Test");
//     Serial.println("A = GPIO 13");
//     Serial.println("B = GPIO 14");
// }

// void loop()
// {
//     static long lastCount = 0;

//     // noInterrupts();
//     long count = encoderCount;
//     interrupts();

//     if (count != lastCount)
//     {
//         Serial.print("Encoder count: ");
//         Serial.println(count);

//         lastCount = count;
//     }

//     delay(10);
// }
