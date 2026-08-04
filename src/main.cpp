// Sources- 
// BLE (e.g. GATT [hierarchy...]) vs BT Classic (e.g. SPP)
// https://youtu.be/P0aqbD9umDE, https://youtu.be/wkO-ytWVvC0
// https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE
// MIT App Inventor- https://youtu.be/qS4WtLu5wzE
// Servomotor- https://www.youtube.com/shorts/G6hntRnA8fM

// ESP32-CAM CH340 HW-818, AI Thinker ESP32-CAM, ESP32-S
// OV2640 Camera Sensor
// Hold Flash... > Upload > Connecting... (Click RST) > Writing @ 0x00... (Release Flash)
// Click RST to start program...

// millis()- returns #ms passed since MCU started running program (up to ~49days)
// delay()- blocking, millis()- non-blocking

// PSRAM, DRAM, IRAM?

// <>- built-in (Arduino Framework, Espressif32 Platform), ""- 3rd-party
#include <Arduino.h>
#include <WiFi.h>
// #include <LittleFS.h>  // 
#include <BluetoothSerial.h>  // BT Classic (Serial Port Profile / SPP, like UART)
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <BLECharacteristic.h>
// #include <BLE2902.h>
#include "ESP32Servo.h"
// #include "CameraController.h"

#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
// #include "app_httpd.cpp"
#include "camera_pins.h"
#include <WebServer.h>

// Pins
const int IN1 = 12;   // Motor A direction pin 1
const int IN2 = 13;   // Motor A direction pin 2
const int IN3 = 14;   // Motor B direction pin 1
const int IN4 = 15;   // Motor B direction pin 2
// const int ENA = 2;    // PWM for Motor A
// const int ENB = 16;   // PWM for Motor B
// #define LED_PIN 2  // or 32/33/4/3
const int FLASH_PIN = 4;
bool isFlash = false;

// PWM channels for ESP32
// const int pwmChannelA = 0;  // 0-15
// const int pwmChannelB = 1;
// const int pwmFreq = 15000;
// const int pwmResolution = 8;  // Bits, 0–255

// WiFi- enter credentials
const char* SSID = "";  // Init pointer
const char* PASSWORD = "";
// WiFiServer Server(80);  // Port 80?

// Bluetooth
BluetoothSerial SerialBT;

// Servomotor
Servo MyServo;
const int SERVO_PIN = 2;
int angle = 90;
const int minPulseWidth = 500;  // 0.5ms
const int maxPulseWidth = 2400;  // 2.5ms
int targetAngle = 90;
int tiltDir = 0;  // 0- stop, -1- left, 1- right
unsigned long lastMove = 0;
const int ROTATE_SPEED = 20;  // ms per step

// Camera
// CameraController cam;
WebServer Server(80);  // Initializes web server obj... listen to incoming HTTP web reqs on port80
// void startCameraServer();
// void setupLedFlash(int pin);
// void handleJPGStream();
const int DELAY_CAMERA = 60;

// Helper funcs
bool setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  // config.frame_size = FRAMESIZE_UXGA;
  // config.frame_size = FRAMESIZE_QVGA;  // 320x240
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  // config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;  // 0-63 (higher-worst), 10/12/15/20/35/30/35
  config.fb_count = 1;
  // PSRAM, DRAM, IRAM?

  // Better settings
  config.frame_size = FRAMESIZE_UXGA;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed 0x%x\n", err);
    return false;
    // delay(3000);
    // ESP.restart();
  }

  // // Reduce final frame size further for stability
  sensor_t * s = esp_camera_sensor_get();
  // s->set_framesize(s, FRAMESIZE_QVGA);  // Fastest + least IRAM
  if (s) {
    // s->set_framesize(s, FRAMESIZE_QVGA);
    // reduce features that increase CPU
    // s->set_gain_ctrl(s, 0);     // gain control off
    // s->set_exposure_ctrl(s, 0); // exposure off (auto exposure costs cycles)
    // s->set_whitebal(s, 0);      // white balance off
    s->set_wb_mode(s, 3);       // 0:sunny, 1:cloudy, 2:office, 3:home
    s->set_special_effect(s, 0);

    // Better settings
    // Other settings- brightness, contrast, saturation [-2, 2], sharpness [0, 3]
    s->set_framesize(s, FRAMESIZE_UXGA);
    s->set_aec2(s, 1);  // Auto Exposure improved
    s->set_exposure_ctrl(s, 1); 
    s->set_gain_ctrl(s, 1);
    s->set_whitebal(s, 1);
    // s->set_blc(s, 1);  // Backlight Compensation

    s->set_brightness(s, 1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 0);
  }

// Setup LED FLash if LED pin is defined in camera_pins.h
// #if defined(LED_GPIO_NUM)
//   setupLedFlash(LED_GPIO_NUM);
// #endif

  return true;
}

// HTML for online camera...- typical structure in C++ notes...
// Stored in progmem/flash instead of ram
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 RC Car Stream</title>
</head>

<body style="padding: 50px; margin:0; background:black;">
  <img src="/stream" width="100%" height="auto" />
</body>
</html>
)rawliteral";
// rawliteral- delimiter name, 

void handleStream() {
  WiFiClient Client = Server.client();
  if (!Client) return;

  Client.println("HTTP/1.1 200 OK");
  Client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  Client.println("Cache-Control: no-cache\n");

  while (Client.connected()){
    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) break;

    Client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
    Client.write(fb->buf, fb->len);
    Client.write("\r\n");

    esp_camera_fb_return(fb);
    delay(DELAY_CAMERA);
  }
}

