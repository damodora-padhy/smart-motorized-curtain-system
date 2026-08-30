// --- BLYNK CREDENTIALS (MUST BE AT THE VERY TOP) ---
#define BLYNK_TEMPLATE_ID "TMPL_YOUR_ID"
#define BLYNK_TEMPLATE_NAME "Smart Curtain"
#define BLYNK_AUTH_TOKEN "fCbdakxBCCQY4Y66NRnGtknRNvetCkSr"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Stepper.h>
#include <DHT.h>
#include "BluetoothSerial.h"

// --- BLUETOOTH SETUP ---
BluetoothSerial SerialBT;

// --- WIFI SETUP ---
char ssid[] = "Damodara_padhy";
char pass[] = "1111111111";
// --- SENSOR PINS ---
#define RAIN_PIN 27
#define IR_PIN 26
#define LDR_PIN 34
#define DHT_PIN 4

// --- MOTOR PINS ---
#define IN1_A 18
#define IN2_A 19
#define IN3_A 21
#define IN4_A 22

#define IN1_B 32
#define IN2_B 33
#define IN3_B 25
#define IN4_B 23

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

const int stepsPerRevolution = 2048; 
Stepper motorA(stepsPerRevolution, IN1_A, IN3_A, IN2_A, IN4_A);
Stepper motorB(stepsPerRevolution, IN1_B, IN3_B, IN2_B, IN4_B);

// --- STATE VARIABLES ---
bool wasRaining = false;
bool wasObjectDetected = false;
bool wasLightDetected = false;
bool wasTempBelow28 = false; 

unsigned long lastDhtReadTime = 0;
unsigned long lastBlynkUpdate = 0;

bool isManualMode = false; 
bool isCurtainOpen = false; // Tracks actual physical position

// ---------------------------------------------------------
// BLYNK APP VIRTUAL PIN HANDLERS
// ---------------------------------------------------------
// V0: Open Button
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) {
    isManualMode = true;
    updateStatus("App Command: OPENING");
    moveDoorsForSeconds(true, 30);
  }
}

// V1: Close Button
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    isManualMode = true;
    updateStatus("App Command: CLOSING");
    moveDoorsForSeconds(false, 30);
  }
}

// V2: Auto Mode Switch
BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    isManualMode = false;
    updateStatus("AUTO MODE ACTIVATED");
    // Reset sensor states
    wasRaining = (digitalRead(RAIN_PIN) == LOW);
    wasObjectDetected = (digitalRead(IR_PIN) == LOW);
    wasLightDetected = (digitalRead(LDR_PIN) == LOW);
  } else {
    isManualMode = true;
    updateStatus("MANUAL MODE ACTIVATED");
  }
}

// Custom function to update both Bluetooth and Blynk simultaneously
void updateStatus(String message) {
  SerialBT.println(message);
  Blynk.virtualWrite(V3, message); // V3: Status Display in Blynk
}

