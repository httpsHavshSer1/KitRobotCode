#include <Arduino.h>


#include <Wire.h>
#include "SparkFunLSM6DS3.h"
#include <math.h>

#define SDA_PIN 21
#define SCL_PIN 22

LSM6DS3 imu(I2C_MODE, 0x6A);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("LSM6DS3TR-C IMU Test");

    // Initialize I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    // Initialize IMU
    int status = imu.begin();

    if (status != 0)
    {
        Serial.print("IMU initialization failed. Status: ");
        Serial.println(status);

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("IMU initialized successfully!");
    Serial.println();
}

void loop()
{
    // Read accelerometer in g
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY();
    float az = imu.readFloatAccelZ();

    // Read gyroscope in degrees/second
    float gx = imu.readFloatGyroX();
    float gy = imu.readFloatGyroY();
    float gz = imu.readFloatGyroZ();

    // Calculate roll and pitch from gravity
    float roll = atan2(ay, az) * 180.0 / PI;

    float pitch = atan2(
        -ax,
        sqrt(ay * ay + az * az)
    ) * 180.0 / PI;

    Serial.print("Accel [g]  X: ");
    Serial.print(ax, 3);
    Serial.print("  Y: ");
    Serial.print(ay, 3);
    Serial.print("  Z: ");
    Serial.println(az, 3);

    Serial.print("Gyro [dps] X: ");
    Serial.print(gx, 2);
    Serial.print("  Y: ");
    Serial.print(gy, 2);
    Serial.print("  Z: ");
    Serial.println(gz, 2);

    Serial.print("Orientation  Roll: ");
    Serial.print(roll, 2);
    Serial.print(" deg  Pitch: ");
    Serial.print(pitch, 2);
    Serial.println(" deg");

    Serial.println("--------------------------------");

    delay(100);
}