void handleCapture() {
  WiFiClient Client = Server.client();
  camera_fb_t * fb = esp_camera_fb_get();

  if(!fb){
    Server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  Server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  Server.sendHeader("Pragma", "no-cache");
  Server.sendHeader("Expires", "0");
  Server.setContentLength(fb->len);
  Server.send(200, "image/jpeg", "");

  if (Client) {
    Client.write(fb->buf, fb->len);
  }
  esp_camera_fb_return(fb);
}

void handleRoot() {
  // Server.send_P(200, "text/html", index_html);
  handleCapture();
}

void handleNotFound() {
  Server.send(404, "text/plain", "Not Found");
}

void startCameraServer() {
  Server.on("/", HTTP_GET, handleRoot);
  Server.on("/stream", HTTP_GET, handleStream);
  Server.on("/capture", HTTP_GET, handleCapture);
  Server.onNotFound(handleNotFound);
  Server.begin();
  Serial.println("Camera Server started");
}

void forward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // ledcWrite(pwmChannelA, speed);
  // ledcWrite(pwmChannelB, speed);
  Serial.println("Forward");
}

void backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("Backward");
}

void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("Left");
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("Right");
}

void stopCar() {
  // ledcWrite(pwmChannelA, 0);
  // ledcWrite(pwmChannelB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("Stop");
}

// void tiltLeft() {
//   if (angle > 0 && angle <= 180) {
//     angle--;
//     MyServo.write(angle);
//     delay(20);
//   }
// }

// void tiltRight() {
//   if (angle < 180 && angle >= 0) {
//     angle++;
//     MyServo.write(angle);
//     delay(20);
//   }
// }

// void tiltCenter() {
//   while (angle != 90) {
//     if (angle < 90 && angle >= 0) tiltRight();
//     if (angle > 90 && angle <= 180) tiltLeft();
//   }
// }

void flash() {
  if (isFlash == false) digitalWrite(FLASH_PIN, LOW);
  else if (isFlash == true) digitalWrite(FLASH_PIN, HIGH);
}

void setup() {
  // Direction pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(FLASH_PIN, OUTPUT);

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  delay(500);

  // Setup PWM channels
  // ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  // ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  // ledcSetup(pwmChannelLed, pwmFreq2, pwmResolution);

  // Attach PWM pins
  // ledcAttachPin(ENA, pwmChannelA);
  // ledcAttachPin(ENB, pwmChannelB);
  // ledcAttachPin(FLASH_LED, pwmChannelLed);

  // Bluetooth
  SerialBT.begin("ESP32RC");
  Serial.println("Connecting to Bluetooth");
  delay(500);

  // Servo
  MyServo.attach(SERVO_PIN, minPulseWidth, maxPulseWidth);
  MyServo.setPeriodHertz(50);  // 50Hz
  MyServo.write(angle);
  delay(500);

  // WiFi
  WiFi.begin(SSID, PASSWORD);
  // WiFi.setSleep(false);
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to WiFi: ");
  Serial.println(WiFi.localIP());
  // Serial.print(WiFi.localIP().toString());
  // Server.begin();
  delay(500);

  // Init cam & start server
  // cam.begin();
  setupCamera();
  startCameraServer();

  Serial.println("Start");
  // ledcWrite(pwmChannelLed, 10);
  // delay(1000);
  // ledcWrite(pwmChannelLed, 0);
}

void loop() {
  // WiFiClient client = server.available();
  // if (!client) return;
  
  // String req = client.readStringUntil('\r');
  // client.flush();

  // // Process reqs from web page
  // if (req.indexOf("/forward") != -1) { forward(255); }
  // if (req.indexOf("/backward") != -1) { backward(255); }
  // if (req.indexOf("/left") != -1) { left(255); }
  // if (req.indexOf("/right") != -1) { right(255); }
  // if (req.indexOf("/stopCar") != -1) { stopCar(); }

  // // Send the webpage
  // client.println("HTTP/1.1 200 OK");
  // client.println("Content-type:text/html");
  // client.println("Access-Control-Allow-Origin: *");
  // client.println();
  // client.println("OK");

  Server.handleClient();

  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.print("Clicked: ");
    Serial.println(cmd);

    // 
    if(cmd == 'F') forward(255);
    else if(cmd == 'B') backward();
    else if(cmd == 'L') left();
    else if(cmd == 'R') right();
    else if(cmd == 'S') stopCar();

    if(cmd == '<') { tiltDir = -1; }
    else if(cmd == '>') { tiltDir = 1; }
    else if(cmd == 'C') { tiltDir = 0; targetAngle = 90; }
    else if(cmd == '.') { tiltDir = 0; targetAngle = angle; }

    if(cmd == ',') {
      if (isFlash == true) { isFlash = false; }
      else if (isFlash == false) { isFlash = true; }
      // isFlash = !isFlash;
      flash();
    }
  }

  if (millis() - lastMove >= ROTATE_SPEED) {
    if (tiltDir == -1 && angle > 0 && angle <= 180) { angle--; Serial.println("Tilt Left"); }  // To left
    else if (tiltDir == 1 && angle < 180 && angle >= 0) { angle++; Serial.println("Tilt Right"); }
    else if (tiltDir == 0) {
      if (angle < targetAngle) { angle++; Serial.println("Centering"); }
      else if (angle > targetAngle) { angle--; Serial.println("Centering"); }
    }
    MyServo.write(angle);
    lastMove = millis();
  }

  // Cam
  // cam.update();
}