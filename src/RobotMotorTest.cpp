#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

void driveMotorOne(float power);
void driveMotorTwo(float power);

// PestoLink's BLE service and gamepad packet format.
static const char* SERVICE_UUID = "27df26c5-83f4-4964-bae0-d7b7cb0a1f54";
static const char* GAMEPAD_UUID = "452af57e-ad27-422c-88ae-76805ea641a9";
static const char* TELEMETRY_UUID = "266d9d74-3e10-4fcd-88d2-cb63b5324d0c";
static const uint32_t PACKET_TIMEOUT_MS = 500;
static const uint32_t TELEMETRY_PERIOD_MS = 200;
static const uint32_t SERVO_STEP_PERIOD_MS = 20;

const int M1_EN_PIN = 18;
const int M1_PH_PIN = 19;
const int M1_FAULT = 36;
const int M2_EN_PIN = 16;
const int M2_PH_PIN = 17;
// M2 fault is not connected on this board.
const int SLEEP_PIN = 23;
const int SERVO_PIN = 25;
static const int SERVO_STEP_DEGREES = 1;

portMUX_TYPE commandMux = portMUX_INITIALIZER_UNLOCKED;
float requestedM1 = 0.0f;
float requestedM2 = 0.0f;
uint32_t lastPacketMs = 0;
bool deviceConnected = false;
uint8_t requestedButtons = 0;
BLECharacteristic* telemetryCharacteristic = nullptr;
Servo armServo;
int servoAngle = 90;

float clampPower(float value) {
  if (value > 1.0f) return 1.0f;
  if (value < -1.0f) return -1.0f;
  return value;
}

float axisValue(uint8_t raw) {
  // PestoLink sends 127 for a centered joystick.
  const float value = (static_cast<float>(raw) - 127.0f) / 128.0f;
  return value > -0.05f && value < 0.05f ? 0.0f : value;
}

class PestoServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    portENTER_CRITICAL(&commandMux);
    deviceConnected = true;
    requestedM1 = 0.0f;
    requestedM2 = 0.0f;
    requestedButtons = 0;
    lastPacketMs = 0;
    portEXIT_CRITICAL(&commandMux);
    Serial.println("PestoLink connected");
  }

  void onDisconnect(BLEServer* server) override {
    portENTER_CRITICAL(&commandMux);
    deviceConnected = false;
    requestedM1 = 0.0f;
    requestedM2 = 0.0f;
    requestedButtons = 0;
    lastPacketMs = 0;
    portEXIT_CRITICAL(&commandMux);
    server->startAdvertising();
    Serial.println("PestoLink disconnected; motors stopped");
  }
};

class PestoGamepadCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    const std::string packet = characteristic->getValue();
    if (packet.size() != 18 || static_cast<uint8_t>(packet[0]) != 1) {
      portENTER_CRITICAL(&commandMux);
      requestedM1 = 0.0f;
      requestedM2 = 0.0f;
      requestedButtons = 0;
      lastPacketMs = 0;
      portEXIT_CRITICAL(&commandMux);
      return;
    }

    // Axis 0 is left/right; axis 1 is forward/backward.
    const float steering = axisValue(static_cast<uint8_t>(packet[1]));
    const float throttle = -axisValue(static_cast<uint8_t>(packet[2]));
    const float m1 = clampPower(throttle + steering);
    const float m2 = clampPower(throttle - steering);

    portENTER_CRITICAL(&commandMux);
    requestedM1 = m1;
    requestedM2 = m2;
    // Button 0 and button 1 are bits 0 and 1 of packet byte 5.
    requestedButtons = static_cast<uint8_t>(packet[5]) & 0x03;
    lastPacketMs = millis();
    portEXIT_CRITICAL(&commandMux);
  }
};

