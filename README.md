# Arduino + ESP32-CAM Smart Robot Car

Beginner-friendly embedded robotics project based on an **Arduino Uno**, an **ESP32-CAM**, motors, a servo, an ultrasonic distance sensor, and line-tracking sensors.

The project combines two microcontrollers:

- **Arduino Uno** controls the physical movement of the robot, motors, servo, ultrasonic sensor, and line sensors.
- **ESP32-CAM** provides Wi-Fi connectivity, live camera streaming, a browser-based control page, and sends control commands to the Arduino side.

The repository contains both a simple robot-control version and a more advanced line-tracking version.

---

## What this robot can do

The robot can perform several different tasks:

- drive forward;
- drive backward;
- move left and right;
- move diagonally;
- rotate in place;
- stop immediately;
- move a servo left and right;
- use an ultrasonic sensor to measure distance;
- avoid obstacles automatically;
- follow an object using distance information;
- follow a black line using three tracking sensors;
- stream live video from the ESP32-CAM;
- show the video in a web browser;
- receive movement commands from buttons on the browser page;
- turn the ESP32-CAM light on and off;
- switch between several control modes from the web interface.

This makes the project useful as a beginner embedded-systems demonstration because it combines **inputs, outputs, sensors, actuators, communication, Wi-Fi, and real-time control** in one robot.

---

# Main idea of the project

The system can be viewed as two connected parts:

```text
                 PHONE / LAPTOP
                      |
                      | Wi-Fi
                      v
               +-------------+
               |  ESP32-CAM  |
               |             |
               | camera      |
               | web server  |
               | control UI  |
               +------+------+ 
                      |
                      | serial commands
                      v
               +-------------+
               | Arduino Uno |
               |             |
               | robot logic |
               +------+------+ 
                      |
          +-----------+-----------+
          |           |           |
          v           v           v
       Motors      Servo      Sensors
                              |     |
                              |     +--> line sensors
                              +--------> ultrasonic sensor
```

The ESP32-CAM is responsible mainly for **communication and video**, while the Arduino Uno is responsible mainly for **real-world hardware control**.

---

# Hardware

The project is designed around the following components.

## Main components

- Arduino Uno
- ESP32-CAM / AI Thinker camera module
- robot car chassis
- DC motors
- motor control board
- 74HCT595 shift register based motor control
- servo motor
- ultrasonic distance sensor such as HC-SR04
- three line-tracking sensors
- jumper wires
- suitable battery / power source
- USB cable for programming
- computer with Arduino IDE

Depending on the exact robot-car kit, some of these parts can already be integrated on the same control board.

---

# Arduino Uno pin configuration

The main Arduino sketch uses these pins:

| Device / signal | Arduino pin |
|---|---:|
| Servo signal | D9 |
| Motor PWM 1 | D5 |
| Motor PWM 2 | D6 |
| Shift register SHCP | D2 |
| Shift register EN | D7 |
| Shift register DATA | D8 |
| Shift register STCP | D4 |
| Ultrasonic TRIG | D12 |
| Ultrasonic ECHO | D13 |
| Left line sensor | A0 |
| Center line sensor | A1 |
| Right line sensor | A2 |

The servo starts at approximately **90 degrees**, which is the center position.

---

# Repository structure

```text
.
├── firmware/
│   ├── arduino_uno_robot/
│   │   └── arduino_uno_robot.ino
│   │
│   ├── esp32_camera_car/
│   │   ├── esp32_camera_car.ino
│   │   ├── app_httpd.cpp
│   │   └── camera_index.h
│   │
│   └── line_tracking_robot/
│       ├── FullTrackingRoboControl.ino
│       ├── init.h
│       ├── Motor.h
│       ├── ADC.h
│       └── supporting I2C / MPU6050 files
│
├── examples/
│   └── servo_test/
│       └── servo_test.ino
│
├── reference/
│   └── esp_http_server.h
│
├── .gitignore
├── SOURCE_NOTES.md
└── README.md
```

---

# 1. Arduino Uno robot firmware

Main file:

```text
firmware/arduino_uno_robot/arduino_uno_robot.ino
```

This sketch is responsible for controlling the robot hardware.

It initializes:

- the servo;
- motor-control pins;
- ultrasonic sensor pins;
- line-tracking sensor pins;
- serial communication.

Serial communication runs at:

```cpp
Serial.begin(115200);
```

The Arduino waits for incoming command packets and then selects the appropriate movement or operating mode.

---

## Robot movement commands

The code defines separate bit combinations for each movement.

The robot can perform:

| Movement | Meaning |
|---|---|
| `Forward` | move forward |
| `Backward` | move backward |
| `Turn_Left` | move / translate left |
| `Turn_Right` | move / translate right |
| `Top_Left` | diagonal forward-left |
| `Top_Right` | diagonal forward-right |
| `Bottom_Left` | diagonal backward-left |
| `Bottom_Right` | diagonal backward-right |
| `Clockwise` | rotate clockwise |
| `Contrarotate` | rotate counter-clockwise |
| `Stop` | stop all movement |

Motor movement is controlled by the function:

```cpp
Motor(direction, speed);
```

For example, manual forward motion is executed with a command similar to:

```cpp
Motor(Forward, 250);
```

The second argument controls motor speed through PWM.

---

# 2. Robot operating modes

The Arduino program contains four main operating modes.

```text
model 1 -> manual control
model 2 -> obstacle avoidance
model 3 -> following mode
model 4 -> line tracking
```

The variable `model_var` determines which mode is currently active.

---

## Mode 1 — Manual control

In manual mode, the robot executes commands received through serial communication.

Possible actions include:

- forward;
- backward;
- left;
- right;
- diagonal movement;
- rotation;
- stopping;
- servo left;
- servo right.

This is the main mode used when the robot is controlled from the ESP32-CAM web interface.

---

## Mode 2 — Obstacle avoidance

Obstacle-avoidance mode uses the ultrasonic sensor together with the servo.

The basic algorithm is:

```text
1. Look forward.
2. Measure the distance in front of the robot.
3. If the path is clear -> continue moving.
4. If an obstacle is close -> stop.
5. Turn the sensor to one side.
6. Measure distance.
7. Turn the sensor to the other side.
8. Measure distance again.
9. Compare both distances.
10. Choose the direction with more free space.
```

The program first places the servo around:

```text
90 degrees
```

which represents the center position.

If an obstacle is detected at approximately **25 cm or less**, the robot stops and scans both sides.

The servo positions used by the current code are approximately:

```text
10°   -> one side
90°   -> center
170°  -> other side
```

The ultrasonic sensor then measures the available space in each direction.

The robot can:

- reverse when both sides are blocked;
- rotate to escape;
- choose the side with more free space.

This is also the part of the project that gives the robot a "looking around" movement if the sensor or camera assembly is physically mounted on the servo.

---

# 3. Ultrasonic distance measurement

The project uses an ultrasonic distance sensor with:

```text
TRIG -> D12
ECHO -> D13
```

The Arduino sends a short ultrasonic pulse and measures how long the echo takes to return.

The main distance function is:

```cpp
SR04(Trig_PIN, Echo_PIN);
```

The measured distance is then used by the obstacle-avoidance and following modes.

Conceptually:

```text
Arduino
   |
   | trigger pulse
   v
Ultrasonic sensor  ---> sound ---> obstacle
Ultrasonic sensor  <--- echo  <--- obstacle
   |
   v
Arduino calculates distance
```

The farther the obstacle is, the longer the echo needs to return.

---

# 4. Servo control

The servo is connected to:

```text
D9
```

and initialized with the Arduino Servo library:

```cpp
#include <Servo.h>
```

The servo is attached using:

```cpp
MOTORservo.attach(MOTOR_PIN);
```

The main sketch includes commands for turning the servo left and right.

Relevant functions include:

```cpp
motorleft();
motorright();
```

The obstacle-avoidance code also moves the servo automatically to approximately:

```text
10°
90°
170°
```

This can be used to rotate an ultrasonic sensor, and if the physical camera mount is attached to the same moving mechanism, it can also create a rotating camera effect.

Important: the code can move only the hardware that is physically attached to the servo. If the camera is mounted separately, moving the servo will not move the camera.

---

# 5. Following mode

The sketch also contains a following mode:

```cpp
model3_func();
```

This mode uses ultrasonic distance measurements to react to an object in front of the robot.

The intention of this mode is different from obstacle avoidance:

- obstacle avoidance tries not to hit an object;
- following mode tries to keep the robot at an appropriate distance from an object.

This provides another simple autonomous behavior without requiring computer vision.

---

# 6. Line-tracking mode

The basic robot sketch also contains:

```cpp
model4_func();
```

This is the line-tracking mode.

Three analog line sensors are used:

```text
Left sensor   -> A0
Center sensor -> A1
Right sensor  -> A2
```

The current sketch uses a black-line threshold around:

```cpp
int Black_Line = 400;
```

The robot compares sensor readings to decide whether it should:

- continue straight;
- correct left;
- correct right;
- turn more strongly;
- stop or recover if the line is lost.

Conceptually:

```text
     LEFT   CENTER   RIGHT
       |      |       |
       v      v       v
    +-------------------+
    |    Arduino Uno    |
    +---------+---------+
              |
              v
         motor control
              |
              v
       robot follows line
```

---

# 7. Advanced line-tracking firmware

A more advanced version is stored in:

```text
firmware/line_tracking_robot/
```

Main file:

```text
FullTrackingRoboControl.ino
```

This version is separated into multiple helper files rather than keeping all logic in one sketch.

The folder includes files related to:

- motor control;
- ADC / sensor reading;
- initialization;
- filtering;
- I2C communication;
- MPU6050 support;
- more detailed movement correction.

This version is useful if the simple line-tracking mode is not stable enough or if the project is being extended with more advanced control logic.

For a beginner presentation, it is usually easier to explain the simpler Arduino sketch first and mention this directory as the more advanced version.

---

# 8. ESP32-CAM firmware

Main file:

```text
firmware/esp32_camera_car/esp32_camera_car.ino
```

This sketch handles:

- ESP32 initialization;
- camera initialization;
- Wi-Fi connection;
- starting the web server;
- printing the robot's IP address;
- camera streaming.

The current configuration uses:

```cpp
#define CAMERA_MODEL_AI_THINKER
```

so the pin mapping is configured for an **AI Thinker ESP32-CAM**.

---

# 9. Wi-Fi configuration

Before uploading the ESP32 code, replace:

```cpp
const char *ssid = "YOUR_WIFI_NAME";
const char *password = "YOUR_WIFI_PASSWORD";
```

with your local Wi-Fi credentials.

Example:

```cpp
const char *ssid = "MyHomeWiFi";
const char *password = "my-password";
```

Do **not** commit your real Wi-Fi password to a public GitHub repository.

The placeholders in this repository are intentional.

---

# 10. Camera initialization

The ESP32 initializes the camera using `camera_config_t`.

The project configures:

- camera data pins;
- XCLK;
- pixel clock;
- VSYNC;
- HREF;
- SCCB communication pins;
- JPEG output;
- frame size;
- JPEG quality;
- frame buffer count.

The camera uses:

```cpp
config.pixel_format = PIXFORMAT_JPEG;
```

If PSRAM is available, the program uses a larger frame configuration before later selecting a practical streaming resolution.

After initialization, the ESP32 connects to Wi-Fi and starts the camera server.

---

# 11. Opening the camera page

The ESP32 uses serial speed:

```text
115200 baud
```

After successful Wi-Fi connection, Serial Monitor prints a message similar to:

```text
WiFi connected
Camera Ready! Use 'http://192.168.x.x' to connect
```

Open the shown IP address in a browser on a device connected to the same network.

Example:

```text
http://192.168.1.105
```

The exact IP address depends on your router and network.

---

# 12. Browser control interface

The web server code is implemented mainly in:

```text
firmware/esp32_camera_car/app_httpd.cpp
```

The generated browser page includes a live camera image and control buttons.

The interface currently contains controls for:

### Movement

- `LeftUp`
- `Forward`
- `RightUp`
- `Left`
- `Clockwise`
- `Right`
- `LeftDown`
- `Backward`
- `RightDown`
- `Stop`

### Robot modes

- `model1`
- `model2`
- `model3`
- `model4`

### Servo

- `MotorLeft`
- `MotorRight`

### ESP32-CAM light

- `Light ON`
- `Light OFF`

This allows the robot to be controlled from a phone or laptop without a separate desktop application.

---

# 13. HTTP control endpoints

The web server registers several URL handlers.

Examples include:

```text
/go
/back
/stop
/left
/right
/leftup
/leftdown
/rightup
/rightdown
/clockwise
/model1
/model2
/model3
/model4
/motorleft
/motorright
/ledon
/ledoff
/capture
/stream
/status
/control
```

When a browser button is pressed, JavaScript sends an HTTP request to one of these endpoints.

For example:

```text
Forward button
      |
      v
GET /go
      |
      v
ESP32 handler
      |
      v
command sent to robot
```

---

# 14. Live camera streaming

The ESP32 camera server provides a stream endpoint:

```text
/stream
```

The server repeatedly:

1. captures a camera frame;
2. ensures the image is in JPEG format;
3. sends the frame through the HTTP connection;
4. captures the next frame;
5. repeats the process.

This creates a continuous live camera feed in the browser.

The project also supports a single-image capture handler through:

```text
/capture
```

---

# 15. Camera light

The ESP32-CAM code defines the light pin as:

```cpp
extern int gpLed = 4;
```

The browser page contains buttons for:

```text
Light ON
Light OFF
```

