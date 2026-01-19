#include <WiFiS3.h>
#include <Adafruit_MotorShield.h>
#include <stdlib.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *m1, *m2, *m3, *m4;

const int IR_SENSOR_D2 = 2;  // Right sensor
const int IR_SENSOR_D3 = 3;  // Left sensor
const int IR_SENSOR_MIDDLE = 4; // Middle sensor

const char* ssid = "MarvinMobile";
const char* password = "MarvinOS";

unsigned long leftSensorIgnoreUntil = 0;
unsigned long rightSensorIgnoreUntil = 0;

WiFiServer server(80);

// Car state
bool motorsEnabled = false; // Initially stopped
int motorSpeed = 100;       // Default speed

//manual driving
unsigned long lastManualCommandTime = 0;
const unsigned long MANUAL_TIMEOUT = 550; // Auto-Stop after 0.5
bool manualActive = false;

String makePage() {
  String buttonText = motorsEnabled ? "STOP AUTONOMOUS" : "START AUTONOMOUS";
  String buttonAction = motorsEnabled ? "/stop" : "/start";
  String buttonColorClass = motorsEnabled ? "btn-stop" : "btn-start";
  
  String manualState = motorsEnabled ? "disabled-area" : "";

  String html = R"===(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>
    <style>
      :root {
        /* Dark Mode */
        --bg-color: #121212;
        --card-bg: #1e1e1e;
        --text: #e0e0e0;
        --sub-text: #888;
        --accent: #3b82f6;
        --btn-active: #2563eb;
        --dpad-bg: #334155;
        --dpad-shadow: #1e293b;
        --track-bg: #444;
        --shadow-color: rgba(0,0,0,0.5);
      }

      /* Light Mode */
      .light-mode {
        --bg-color: #f0f2f5;
        --card-bg: #ffffff;
        --text: #333333;
        --sub-text: #666;
        --accent: #2563eb;
        --btn-active: #1d4ed8;
        --dpad-bg: #3b82f6;
        --dpad-shadow: #1d4ed8;
        --track-bg: #e2e8f0;
        --shadow-color: rgba(0,0,0,0.15);
      }

      body { 
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; 
        text-align: center; 
        padding: 20px; 
        background: var(--bg-color); 
        color: var(--text);
        user-select: none; 
        -webkit-user-select: none; 
        margin: 0;
        transition: background 0.3s, color 0.3s; /* Smooth theme transition */
      }
      .container { 
        max-width: 400px; 
        margin: 0 auto; 
        background: var(--card-bg); 
        padding: 25px; 
        border-radius: 20px; 
        box-shadow: 0 10px 25px var(--shadow-color); 
        transition: background 0.3s, box-shadow 0.3s;
      }
      
      /* Clickable Title */
      h1 { 
        margin-top: 0; 
        color: var(--text); 
        font-weight: 300; 
        letter-spacing: 1px; 
        cursor: pointer; 
      }
      h1:active { opacity: 0.7; }
      
      .btn { 
        display: block; width: 100%; padding: 20px 0; 
        font-size: 18px; font-weight: bold; border: none; border-radius: 12px; 
        cursor: pointer; text-decoration: none; color: white; margin-bottom: 25px; 
        text-transform: uppercase; letter-spacing: 1px;
        transition: transform 0.1s;
      }
      .btn:active { transform: scale(0.98); }
      .btn-start { background: linear-gradient(135deg, #10b981, #059669); box-shadow: 0 4px 15px rgba(16, 185, 129, 0.4); }
      .btn-stop { background: linear-gradient(135deg, #ef4444, #dc2626); box-shadow: 0 4px 15px rgba(239, 68, 68, 0.4); }
      
      /* SLIDER */
      .slider-container { 
        margin: 30px 0; padding: 20px; 
        background: var(--bg-color); 
        border-radius: 15px; 
        border: 1px solid var(--track-bg);
      }
      .slider-header { display: flex; justify-content: space-between; margin-bottom: 15px; font-weight: bold; color: var(--sub-text); }
      
      input[type=range] { -webkit-appearance: none; width: 100%; background: transparent; padding: 10px 0; }
      input[type=range]::-webkit-slider-runnable-track {
        width: 100%; height: 8px; cursor: pointer;
        background: var(--track-bg); border-radius: 4px;
      }
      input[type=range]::-moz-range-track {
        width: 100%; height: 8px; cursor: pointer;
        background: var(--track-bg); border-radius: 4px;
      }
      input[type=range]::-webkit-slider-thumb {
        -webkit-appearance: none; height: 32px; width: 32px; 
        border-radius: 50%; background: var(--accent); cursor: pointer;
        margin-top: -12px; box-shadow: 0 0 10px rgba(59, 130, 246, 0.5);
      }

      /* CONTROLLER */
      .controller { margin-top: 30px; padding-top: 20px; border-top: 1px solid var(--track-bg); }
      .disabled-area { opacity: 0.3; pointer-events: none; filter: grayscale(100%); }
      
      .d-pad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; max-width: 280px; margin: 0 auto; }
      .pad-btn { 
        background: var(--dpad-bg); color: white; border: none; border-radius: 12px; 
        font-size: 28px; height: 75px; cursor: pointer; touch-action: manipulation; 
        box-shadow: 0 6px 0 var(--dpad-shadow); transition: transform 0.1s, box-shadow 0.1s;
        display: flex; align-items: center; justify-content: center;
      }
      .pad-btn:active { transform: translateY(6px); box-shadow: 0 0 0 var(--dpad-shadow); background: var(--btn-active); }
      
      .stop-btn { background: #64748b !important; box-shadow: 0 6px 0 #475569 !important; font-size: 16px; font-weight: bold; }
      .stop-btn:active { box-shadow: 0 0 0 #475569 !important; }
      
      svg { pointer-events: none; }
    </style>
    <script>
      // --- THEME LOGIC ---
      function toggleTheme() {
        document.body.classList.toggle('light-mode');
        const isLight = document.body.classList.contains('light-mode');
        localStorage.setItem('theme', isLight ? 'light' : 'dark');
      }
      
      window.onload = function() {
        const savedTheme = localStorage.getItem('theme');
        if (savedTheme === 'light') {
          document.body.classList.add('light-mode');
        }
      };

      var moveInterval = null;
      var isRequestPending = false; 
      var lastRequestTime = 0;   
      var pendingXHR = null;

      function send(cmd) {
        var now = Date.now();
        if (cmd === 'stop') {
          if (pendingXHR) pendingXHR.abort();
          isRequestPending = false;
        } else {
          if (now - lastRequestTime < 150) return; 
          if (isRequestPending) return;
        }
        isRequestPending = true;
        lastRequestTime = now;
        
        pendingXHR = new XMLHttpRequest();
        pendingXHR.timeout = 300; 
        pendingXHR.open('GET', '/manual?dir=' + cmd, true);
        pendingXHR.onload = function() { isRequestPending = false; pendingXHR = null; };
        pendingXHR.onerror = function() { isRequestPending = false; pendingXHR = null; };
        pendingXHR.ontimeout = function() { isRequestPending = false; pendingXHR = null; };
        pendingXHR.send();
      }

      function updateValVisual(val) { document.getElementById('val').innerText = val; }

      function sendSpeed(val) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/set-speed?value=' + val, true);
        xhr.send();
      }

      function startMove(e, dir) {
        if(e.cancelable) e.preventDefault(); 
        if(moveInterval) clearInterval(moveInterval);
        send(dir);
        moveInterval = setInterval(function() { send(dir); }, 150);
      }
      
      function stopMove(e) {
        if(e && e.cancelable) e.preventDefault();
        if(moveInterval) { clearInterval(moveInterval); moveInterval = null; }
        send('stop'); 
      }
    </script>
  </head>
  <body>
    <div class='container'>
      <h1 onclick="toggleTheme()" title="Tap to switch Light/Dark mode">Marvin Mobile</h1>
      
      <a href='CHANGE_ACTION' class='btn CHANGE_COLOR'>CHANGE_TEXT</a>

      <div class='slider-container'>
        <div class='slider-header'>
          <span>Speed</span>
          <span id='val'>CURRENT_SPEED</span>
        </div>
        <input type='range' min='70' max='250' step='10' value='CURRENT_SPEED' 
              oninput='updateValVisual(this.value)' 
              onchange='sendSpeed(this.value)'>
      </div>

      <div class='controller MANUAL_STATE'>
        <h2 style='color:var(--sub-text); font-size:14px; text-transform:uppercase; letter-spacing:2px;'>Manual Control</h2>
        <div class="d-pad">
          <div></div>
          <button class="pad-btn" ontouchstart="startMove(event, 'fwd')" onmousedown="startMove(event, 'fwd')">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M12 19V5"/><path d="M5 12l7-7 7 7"/></svg>
          </button>
          <div></div>

          <button class="pad-btn" ontouchstart="startMove(event, 'lft')" onmousedown="startMove(event, 'lft')">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M19 12H5"/><path d="M12 19l-7-7 7-7"/></svg>
          </button>
          
          <button class="pad-btn stop-btn" onclick="stopMove(event)">STOP</button>
          
          <button class="pad-btn" ontouchstart="startMove(event, 'rgt')" onmousedown="startMove(event, 'rgt')">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M5 12h14"/><path d="M12 5l7 7-7 7"/></svg>
          </button>

          <div></div>
          <button class="pad-btn" ontouchstart="startMove(event, 'bwd')" onmousedown="startMove(event, 'bwd')">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M12 5v14"/><path d="M19 12l-7 7-7-7"/></svg>
          </button>
          <div></div>
        </div>
      </div>
    </div>
  </body>
  </html>
)===";

  html.replace("CHANGE_ACTION", buttonAction);
  html.replace("CHANGE_TEXT", buttonText);
  html.replace("CHANGE_COLOR", buttonColorClass);
  html.replace("CURRENT_SPEED", String(motorSpeed));
  html.replace("MANUAL_STATE", manualState);

  return html;
}

void ignoreSensorFor(unsigned long &sensorTimer, int milliseconds){
  sensorTimer = millis() + milliseconds;
}

void setSensorIgnore(unsigned long &sensorTimer, int milliseconds){
  sensorTimer = millis() + milliseconds;
}

bool isSensorIgnored(unsigned long sensorTimer){
  return millis() < sensorTimer;
}

void releaseMotors(){
  m1->run(RELEASE); m2->run(RELEASE);
  m3->run(RELEASE); m4->run(RELEASE);
}

void setMotorSpeed(int motorSpeed){
  m1->setSpeed(motorSpeed); m2->setSpeed(motorSpeed);
  m3->setSpeed(motorSpeed); m4->setSpeed(motorSpeed);
}

void leftCurve(int fastSpeed, int slowSpeed){
  Serial.println("Junction: Random TURN RIGHT (Curve)");
  m1->setSpeed(slowSpeed); m2->setSpeed(slowSpeed);
  m1->run(BACKWARD); m2->run(BACKWARD);

  m3->setSpeed(fastSpeed); m4->setSpeed(fastSpeed);
  m3->run(FORWARD); m4->run(FORWARD);
}

void rightCurve(int fastSpeed, int slowSpeed){
  Serial.println("Junction: Random TURN LEFT (Curve)");
  m1->setSpeed(fastSpeed); m2->setSpeed(fastSpeed);
  m1->run(FORWARD); m2->run(FORWARD);

  m3->setSpeed(slowSpeed); m4->setSpeed(slowSpeed);
  m3->run(BACKWARD); m4->run(BACKWARD);
}

void forwardCurve(){
  m1->run(FORWARD); m2->run(FORWARD); 
  m3->run(FORWARD); m4->run(FORWARD);
}


// --- main stuff ---
void updateMotors()
 {
  if (!motorsEnabled) {
    releaseMotors();
    return;
  }

  // read sensors
  int rightSensor = digitalRead(IR_SENSOR_D2);
  int leftSensor = digitalRead(IR_SENSOR_D3);
  int middleSensor = digitalRead(IR_SENSOR_MIDDLE);

  setMotorSpeed(motorSpeed);

  // --- logic-priority kinda important ---
  
  // 1. Cross
  if (leftSensor == HIGH && rightSensor == HIGH && middleSensor == LOW){
    int randomNumber = random(1,3);

    int fastSpeed = motorSpeed * 1.5;
    if (fastSpeed > 240) fastSpeed = 240;
    int slowSpeed = fastSpeed / 2; 

        if (randomNumber == 1) {
      leftCurve(fastSpeed, slowSpeed);      
      delay(750); 

    } 
    else if (randomNumber == 2) {
      rightCurve(fastSpeed, slowSpeed);      
      delay(750);
    }
  }
  if (leftSensor == HIGH && rightSensor == HIGH && middleSensor == HIGH) {
    int randomNumber = random(1, 4); 
    
    int fastSpeed = motorSpeed * 1.5;
    if (fastSpeed > 240) fastSpeed = 240;
    int slowSpeed = fastSpeed / 2; 

    if (randomNumber == 1) {
      leftCurve(fastSpeed, slowSpeed);
      delay(750);

    } else if (randomNumber == 2) {
      rightCurve(fastSpeed, slowSpeed);      
      delay(750);

    } else {
      Serial.println("Junction: Random STRAIGHT");
      setMotorSpeed(motorSpeed);
      forwardCurve();
      delay(400); 
    }
    releaseMotors();
    delay(100);
  }
  else if (rightSensor == HIGH) {
    rightCurve(motorSpeed,motorSpeed);
  }
  else if (leftSensor == HIGH) {
    leftCurve(motorSpeed,motorSpeed);
  }
  else {
    forwardCurve();
  }
}

void executeManualCommand(String dir) {
  manualActive = true;
  lastManualCommandTime = millis();

  // activate motor
  setMotorSpeed(motorSpeed);

  if (dir == "sto") {
    releaseMotors();
    manualActive = false;
    Serial.println("stop manuell");
  } else if (dir == "fwd") {
    m1->run(FORWARD); m2->run(FORWARD); m3->run(FORWARD); m4->run(FORWARD);
    Serial.println("forward manuell");
  } else if (dir == "bwd") {
    m1->run(BACKWARD); m2->run(BACKWARD); m3->run(BACKWARD); m4->run(BACKWARD);
    Serial.println("backward manuell");
  } else if (dir == "lft") {
    m1->run(BACKWARD); m2->run(BACKWARD); m3->run(FORWARD); m4->run(FORWARD);
    Serial.println("left manuell");
  } else if (dir == "rgt") {
    m1->run(FORWARD); m2->run(FORWARD); m3->run(BACKWARD); m4->run(BACKWARD);
    Serial.println("right manuell");
  }
}

void handleRequest(String request) {
  if (request.indexOf("GET /stop") != -1) {
    motorsEnabled = false;
    releaseMotors();
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
      String dir = request.substring(dirIndex + 4, dirIndex + 4 + 3);
      Serial.println("execute manual" + dir);
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

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  String request = "";
  // Wait for the client to send data
  unsigned long timeout = millis() + 50;
  while(client.connected() && !client.available() && millis() < timeout) {
    Serial.println("why stay here? request loop");
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
  Serial.println("REQUEST:" + request);
  handleRequest(request);

  //trying to timeout
  if (motorsEnabled) {
    updateMotors();
  } 
  else {
    if (manualActive && (millis() - lastManualCommandTime > MANUAL_TIMEOUT)) {
      releaseMotors();
      if(client){
        Serial.println((millis() - lastManualCommandTime));
      }
      manualActive = false;
      Serial.println("Manual Timeout - Safety Stop, from manual active check");
    }
  }

  String htmlPage = makePage();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.print(htmlPage);
  Serial.println("connection closed, fuck you!");

  delay(10);
  client.stop();
}