void setup() {
  Serial.begin(115200);
  
  pinMode(RAIN_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  dht.begin();
  
  motorA.setSpeed(12);
  motorB.setSpeed(12);
  
  // Start Wireless Protocols
  SerialBT.begin("Smart_Sliding_Door"); 
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  wasRaining = (digitalRead(RAIN_PIN) == LOW);
  wasObjectDetected = (digitalRead(IR_PIN) == LOW);
  wasLightDetected = (digitalRead(LDR_PIN) == LOW);

  updateStatus("System Online");
}

void loop() {
  // 1. Maintain WiFi Connection
  Blynk.run();

  // 2. BLUETOOTH COMMAND LISTENER
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim(); 
    command.toLowerCase(); 
    
    if (command == "open") {
      isManualMode = true;
      updateStatus("BT Command: OPENING");
      moveDoorsForSeconds(true, 30); 
    } 
    else if (command == "close") {
      isManualMode = true;
      updateStatus("BT Command: CLOSING");
      moveDoorsForSeconds(false, 30); 
    } 
    else if (command == "force open") {
      isManualMode = true;
      isCurtainOpen = false; 
      updateStatus("BT: FORCE OPEN");
      moveDoorsForSeconds(true, 30); 
    }
    else if (command == "force close") {
      isManualMode = true;
      isCurtainOpen = true; 
      updateStatus("BT: FORCE CLOSE");
      moveDoorsForSeconds(false, 30); 
    }
    else if (command == "auto") {
      isManualMode = false;
      updateStatus("AUTO MODE ACTIVATED");
      Blynk.virtualWrite(V2, 1); // Sync switch in Blynk App
      wasRaining = (digitalRead(RAIN_PIN) == LOW);
      wasObjectDetected = (digitalRead(IR_PIN) == LOW);
      wasLightDetected = (digitalRead(LDR_PIN) == LOW);
    }
  }

  // 3. AUTO MODE SENSOR LOGIC
  if (!isManualMode) {
    bool isRaining = (digitalRead(RAIN_PIN) == LOW);
    bool isObjectDetected = (digitalRead(IR_PIN) == LOW);
    bool isLightDetected = (digitalRead(LDR_PIN) == LOW);
    
    float currentTemp = 28.0; 
    
    // Read temp and push to Blynk App Gauge (V4) every 2 seconds
    if (millis() - lastDhtReadTime > 2000) {
      currentTemp = dht.readTemperature();
      if (!isnan(currentTemp)) {
        Blynk.virtualWrite(V4, currentTemp); 
      }
      lastDhtReadTime = millis();
    }
    
    bool isTempBelow28 = (currentTemp < 28.0);

    // --- PRIORITY 1: SAFETY (IR SENSOR) ---
    if (isObjectDetected) {
      if (!wasObjectDetected) {
        updateStatus("Auto: Radar Hit! OPENING");
        moveDoorsForSeconds(true, 30); 
        wasObjectDetected = true;
      }
    } 
    // --- PRIORITY 2: WEATHER (RAIN SENSOR) ---
    else if (isRaining) {
      wasObjectDetected = false; 
      if (!wasRaining) {
        updateStatus("Auto: Rain! CLOSING");
        moveDoorsForSeconds(false, 30); 
        wasRaining = true;
      }
    }
    // --- PRIORITY 3: COMFORT (LIGHT & TEMP) ---
    else {
      wasObjectDetected = false; 
      wasRaining = false; 

      if (isTempBelow28 && !wasTempBelow28) {
        updateStatus("Auto: Cool Temp. OPENING");
        moveDoorsForSeconds(true, 30); 
        wasTempBelow28 = true;
      } else if (!isTempBelow28 && wasTempBelow28) {
        updateStatus("Auto: Hot Temp! CLOSING");
        moveDoorsForSeconds(false, 30); 
        wasTempBelow28 = false;
      }

      if (isLightDetected && !wasLightDetected) {
        updateStatus("Auto: Sunlight. OPENING");
        moveDoorsForSeconds(true, 30); 
        wasLightDetected = true;
      } else if (!isLightDetected && wasLightDetected) {
        updateStatus("Auto: Nighttime. CLOSING");
        moveDoorsForSeconds(false, 30); 
        wasLightDetected = false;
      }
    }
  }
}

// ---------------------------------------------------------
// SYNCHRONIZED TIME-BASED MOVEMENT WITH DUAL-STOP
// ---------------------------------------------------------
void moveDoorsForSeconds(bool isOpening, int seconds) {
  
  if (isOpening && isCurtainOpen) {
    updateStatus("Already Open. Stopped.");
    return; 
  }
  if (!isOpening && !isCurtainOpen) {
    updateStatus("Already Closed. Stopped.");
    return; 
  }

  unsigned long duration = seconds * 1000UL; 
  unsigned long startTime = millis();
  int stepChunk = 20; 
  
  while (millis() - startTime < duration) {
    
    // We must run Blynk.run() inside the motor loop so it doesn't disconnect from WiFi!
    Blynk.run(); 

    // --- EMERGENCY STOP LISTENER (BLUETOOTH) ---
    if (SerialBT.available()) {
      String emergencyCmd = SerialBT.readStringUntil('\n');
      emergencyCmd.trim(); 
      emergencyCmd.toLowerCase(); 
      if (emergencyCmd == "stop") {
        updateStatus("⚠️ BT E-STOP ACTIVATED!");
        break; 
      }
    }

    // --- MOTOR MOVEMENT ---
    if (isOpening) {
      motorA.step(-stepChunk); 
      motorB.step(stepChunk);  
    } else {
      motorA.step(stepChunk);  
      motorB.step(-stepChunk); 
    }
  }
  
  // Cut power to coils to prevent overheating
  digitalWrite(IN1_A, LOW); digitalWrite(IN2_A, LOW);
  digitalWrite(IN3_A, LOW); digitalWrite(IN4_A, LOW);
  
  digitalWrite(IN1_B, LOW); digitalWrite(IN2_B, LOW);
  digitalWrite(IN3_B, LOW); digitalWrite(IN4_B, LOW);

  isCurtainOpen = isOpening; 
  updateStatus(isOpening ? "Status: OPEN" : "Status: CLOSED");
}