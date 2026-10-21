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
| `motor` | `src/RobotMotorTest.cpp` | Check the PestoLink Bluetooth connection and button 0 |

The regular PlatformIO toolbar Upload button uses `motor`, the current default environment. To upload another test, use **Project Tasks → that environment → General → Upload**. Each upload replaces the program previously on the ESP32.

Open **Project Tasks → that environment → General → Monitor** for serial output. The monitor speed is 115200 baud.

## PestoLink connection test

Upload the `motor` environment, then connect to **ESP32 Test** in PestoLink. This simple test uses the official `PestoLink-Receive` library. Press button 0; the Serial Monitor prints `Button A was pressed` once per press. It does not drive the motors or servo.

PestoLink's [project page](https://github.com/AlfredoSystems/PestoLink-Online#browser-support) lists supported browser options, including iPhone guidance.

If PestoLink stays on **Searching for robots**, check that the ESP32 is powered and that the `motor` upload succeeded. Turn on Bluetooth and allow Bluetooth permission for the app. If the Serial Monitor repeatedly shows `Running on single core variant of a chip, but app is built with multi-core support`, the selected board/framework does not match the chip and the program never reaches Bluetooth setup. That startup error must be resolved before testing PestoLink.

## Source layout

The five C++ files in `src` are independent test programs. PlatformIO's `build_src_filter` setting in `platformio.ini` selects exactly one for each environment, avoiding duplicate `setup()` and `loop()` definitions. External libraries are declared in `platformio.ini` and install automatically; there is no need to copy them into `include` or `lib`.
