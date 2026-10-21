# Highlanders 4499 Outreach Kit Robot Code

ESP32 test programs for the Highlanders 4499 Outreach Kit Robot. This is a PlatformIO project using the Arduino framework. Each test is a separate build environment, so only one test program is uploaded to an ESP32 at a time.

## Open the project in VS Code

1. Install [Visual Studio Code](https://code.visualstudio.com/) and the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode).
2. In VS Code, open the Command Palette and choose **Git: Clone**.
3. Paste `https://github.com/httpsHavshSer1/KitRobotCode.git` and choose a local folder.
4. Open the cloned **KitRobotCode** folder in VS Code. The folder containing `platformio.ini` must be the workspace root.
5. Allow PlatformIO to initialize the project. On the first build, it downloads the ESP32 platform and the libraries listed in `platformio.ini`.

The project targets an **ESP32 Dev Module** (`esp32dev`). If your outreach kit uses a different ESP32 board, update `board` in `platformio.ini` before uploading.

## Choose and upload one test

In the PlatformIO sidebar (alien-head icon), expand **Project Tasks**, then expand the environment you want. Under **General**, click **Build** to compile or **Upload** to send that test to the connected ESP32. The terminal output shows `Processing <environment name>` so you can confirm your selection.

| Environment | Source file | Purpose |
| --- | --- | --- |
| `control_board` | `src/RobotControlBoardTest.cpp` | Sweep the servo on GPIO 25 |
| `encoder` | `src/RobotEncoderTest.cpp` | Print encoder position over serial |
| `imu_native` | `src/RobotImuNativeTest.cpp` | Read the IMU directly over I2C |
| `imu_library` | `src/RobotIMUTest.cpp` | Read the IMU through the SparkFun library |
| `motor` | `src/RobotMotorTest.cpp` | Drive motors and servo with PestoLink |

The regular PlatformIO toolbar Upload button uses `motor`, the current default environment. To upload another test, use **Project Tasks → that environment → General → Upload**. Each upload replaces the program previously on the ESP32.

Open **Project Tasks → that environment → General → Monitor** for serial output. The monitor speed is 115200 baud.

## PestoLink motor control

Upload the `motor` environment, then open [PestoLink](https://pestol.ink/) and connect to the BLE device named **RobotMotorTest**. The first joystick drives forward/backward and steers the motors. Hold **button 0** to move the servo down or **button 1** to move it up on GPIO 25. Releasing both buttons holds its angle; pressing both at once also holds it. The servo starts at 90 degrees and moves one degree every 20 ms while a button is held, within 0–180 degrees.

The firmware stops both motors and stops moving the servo when Bluetooth disconnects or valid gamepad packets stop for 500 ms. The motor fault input on GPIO 36 must have an external pull-up; a low fault signal also stops both motors. Power the servo from a supply rated for its load and connect that supply's ground to the ESP32 ground.

PestoLink's [project page](https://github.com/AlfredoSystems/PestoLink-Online#browser-support) lists supported browser options, including iPhone guidance.

## Source layout

The five C++ files in `src` are independent test programs. PlatformIO's `build_src_filter` setting in `platformio.ini` selects exactly one for each environment, avoiding duplicate `setup()` and `loop()` definitions. External libraries are declared in `platformio.ini` and install automatically; there is no need to copy them into `include` or `lib`.