This can be useful when the robot is driving in a darker area.

---

# 16. Communication between ESP32-CAM and Arduino Uno

The ESP32 side uses small command packets to represent actions.

The Arduino side receives commands with:

```cpp
RXpack_func();
```

The Arduino then interprets the received command and changes either:

- movement direction;
- robot mode;
- servo position.

The conceptual data flow is:

```text
Browser button
     |
     v
ESP32 HTTP handler
     |
     v
command packet
     |
     v
Arduino Serial
     |
     v
RXpack_func()
     |
     v
movement / mode / servo action
```

---

# 17. Servo test example

Before running the complete robot, it is useful to verify the servo separately.

Open:

```text
examples/servo_test/servo_test.ino
```

This small sketch is intended only for checking whether:

- the servo is powered;
- the signal wire is connected correctly;
- pin D9 works;
- the servo can move to the expected angle.

Testing individual components first makes debugging the complete robot much easier.

---

# 18. How to upload the Arduino Uno code

## Step 1 — Install Arduino IDE

Install Arduino IDE on your computer.

## Step 2 — Connect Arduino Uno

Connect the Arduino Uno using USB.

## Step 3 — Open the sketch

Open:

```text
firmware/arduino_uno_robot/arduino_uno_robot.ino
```

## Step 4 — Select the board

In Arduino IDE choose:

```text
Tools -> Board -> Arduino AVR Boards -> Arduino Uno
```

## Step 5 — Select the port

Choose the COM port corresponding to the Arduino.

## Step 6 — Compile

Click **Verify**.

## Step 7 — Upload

Click **Upload**.

## Step 8 — Serial Monitor

If needed, open Serial Monitor and use:

```text
115200 baud
```

---

# 19. How to upload the ESP32-CAM code

ESP32-CAM programming is slightly different from Arduino Uno programming.

## Step 1 — Install ESP32 board support

Make sure Arduino IDE has ESP32 board support installed.

## Step 2 — Open the ESP32 project

Open:

```text
firmware/esp32_camera_car/esp32_camera_car.ino
```

Keep these files in the same sketch/project folder:

```text
esp32_camera_car.ino
app_httpd.cpp
camera_index.h
```

## Step 3 — Enter Wi-Fi credentials

Replace the placeholders locally.

## Step 4 — Select the correct ESP32-CAM board

The code is configured for:

```text
AI Thinker ESP32-CAM
```

## Step 5 — Upload

Upload using the programming method required by your ESP32-CAM board / adapter.

## Step 6 — Open Serial Monitor

Use:

```text
115200 baud
```

## Step 7 — Find the IP address

Wait until the ESP32 prints its local IP address.

## Step 8 — Open the browser interface

Open the address on a phone or laptop connected to the same Wi-Fi network.

---

# 20. Recommended first-time testing order

Do not test everything at the same time on the first attempt.

A good debugging order is:

### Test 1 — Arduino power

Verify that the Arduino starts correctly.

### Test 2 — Servo

Use:

```text
examples/servo_test/servo_test.ino
```

Confirm that the servo moves.

### Test 3 — Motors

Upload the main Arduino sketch and check basic movement.

### Test 4 — Ultrasonic sensor

Use Serial Monitor and check whether distance values change when an object is moved in front of the sensor.

### Test 5 — Obstacle avoidance

Activate model 2 and test with a large object placed in front of the robot.

### Test 6 — Line sensors

Place the robot above a black line and verify the sensor values.

### Test 7 — ESP32-CAM

Upload the ESP32 sketch and verify the live camera feed.

### Test 8 — Browser controls

Test `Forward`, `Backward`, `Stop`, and servo controls before trying more advanced modes.

---

# 21. Simple demo scenario

A clear project demonstration can be done in this order:

1. Power on the robot.
2. Open the ESP32-CAM IP address on a phone.
3. Show the live camera image.
4. Press **Forward** and drive the robot.
5. Press **Left** / **Right**.
6. Press **MotorLeft** / **MotorRight** to demonstrate servo movement.
7. Turn the camera light on and off.
8. Activate obstacle-avoidance mode.
9. Put an object in front of the robot.
10. Show that the robot stops, scans the sides and chooses a direction.
11. Activate line-tracking mode.
12. Put the robot on a black line and show autonomous movement.

This demonstrates both manual and autonomous embedded control.

---

# 22. How to explain the project in a presentation

A simple explanation is:

> The project is a smart robot car controlled by an Arduino Uno and an ESP32-CAM. The Arduino controls the motors, servo, ultrasonic sensor, and line sensors. The ESP32-CAM connects to Wi-Fi, streams live video, and provides a web page where the user can control the robot. The robot also has autonomous modes such as obstacle avoidance and line tracking.

