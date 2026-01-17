#include <WiFiS3.h>
#include <Adafruit_MotorShield.h>
#include <stdlib.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *m1, *m2, *m3, *m4;

const int IR_SENSOR_D2 = 2;  // Right sensor
const int IR_SENSOR_D3 = 3;  // Left sensor
const int IR_SENSOR_MIDDLE = 4; // Middle sensor

const char* ssid = "MotorControl";
const char* password = "12345678";

unsigned long leftSensorIgnoreUntil = 0;
unsigned long rightSensorIgnoreUntil = 0;

WiFiServer server(80);

// Car state
bool motorsEnabled = false; // Initially stopped
int motorSpeed = 100;       // Default speed

String makePage() {
  String buttonText = motorsEnabled ? "STOP" : "START";
  String buttonAction = motorsEnabled ? "/stop" : "/start";
  String manualControllerStyle = motorsEnabled ? "style='opacity: 0.5; pointer-events: none;'" : "";

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background: #f0f0f0; }";
  html += "h1 { color: #333; }";
  html += ".container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }";
  html += ".btn { padding: 15px 40px; font-size: 18px; border: none; border-radius: 5px; cursor: pointer; text-decoration: none; display: inline-block; margin: 20px 0; }";
  html += ".btn-start { background: #4CAF50; color: white; }";
  html += ".btn-stop { background: #f44336; color: white; }";
  html += ".slider-container { margin: 30px 0; }";
  html += "label { display: block; margin-bottom: 10px; font-size: 16px; color: #555;}";
  html += "input[type=range] { -webkit-appearance: none; width: 80%; height: 15px; background: #ddd; outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s; border-radius: 5px; }";
  html += "input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 25px; height: 25px; background: #4CAF50; cursor: pointer; border-radius: 50%; }";
  html += "input[type=range]::-moz-range-thumb { width: 25px; height: 25px; background: #4CAF50; cursor: pointer; border-radius: 50%; }";
  // Controller styles
  html += ".controller { margin-top: 30px; padding-top: 20px; border-top: 1px solid #eee; transition: opacity 0.3s; }";
  html += ".controller h2 { margin: 0 0 15px 0; font-size: 18px; color: #555; }";
  html += ".controller table { margin: 0 auto; }";
  html += ".ctrl-btn { display: block; width: 50px; height: 50px; line-height: 50px; text-align: center; font-size: 24px; text-decoration: none; background: #ddd; color: #333; border-radius: 8px; border: 2px solid #ccc; }";
  html += ".ctrl-btn:hover { background: #ccc; }";
  html += "</style>";
  html += "<script>";
  html += "function updateSpeed(value) {";
  html += "  document.getElementById('speedValue').innerText = value;";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/set-speed?value=' + value, true);";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Car Control</h1>";
  html += "<h3>Autonomous Mode</h3>";

  html += "<div class='slider-container'>";
  html += "<label for='speed'>Speed: <span id='speedValue'>" + String(motorSpeed) + "</span></label>";
  html += "<input type='range' min='0' max='255' value='" + String(motorSpeed) + "' class='slider' id='speed' oninput='updateSpeed(this.value)'>";
  html += "</div>";

  html += "<a href='" + buttonAction + "' class='btn " + (motorsEnabled ? "btn-stop" : "btn-start") + "'>" + buttonText + "</a>";

  html += "<div class='controller' " + manualControllerStyle + ">";
  html += "<h2>Manual Control</h2>";
  html += "<table>";
  html += "<tr><td></td><td><a href='/manual?dir=fwd' class='ctrl-btn'>&uarr;</a></td><td></td></tr>";
  html += "<tr><td><a href='/manual?dir=lft' class='ctrl-btn'>&larr;</a></td><td></td><td><a href='/manual?dir=rgt' class='ctrl-btn'>&rarr;</a></td></tr>";
  html += "<tr><td></td><td><a href='/manual?dir=bwd' class='ctrl-btn'>&darr;</a></td><td></td></tr>";
  html += "</table>";
  html += "</div>";

  html += "</div>";
  html += "</body></html>";

  return html;
}

void ignoreSensorFor(unsigned long &sensorTimer, int milliseconds){
  sensorTimer = millis() + milliseconds;
}

// --- Hilfsfunktionen (Korrigiert, falls du sie später brauchst) ---
void setSensorIgnore(unsigned long &sensorTimer, int milliseconds){
  sensorTimer = millis() + milliseconds;
}

bool isSensorIgnored(unsigned long sensorTimer){
  return millis() < sensorTimer;
}

