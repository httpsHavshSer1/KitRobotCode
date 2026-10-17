#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define IMU_ADDRESS 0x6A

// Registers
#define WHO_AM_I       0x0F
#define CTRL1_XL       0x10
#define CTRL2_G        0x11
#define OUTX_L_G       0x22
#define OUTX_L_A       0x28

void writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t readRegister(uint8_t reg)
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)IMU_ADDRESS, (uint8_t)1);

    if (Wire.available())
        return Wire.read();

    return 0;
}

void readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)IMU_ADDRESS, length);

    for (uint8_t i = 0; i < length && Wire.available(); i++)
        buffer[i] = Wire.read();
}

int16_t combineBytes(uint8_t low, uint8_t high)
{
    return (int16_t)((high << 8) | low);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("LSM6DS3TR-C Test");

    uint8_t whoAmI = readRegister(WHO_AM_I);

    Serial.print("WHO_AM_I = 0x");
    Serial.println(whoAmI, HEX);

    if (whoAmI != 0x6A)
    {
        Serial.println("ERROR: Unexpected device!");
        while (true)
            delay(1000);
    }

    /*
     * CTRL1_XL = 0x50
     *
     * Accelerometer:
     * ODR = 104 Hz
     * Full scale = +/- 2 g
     */
    writeRegister(CTRL1_XL, 0x50);

    /*
     * CTRL2_G = 0x50
     *
     * Gyroscope:
     * ODR = 104 Hz
     * Full scale = +/- 245 dps
     */
    writeRegister(CTRL2_G, 0x50);

    delay(100);

    Serial.println("IMU configured.");
}

void loop()
{
    uint8_t accelData[6] = {};
    uint8_t gyroData[6] = {};

    readRegisters(OUTX_L_A, accelData, 6);
    readRegisters(OUTX_L_G, gyroData, 6);

    int16_t rawAx = combineBytes(accelData[0], accelData[1]);
    int16_t rawAy = combineBytes(accelData[2], accelData[3]);
    int16_t rawAz = combineBytes(accelData[4], accelData[5]);

    int16_t rawGx = combineBytes(gyroData[0], gyroData[1]);
    int16_t rawGy = combineBytes(gyroData[2], gyroData[3]);
    int16_t rawGz = combineBytes(gyroData[4], gyroData[5]);

    /*
     * +/- 2g accelerometer
     *
     * Sensitivity = 0.061 mg/LSB
     */
    float ax = rawAx * 0.000061f;
    float ay = rawAy * 0.000061f;
    float az = rawAz * 0.000061f;

    /*
     * +/- 245 dps gyroscope
     *
     * Sensitivity = 8.75 mdps/LSB
     */
    float gx = rawGx * 0.00875f;
    float gy = rawGy * 0.00875f;
    float gz = rawGz * 0.00875f;

    /*
     * Calculate orientation from gravity.
     *
     * These values are reliable when the device is
     * stationary or moving slowly.
     */
    float roll =
        atan2(ay, az) * 180.0f / PI;

    float pitch =
        atan2(
            -ax,
            sqrt(ay * ay + az * az)
        ) * 180.0f / PI;

    Serial.print("Accel [g]  ");
    Serial.print(ax, 3);
    Serial.print(", ");
    Serial.print(ay, 3);
    Serial.print(", ");
    Serial.println(az, 3);

    Serial.print("Gyro [dps] ");
    Serial.print(gx, 2);
    Serial.print(", ");
    Serial.print(gy, 2);
    Serial.print(", ");
    Serial.println(gz, 2);

    Serial.print("Roll: ");
    Serial.print(roll, 2);

    Serial.print("  Pitch: ");
    Serial.println(pitch, 2);

    Serial.println();

    delay(2000);
}