void setup() {
  pinMode(M1_PH_PIN, OUTPUT);
  pinMode(M1_EN_PIN, OUTPUT);
  pinMode(M2_EN_PIN, OUTPUT);
  pinMode(M2_PH_PIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  // GPIO36 has no internal pull-up; the fault line needs a board pull-up.
  pinMode(M1_FAULT, INPUT);
  digitalWrite(SLEEP_PIN, LOW);
  driveMotorOne(0.0f);
  driveMotorTwo(0.0f);
  armServo.attach(SERVO_PIN);
  armServo.write(servoAngle);
  Serial.begin(115200);

  BLEDevice::init("RobotMotorTest");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new PestoServerCallbacks());
  BLEService* service = server->createService(SERVICE_UUID);
  BLECharacteristic* gamepad = service->createCharacteristic(
      GAMEPAD_UUID, BLECharacteristic::PROPERTY_WRITE_NR);
  gamepad->setCallbacks(new PestoGamepadCallbacks());
  telemetryCharacteristic = service->createCharacteristic(
      TELEMETRY_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  telemetryCharacteristic->addDescriptor(new BLE2902());
  uint8_t telemetry[12] = {'R','E','A','D','Y',' ',' ',' ',0,255,0,0};
  telemetryCharacteristic->setValue(telemetry, sizeof(telemetry));
  service->start();
  // PestoLink scans for this service UUID. A 128-bit UUID and the full name
  // do not both fit in one 31-byte BLE advertisement, so split them.
  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x06);
  advertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
  BLEAdvertisementData scanResponseData;
  scanResponseData.setName("RobotMotorTest");
  BLEAdvertising* advertising = server->getAdvertising();
  advertising->setAdvertisementData(advertisementData);
  advertising->setScanResponseData(scanResponseData);
  server->startAdvertising();
  digitalWrite(SLEEP_PIN, HIGH);
  Serial.println("PestoLink ready: RobotMotorTest");
}

void loop() {
  float m1;
  float m2;
  uint32_t last;
  bool connected;
  uint8_t buttons;
  portENTER_CRITICAL(&commandMux);
  m1 = requestedM1;
  m2 = requestedM2;
  last = lastPacketMs;
  connected = deviceConnected;
  buttons = requestedButtons;
  portEXIT_CRITICAL(&commandMux);

  const bool freshCommand = connected && last != 0 &&
                            millis() - last <= PACKET_TIMEOUT_MS;
  if (!freshCommand || digitalRead(M1_FAULT) == LOW) {
    m1 = 0.0f;
    m2 = 0.0f;
  }
  driveMotorOne(m1);
  driveMotorTwo(m2);

  // Hold button 0 to lower and button 1 to raise the servo.
  // Releasing both (or pressing both) holds its current angle.
  static uint32_t lastServoStepMs = 0;
  if (freshCommand && millis() - lastServoStepMs >= SERVO_STEP_PERIOD_MS) {
    lastServoStepMs = millis();
    if (buttons == 0x01 && servoAngle > 0) {
      servoAngle -= SERVO_STEP_DEGREES;
      armServo.write(servoAngle);
    } else if (buttons == 0x02 && servoAngle < 180) {
      servoAngle += SERVO_STEP_DEGREES;
      armServo.write(servoAngle);
    }
  }

  static uint32_t lastTelemetryMs = 0;
  if (connected && millis() - lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
    lastTelemetryMs = millis();
    telemetryCharacteristic->notify();
  }
  delay(20);
}
void driveMotorOne(float power){
  if(power > 1){
    power = 1;
  }else if(power < -1){
    power = -1;
  }

  if (power > 0){
    analogWrite(M1_EN_PIN, 255 - (int)(255.0 * power));
    digitalWrite(M1_PH_PIN, HIGH);
  }else{
    analogWrite(M1_EN_PIN, (int)(-255.0*power));
    digitalWrite(M1_PH_PIN, LOW);
  }
}

void driveMotorTwo(float power){
  if(power > 1){
    power = 1;
  }else if(power < -1){
    power = -1;
  }

  if (power > 0){
    analogWrite(M2_EN_PIN, (int)(255.0f * power));
    digitalWrite(M2_PH_PIN, HIGH);
  }else{
    analogWrite(M2_EN_PIN, (int)(-255.0f*power));
    digitalWrite(M2_PH_PIN, LOW);
  }
}