// --- Hauptlogik ---
void updateMotors() {
  if (!motorsEnabled) {
    m1->run(RELEASE); m2->run(RELEASE);
    m3->run(RELEASE); m4->run(RELEASE);
    return;
  }

  // Sensoren lesen
  int rightSensor = digitalRead(IR_SENSOR_D2);
  int leftSensor = digitalRead(IR_SENSOR_D3);
  int middleSensor = digitalRead(IR_SENSOR_MIDDLE);

  // Basis-Geschwindigkeit setzen
  m1->setSpeed(motorSpeed); m2->setSpeed(motorSpeed);
  m3->setSpeed(motorSpeed); m4->setSpeed(motorSpeed);

  // --- Logik-Prioritäten ---
  
  // 1. KREUZUNG (Beide Sensoren auf Schwarz/HIGH)
  if (leftSensor == HIGH && rightSensor == HIGH && middleSensor == LOW){
    int randomNumber = random(1,3);

    int fastSpeed = motorSpeed * 1.5;
    if (fastSpeed > 240) fastSpeed = 240;
    int slowSpeed = fastSpeed / 2; 

        if (randomNumber == 1) {
      // --- LINKSKURVE ---
      Serial.println("Junction: Random TURN LEFT (Curve)");
      m1->setSpeed(fastSpeed); m2->setSpeed(fastSpeed);
      m1->run(FORWARD); m2->run(FORWARD);
      
      m3->setSpeed(slowSpeed); m4->setSpeed(slowSpeed);
      m3->run(BACKWARD); m4->run(BACKWARD);
      
      delay(750); // Zeit für die Kurve

    } 
    else if (randomNumber == 2) {
      // --- RECHTSKURVE ---
      Serial.println("Junction: Random TURN RIGHT (Curve)");
      m1->setSpeed(slowSpeed); m2->setSpeed(slowSpeed);
      m1->run(BACKWARD); m2->run(BACKWARD);
      
      m3->setSpeed(fastSpeed); m4->setSpeed(fastSpeed);
      m3->run(FORWARD); m4->run(FORWARD);
      
      delay(750); // Zeit für die Kurve
    }
  }
  if (leftSensor == HIGH && rightSensor == HIGH && middleSensor == HIGH) {
    // Zufallszahl 1 bis 3 (1, 2 oder 3)
    // 1 = Links, 2 = Rechts, 3 = Geradeaus
    int randomNumber = random(1, 4); 
    
    // Berechne die Geschwindigkeiten für Kurven
    int fastSpeed = motorSpeed * 1.5;
    if (fastSpeed > 240) fastSpeed = 240;
    int slowSpeed = fastSpeed / 2; 

    if (randomNumber == 1) {
      // --- LINKSKURVE ---
      Serial.println("Junction: Random TURN LEFT (Curve)");
      m1->setSpeed(fastSpeed); m2->setSpeed(fastSpeed);
      m1->run(FORWARD); m2->run(FORWARD);
      
      m3->setSpeed(slowSpeed); m4->setSpeed(slowSpeed);
      m3->run(BACKWARD); m4->run(BACKWARD);
      
      delay(750); // Zeit für die Kurve

    } else if (randomNumber == 2) {
      // --- RECHTSKURVE ---
      Serial.println("Junction: Random TURN RIGHT (Curve)");
      m1->setSpeed(slowSpeed); m2->setSpeed(slowSpeed);
      m1->run(BACKWARD); m2->run(BACKWARD);
      
      m3->setSpeed(fastSpeed); m4->setSpeed(fastSpeed);
      m3->run(FORWARD); m4->run(FORWARD);
      
      delay(750); // Zeit für die Kurve

    } else {
      // --- GERADEAUS ---
      Serial.println("Junction: Random STRAIGHT");
      
      // Standard-Geschwindigkeit nutzen
      m1->setSpeed(motorSpeed); m2->setSpeed(motorSpeed);
      m3->setSpeed(motorSpeed); m4->setSpeed(motorSpeed);
      
      // Einfach alle vorwärts
      m1->run(FORWARD); m2->run(FORWARD);
      m3->run(FORWARD); m4->run(FORWARD);
      
      // Kürzere Zeit! Wir müssen nur über die schwarze Querlinie kommen.
      // 1000ms wäre hier viel zu lang.
      delay(400); 
    }
    
    // Nach dem Manöver kurz entspannen
    m1->run(RELEASE); m2->run(RELEASE);
    m3->run(RELEASE); m4->run(RELEASE);
    delay(100);
  }

  // 2. KORREKTUR RECHTS (Rechter Sensor auf Schwarz)
  else if (rightSensor == HIGH) {
    // Nur korrigieren, wenn wir nicht gerade eine Kreuzung ignorieren sollten
    m1->run(FORWARD);  m2->run(FORWARD);
    m3->run(BACKWARD); m4->run(BACKWARD);
    Serial.println("Auto: CORRECTION RIGHT");
  }
  
  // 3. KORREKTUR LINKS (Linker Sensor auf Schwarz)
  else if (leftSensor == HIGH) {
    m1->run(BACKWARD); m2->run(BACKWARD);
    m3->run(FORWARD);  m4->run(FORWARD);
    Serial.println("Auto: CORRECTION LEFT");
  }
  
  // 4. GERADEAUS (Standard: Beide Sensoren auf Weiß/LOW)
  else {
    m1->run(FORWARD); m2->run(FORWARD); 
    m3->run(FORWARD); m4->run(FORWARD);
    Serial.println("Auto: FORWARD");
  }
}

