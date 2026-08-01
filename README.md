# ESP32_Arduino_PlatformIO Remote-Controlled Car

This project utilized the ESP32Servo by Kevin Harrington and esp32-camera by Espressif Systems libraries. The esp32-camera library was too large for the ESP32 used thus only the utilized APIs in the library were included in the source code. The project also used WiFi and BluetoothSerial (Bluetooth Classic) modules that are built-in Espressif32 Platform with Arduino Framework.

This RC Car and its camera part can be controlled through an android phone via Bluetooth Classic. The RC Car features a camera, and users can take photos (viewed in app, via WiFi) as well as toggle the camera flash through the android app. Attached to the camera part is a servomotor to tilt it 180° horizontally. The RC Car is powered by 4 AA batteries.

The simple android app was developed in the MIT App Inventor. The components and code blocks used are in photos/.

Further implementation will allow for 2-way audio communication also via BT Classic, as well as will miniaturized the RC Car further using 1:32 to 1:64 car models instead.

*Materials- 

1. ESP32-CAM CH340 HW818 w/ OV2640 camera sensor

2. 2WD Car Chassis Kit w/ 2 motors and L298N Motor Driver

3. SG90 Servomotor w/ Tilt Bracket

4. 16V 1000uF External Electrolytic Capacitor for servomotor (to smoothen sudden I surge and prevent V drops)

*To upload the code to the board-

1. Enter the appropriate WiFi credentials then build the program.

2. Select the appropriate port, hold the FLASH button and when it starts to write, click the RST button (to reset the board into Firmware Download Mode).

3. When the upload succeeded, clik the RST button again (to reset the board into Execution Mode).

*To install android app- Download then install the apk from apk/ESP32_Arduino_RC-Car.apk.

*Other References- 

1. MIT App Inventor- https://youtu.be/qS4WtLu5wzE

2. Servomotor- https://www.youtube.com/shorts/G6hntRnA8fM