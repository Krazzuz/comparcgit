# WiFi Controlled Line-Following Robot

A 4-wheel Arduino-based robotic car with WiFi control, featuring a web interface for both manual operation and autonomous line-following.

## Key Features

- **Wireless Control**: Connect to the robot's WiFi to control it from any web browser.
- **Web Interface**: A simple UI for starting/stopping autonomous mode, adjusting speed, and manual control.
- **Autonomous Mode**: The robot can follow a black line on a light-colored surface.
- **Junction Handling**: In autonomous mode, the robot randomly navigates T-junctions and crossroads.
- **4-Wheel Drive**: Provides robust movement with four DC motors.

## Hardware Requirements

- Arduino with WiFi (e.g., Arduino Uno WiFi Rev2)
- Adafruit Motor Shield V2
- 4x DC Motors
- 3x IR Proximity Sensors
- Robot chassis and wheels
- Power source (battery pack)

## Software and Libraries

The Arduino code requires the following libraries:

- `WiFiS3.h`: For WiFi connectivity.
- `Adafruit_MotorShield.h`: To control the DC motors.

You can install these from the Arduino IDE's Library Manager.

## Setup and Wiring

1. **Motor Shield**: Stack the motor shield on the Arduino.
2. **Motors**: Connect the motors to the shield's terminals (M1, M2, M3, M4).
   - `m1`: Right front
   - `m2`: Right back
   - `m3`: Left back
   - `m4`: Left front
3. **IR Sensors**: Connect the sensors to the following pins:
   - **Right Sensor**: Pin D2
   - **Left Sensor**: Pin D3
   - **Middle Sensor**: Pin D4
4. **Power**: Connect the battery pack to the motor shield's power terminals.

## How to Use

1. **Power On**: The robot will create a WiFi access point.
2. **Connect to WiFi**:
   - **SSID**: `MarvinMobile`
   - **Password**: `MarvinOS`
3. **Open Web Interface**: Navigate to `192.168.4.1` in your browser.
4. **Control the Robot**:
   - **Autonomous Mode**:
     - **START**: Begins line-following.
     - **STOP**: Halts the robot.
     - **Speed Slider**: Adjusts speed in real-time.
   - **Manual Control**:
     - Use the arrow buttons for manual driving.

## Code Overview

The `robot_code.ino` file includes:

- **Global Variables**: Defines pins, WiFi credentials, and motor shield objects.
- `setup()`: Initializes the serial communication, motor shield, pins, and web server.
- `loop()`: The main loop that calls `updateMotors()` and listens for web clients.
- `handleRequest()`: Parses incoming HTTP requests from the web interface.
- `updateMotors()`: Contains the logic for the autonomous line-following.
- `executeManualCommand()`: Handles manual control logic.
- `makePage()`: Generates the HTML, CSS, and JavaScript for the web interface.