void executeManualCommand(String dir) {
  Serial.print("Manual command: ");
  Serial.println(dir);

  m1->setSpeed(motorSpeed); m2->setSpeed(motorSpeed);
  m3->setSpeed(motorSpeed); m4->setSpeed(motorSpeed);

  if (dir.startsWith("fwd")) {
    m1->run(FORWARD); m2->run(FORWARD); m3->run(FORWARD); m4->run(FORWARD);
  } else if (dir.startsWith("bwd")) {
    m1->run(BACKWARD); m2->run(BACKWARD); m3->run(BACKWARD); m4->run(BACKWARD);
  } else if (dir.startsWith("lft")) {
    // User's desired LEFT: Right motors BACKWARD, Left motors FORWARD
    m1->run(BACKWARD); m2->run(BACKWARD);
    m3->run(FORWARD); m4->run(FORWARD);
  } else if (dir.startsWith("rgt")) {
    // User's desired RIGHT: Right motors FORWARD, Left motors BACKWARD
    m1->run(FORWARD); m2->run(FORWARD);
    m3->run(BACKWARD); m4->run(BACKWARD);
  }

  delay(250); // Run motors for a short burst
  // The motors will be released by the main loop's call to updateMotors()
}

void handleRequest(String request) {
  if (request.indexOf("GET /stop") != -1) {
    motorsEnabled = false;
    Serial.println("Autonomous mode stopped");
  } else if (request.indexOf("GET /start") != -1) {
    motorsEnabled = true;
    Serial.println("Autonomous mode started");
  } else if (request.indexOf("GET /set-speed") != -1) {
    int speedIndex = request.indexOf("value=");
    if (speedIndex != -1) {
      String speedValueStr = request.substring(speedIndex + 6);
      motorSpeed = speedValueStr.toInt();
      if (motorSpeed < 0) motorSpeed = 0;
      if (motorSpeed > 255) motorSpeed = 255;
      Serial.println("Speed set to: " + String(motorSpeed));
    }
  } else if (request.indexOf("GET /manual") != -1 && !motorsEnabled) {
    int dirIndex = request.indexOf("dir=");
    if (dirIndex != -1) {
      // Extract the first three chars for safety, e.g. "fwd"
      String dir = request.substring(dirIndex + 4, dirIndex + 4 + 3);
      executeManualCommand(dir);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Use INPUT_PULLUP to provide a default HIGH state and prevent floating inputs
  pinMode(IR_SENSOR_D2, INPUT_PULLUP);
  pinMode(IR_SENSOR_D3, INPUT_PULLUP);
  pinMode(IR_SENSOR_MIDDLE, INPUT_PULLUP);

  if (!AFMS.begin()) {
    Serial.println("Could not find Motor Shield");
    while (1);
  }

  m1 = AFMS.getMotor(1); // Right front
  m2 = AFMS.getMotor(2); // Right back
  m3 = AFMS.getMotor(3); // Left back
  m4 = AFMS.getMotor(4); // Left front

  WiFi.beginAP(ssid, password);
  server.begin();

  Serial.println("WiFi AP started");
  Serial.println("SSID: " + String(ssid));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  updateMotors();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  String request = "";
  // Wait for the client to send data, but don't block forever.
  unsigned long timeout = millis() + 100;
  while(client.connected() && !client.available() && millis() < timeout) {
    // wait
  }

  while (client.available()) {
    char c = client.read();
    if (c == '\n') {
      break;
    }
    if (c != '\r') {
      request += c;
    }
  }

  handleRequest(request);

  String htmlPage = makePage();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.print(htmlPage);

  delay(10);
  client.stop();
}