The important embedded-system idea is:

```text
INPUT -> PROCESSING -> OUTPUT
```

Examples from this project:

```text
Ultrasonic sensor
       |
       v
Arduino measures distance
       |
       v
Motors change direction
```

and:

```text
Phone button
    |
    v
ESP32 receives HTTP request
    |
    v
Arduino receives command
    |
    v
Motors move robot
```

---

# 23. Embedded concepts demonstrated

This project demonstrates several important embedded-programming concepts.

## Digital output

Used for control signals and motor-control hardware.

## PWM

Used to control motor power / speed.

## Analog input

Used to read the line-tracking sensors.

## Timing

Used by the ultrasonic sensor to calculate distance.

## Serial communication

Used for command communication and debugging.

## Servo control

Used to rotate the sensor or a mounted camera assembly.

## Wi-Fi networking

Used by ESP32-CAM to create the browser-based remote-control interface.

## HTTP server

Used to receive commands such as `/go`, `/stop`, and `/motorleft`.

## Real-time decision making

Used by obstacle avoidance and line tracking.

---

# 24. Beginner-friendly explanation of the software loop

The Arduino repeatedly executes:

```cpp
void loop()
```

Inside the loop it:

1. checks whether a new command was received;
2. checks the selected mode;
3. executes the corresponding function.

Conceptually:

```text
loop
 |
 +--> receive command
 |
 +--> check current mode
        |
        +--> manual
        +--> obstacle avoidance
        +--> follow
        +--> line tracking
 |
 +--> repeat
```

This loop runs continuously while the Arduino is powered.

---

# 25. Possible future improvements

The current project already contains several functions, but it can be extended further.

Possible future improvements include:

- smoother servo motion;
- automatic camera scanning;
- keeping the camera facing approximately forward while the robot turns;
- proportional / PID line tracking;
- adjustable motor speed from the browser;
- battery-voltage monitoring;
- distance display on the web interface;
- mobile-friendly redesign of the control page;
- object detection using a more powerful processor;
- automatic recording or snapshots;
- additional safety stop logic;
- displaying the active robot mode in the web interface.

These are not required for the current project but are natural directions for further development.

---

# 26. Important safety and hardware notes

- Do not power motors directly from Arduino GPIO pins.
- Use the motor driver / control board designed for the robot.
- Make sure the Arduino, ESP32, and motor system have a proper common ground where required by the hardware design.
- Be careful with battery polarity.
- Do not change wiring while the robot is powered unless necessary and safe.
- Place the robot on a stand or lift the wheels during the first motor test so unexpected movement does not make it fall from a table.
- Test autonomous modes at low speed first.

---

# 27. Security note

The repository intentionally does not contain real Wi-Fi credentials.

Do not commit code like:

```cpp
const char *ssid = "MyActualWiFi";
const char *password = "MyActualPassword";
```

to a public repository.

Instead, keep the placeholders in GitHub and change them only in your local copy before uploading to the ESP32-CAM.

---

# 28. About `esp_http_server.h`

The file:

```text
reference/esp_http_server.h
```

is included only as a reference because it was present in the supplied project materials.

Normally, the ESP32 framework already provides the HTTP server header, so this reference copy does not usually need to be copied into the Arduino sketch folder.

---

# 29. Main source files summary

| File | Purpose |
|---|---|
| `arduino_uno_robot.ino` | Main Arduino robot controller |
| `esp32_camera_car.ino` | ESP32-CAM initialization and Wi-Fi connection |
| `app_httpd.cpp` | Web server, camera stream, web controls, command handlers |
| `camera_index.h` | Camera web-page data |
| `FullTrackingRoboControl.ino` | More advanced line-tracking implementation |
| `servo_test.ino` | Simple servo hardware test |
| `esp_http_server.h` | Reference ESP HTTP server header |

---

# 30. Project summary

This repository combines a beginner-friendly embedded robot with several useful autonomous and remote-control features.

The **Arduino Uno** handles the physical robot:

```text
motors + servo + ultrasonic sensor + line sensors
```

The **ESP32-CAM** handles:

```text
camera + Wi-Fi + browser interface + remote commands
```

Together they create a robot that can be manually driven from a browser, stream live video, turn its servo, avoid obstacles, follow an object by distance, and follow a line.

The project is suitable for learning and demonstrating:

- Arduino programming;
- embedded C/C++;
- sensors;
- motor control;
- servo control;
- serial communication;
- Wi-Fi;
- simple web control;
- autonomous robot behavior.
