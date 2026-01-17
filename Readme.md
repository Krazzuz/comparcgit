# WiFi Controlled Line-Following Robot

This project is an Arduino-based, 4-wheel robot car that can be controlled over WiFi. It features a web interface for manual control and an autonomous line-following mode.

## Features

- **Wireless Control:** Connect to the robot's WiFi access point to control it from any web browser.
- **Web Interface:** A simple and responsive web UI with controls for:
    - Starting and stopping the autonomous mode.
    - Adjusting motor speed.
    - Manual directional control (forward, backward, left, right).
- **Autonomous Mode:** The robot can autonomously follow a black line on a light-colored surface.
- **Junction Handling:** In autonomous mode, when the robot encounters a T-junction or a crossroad, it will randomly choose to turn left, turn right, or continue straight.
- **4-Wheel Drive:** Uses four DC motors, providing robust movement.

## Hardware Requirements

-   Arduino with WiFi capabilities (e.g., Arduino Uno WiFi Rev2, or any Arduino compatible with the WiFiS3 library).
-   Adafruit Motor Shield V2.
-   4x DC Motors.
-   3x IR (Infrared) Proximity Sensors for line following.
-   A robot chassis and wheels.
-   Power source (battery pack) for the Arduino and motors.

## Software & Libraries

The Arduino code (`robot_code/robot_code.ino`) requires the following libraries:

-   [`WiFiS3.h`](https://www.arduino.cc/en/Reference/WiFiS3): For WiFi connectivity.
-   [`Adafruit_MotorShield.h`](https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library): To control the DC motors with the Adafruit shield.

You can install these libraries through the Arduino IDE's Library Manager.

## Setup and Wiring

1.  **Motor Shield:** Stack the Adafruit Motor Shield on top of the Arduino.
2.  **Motors:** Connect the four DC motors to the motor shield's terminals (M1, M2, M3, M4).
    -   `m1`: Right front motor
    -   `m2`: Right back motor
    -   `m3`: Left back motor
    -   `m4`: Left front motor
3.  **IR Sensors:** Connect the IR sensors to the following digital pins on the Arduino:
    -   **Right Sensor:** Pin D2
    -   **Left Sensor:** Pin D3
    -   **Middle Sensor:** Pin D4
4.  **Power:** Connect the battery pack to the motor shield's power terminals, ensuring correct polarity.

## How to Use

1.  **Power On:** Turn on the robot. It will create a WiFi access point.
2.  **Connect to WiFi:** On your computer or smartphone, connect to the WiFi network with the following credentials:
    -   **SSID:** `MotorControl`
    -   **Password:** `12345678`
3.  **Open Web Interface:** Open a web browser and navigate to the robot's IP address, which is typically `192.168.4.1`.
4.  **Control the Robot:**
    -   **Autonomous Mode:**
        -   Click the **START** button to begin line-following. The manual controls will be disabled.
        -   Click the **STOP** button to halt the robot and re-enable manual controls.
        -   Use the **Speed** slider to adjust the robot's speed in real-time.
    -   **Manual Control:**
        -   When the robot is stopped, use the arrow buttons (⬆️, ⬇️, ⬅️, ➡️) to drive the robot manually.

## Code Overview

The `robot_code.ino` file is structured as follows:

-   **Global Variables:** Defines pins for IR sensors, WiFi credentials, and motor shield objects.
-   `setup()`: Initializes serial communication, motor shield, pins, and starts the WiFi access point and web server.
-   `loop()`: This is the main program loop. It continuously calls `updateMotors()` to handle the robot's movement logic and listens for incoming web clients.
-   `handleRequest()`: Parses incoming HTTP requests from the web interface to toggle autonomous mode, set speed, or trigger manual movements.
-   `updateMotors()`: Contains the core logic for the autonomous line-following mode. It reads the IR sensors and controls the motors to follow the line, correct its path, or navigate junctions.
-   `executeManualCommand()`: Handles the logic for moving the robot based on manual control commands.
-   `makePage()`: Generates the HTML, CSS, and JavaScript for the web control interface.

## Future Improvements

-   **Obstacle Avoidance:** Integrate ultrasonic or other distance sensors to detect and avoid obstacles.
-   **More Advanced Navigation:** Implement more sophisticated algorithms for navigating complex mazes or paths.
-   **Camera Feed:** Add a camera to stream live video to the web interface.
-   **Improved UI:** Enhance the web interface with more features and a more polished design.