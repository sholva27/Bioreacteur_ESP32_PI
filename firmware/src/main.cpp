#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_ADS1X15.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "config.h"
#include "dashboard.html.h"

// Hardware interfaces
Adafruit_ADS1115 ads;
RTC_DS3231 rtc;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Global state
String mqttBroker = MQTT_BROKER;
bool mqttEnabled = false;

float phTarget = PH_TARGET;
float phHysteresis = PH_HYSTERESIS;
float tempTarget = TEMP_TARGET;
int stirrerSpeed = STIRRER_SPEED_DEFAULT;
long feedingInterval = FEEDING_INTERVAL_MS;

// Calibration Data
float phOffset = 0.0;
float phSlope = 1.0;
float odZeroVoltage = 0.0;

float currentPH = 0.0;
float currentOD = 0.0;
float currentTemp = 0.0;
float currentPH_V = 0.0;
float currentOD_V = 0.0;
float currentUV_V = 0.0;  // UV Intensity voltage
float currentFluo = 0.0; // NADH Fluorescence intensity
float growthRate = 0.0;  // Specific growth rate (mu)

float lastOD = 0.0;
unsigned long lastODTime = 0;

bool sensorError = false;
unsigned long lastSensorRead = 0;
unsigned long lastFeedingTime = 0;
unsigned long lastLogTime = 0;
unsigned long lastFluoRead = 0;

// Pump states (for non-blocking duration)
bool nutrientPumpActive = false;
unsigned long nutrientPumpStartTime = 0;

// PID constants for pH control
float Kp = 100.0, Ki = 0.0, Kd = 0.0;
float phIntegral = 0.0, lastPhError = 0.0;

// Function prototypes
void updateSensors();
void setupMQTT();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void controlPH();
void controlTemp();
void controlFeeding();
void logData();
void updateFluo();
String getTelemetryJSON();
void emergencyStop();
void setupOTA();
void loadSettings();
void saveSettings();

void setup() {
  Serial.begin(115200);

  // PLX-DAQ Excel Initialization
  Serial.println("CLEARDATA"); // Clear existing data in Excel
  Serial.println("LABEL,Time,pH,OD,Temp,GrowthRate"); // Set column headers

  // Initialize Filesystem
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  loadSettings();

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize Sensors
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115.");
    sensorError = true;
  }
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    sensorError = true;
  }
  sensors.begin();
  sensors.setWaitForConversion(false); // Non-blocking temperature reads
  sensors.requestTemperatures();       // Start first conversion

  // Initialize Pins
  pinMode(PUMP_ACID_PIN, OUTPUT);
  pinMode(PUMP_BASE_PIN, OUTPUT);
  pinMode(PUMP_NUTRIENT_PIN, OUTPUT);
  pinMode(OD_LIGHT_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(STIRRER_PIN, OUTPUT);
  pinMode(TOUCH_BUTTON_PIN, INPUT);
  pinMode(FLUO_LED_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  emergencyStop(); // Ensure all pumps off at start

  // WiFi Setup
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println(WiFi.localIP());

  // Web Server Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getTelemetryJSON());
  });

  server.on("/download_log", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/log.csv", "text/csv", true);
  });

  server.on("/pump", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("type")) {
      String type = request->getParam("type")->value();
      if(type == "nutrient") {
        nutrientPumpActive = true;
        nutrientPumpStartTime = millis();
        digitalWrite(PUMP_NUTRIENT_PIN, HIGH);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<1024> doc;
    doc["phTarget"] = phTarget;
    doc["phHysteresis"] = phHysteresis;
    doc["tempTarget"] = tempTarget;
    doc["stirrerSpeed"] = stirrerSpeed;
    doc["feedingInterval"] = feedingInterval;
    doc["kp"] = Kp;
    doc["ki"] = Ki;
    doc["kd"] = Kd;
    doc["phOffset"] = phOffset;
    doc["phSlope"] = phSlope;
    doc["odZero"] = odZeroVoltage;
    doc["mqttBroker"] = mqttBroker;
    doc["mqttEnabled"] = mqttEnabled;
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, (const char*)data);
    if(doc.containsKey("phTarget")) phTarget = doc["phTarget"];
    if(doc.containsKey("phHysteresis")) phHysteresis = doc["phHysteresis"];
    if(doc.containsKey("tempTarget")) tempTarget = doc["tempTarget"];
    if(doc.containsKey("stirrerSpeed")) stirrerSpeed = doc["stirrerSpeed"];
    if(doc.containsKey("feedingInterval")) feedingInterval = doc["feedingInterval"];
    if(doc.containsKey("kp")) Kp = doc["kp"];
    if(doc.containsKey("ki")) Ki = doc["ki"];
    if(doc.containsKey("kd")) Kd = doc["kd"];
    if(doc.containsKey("mqttBroker")) mqttBroker = doc["mqttBroker"].as<String>();
    if(doc.containsKey("mqttEnabled")) mqttEnabled = doc["mqttEnabled"];
    if(doc.containsKey("phOffset")) phOffset = doc["phOffset"];
    if(doc.containsKey("phSlope")) phSlope = doc["phSlope"];
    if(doc.containsKey("odZero")) odZeroVoltage = doc["odZero"];
    saveSettings();
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  setupOTA();
  setupMQTT();
}

void loop() {
  ArduinoOTA.handle();
  if (mqttEnabled) {
    if (!mqttClient.connected()) reconnectMQTT();
    mqttClient.loop();
  }
  unsigned long currentMillis = millis();

  // Sensor Reading (Non-blocking)
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    updateSensors();
    lastSensorRead = currentMillis;

    // Control logic dependent on fresh sensor data
    if (!sensorError) {
      controlPH();
      controlTemp();
    } else {
      emergencyStop();
    }
  }

  // Manual touch button logic
  if (digitalRead(TOUCH_BUTTON_PIN) == HIGH && !nutrientPumpActive) {
    nutrientPumpActive = true;
    nutrientPumpStartTime = currentMillis;
    digitalWrite(PUMP_NUTRIENT_PIN, HIGH);
  }

  // Feeding logic (Non-blocking)
  controlFeeding();

  // Fluorescence Reading (Non-blocking)
  if (currentMillis - lastFluoRead >= FLUO_READ_INTERVAL_MS) {
    updateFluo();
    lastFluoRead = currentMillis;
  }

  // Data Logging & MQTT Publishing (Non-blocking)
  if (currentMillis - lastLogTime >= LOG_INTERVAL_MS) {
    logData();
    if (mqttEnabled && mqttClient.connected()) {
       mqttClient.publish(MQTT_TOPIC_PREFIX "/telemetry", getTelemetryJSON().c_str());
    }
    lastLogTime = currentMillis;
  }

  // Heartbeat LED
  static unsigned long lastToggle = 0;
  if (currentMillis - lastToggle >= 500) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastToggle = currentMillis;
  }

  // Stirrer control (simple PWM) - inhibited if sensor error
  if (!sensorError) {
    analogWrite(STIRRER_PIN, stirrerSpeed);
  } else {
    analogWrite(STIRRER_PIN, 0);
  }
}

void controlTemp() {
  if (currentTemp < tempTarget) {
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(HEATER_PIN, LOW);
  }
}

void updateFluo() {
  // Read NADH Fluorescence (340nm exc / 460nm em)
  // For this implementation, we use the AS7341 spectral sensor logic
  // (Assuming AS7341 library is integrated or using ADS channel if analog)

  digitalWrite(FLUO_LED_PIN, HIGH);
  delayMicroseconds(500);

  // Example: Using ADS Channel 2 if using an analog high-sensitivity TIA circuit
  int16_t fluoRaw = ads.readADC_SingleEnded(2);
  digitalWrite(FLUO_LED_PIN, LOW);

  if (fluoRaw != -1) {
    currentFluo = (float)fluoRaw * 0.0001875;
  }
}

void updateSensors() {
  unsigned long currentMillis = millis();

  // Read Temperature result from previous request
  float temp = sensors.getTempCByIndex(0);
  if (temp != DEVICE_DISCONNECTED_C) {
    currentTemp = temp;
  } else {
    sensorError = true;
  }
  // Request temperature for next cycle
  sensors.requestTemperatures();

  // Read UV Intensity (ADS Channel 3)
  int16_t uvRaw = ads.readADC_SingleEnded(ADS_UV_CH);
  if (uvRaw != -1) {
    currentUV_V = (float)uvRaw * 0.0001875;
  }

  // Read pH
  int16_t phRaw = ads.readADC_SingleEnded(ADS_PH_CH);
  if (phRaw == -1) { // Basic check for ADS failure
     sensorError = true;
     return;
  }
  // 0.0001875 is the voltage step for +/- 6.144V range
  currentPH_V = (float)phRaw * 0.0001875;
  currentPH = 7.0 + (currentPH_V * phSlope) + phOffset;

  // Read OD
  digitalWrite(OD_LIGHT_PIN, HIGH);
  delayMicroseconds(500);
  int16_t odRaw = ads.readADC_SingleEnded(ADS_OD_CH);
  digitalWrite(OD_LIGHT_PIN, LOW);

  // OD Calculation with guard
  currentOD_V = (float)odRaw * 0.0001875;
  if (currentOD_V > 0.001) {
    if (odZeroVoltage > 0.1) {
      currentOD = log10(odZeroVoltage / currentOD_V) * OD_CALIBRATION_FACTOR;
    } else {
      currentOD = 0.0;
    }
  } else {
    currentOD = 4.0; // Max out at high density
  }

  // Growth Rate Estimation (Specific Growth Rate mu)
  if (lastODTime > 0 && currentOD > 0.05 && lastOD > 0.05) {
     float dt = (currentMillis - lastODTime) / 3600000.0; // dt in hours
     if (dt > 0.1) { // Every 6 minutes
        float mu = (log(currentOD) - log(lastOD)) / dt;
        growthRate = (growthRate * 0.8) + (mu * 0.2); // Low-pass filter
        lastOD = currentOD;
        lastODTime = currentMillis;
     }
  } else if (currentOD > 0.05 && lastODTime == 0) {
     lastOD = currentOD;
     lastODTime = currentMillis;
  }

  sensorError = false; // Reset if readings successful
}

void controlPH() {
  float error = currentPH - phTarget;
  unsigned long currentMillis = millis();
  static unsigned long lastUpdate = 0;
  float dt = (currentMillis - lastUpdate) / 1000.0;

  if (abs(error) < phHysteresis) {
    digitalWrite(PUMP_ACID_PIN, LOW);
    digitalWrite(PUMP_BASE_PIN, LOW);
    phIntegral = 0;
    lastPhError = error;
    lastUpdate = currentMillis;
    return;
  }

  if (dt > 0) {
    phIntegral += error * dt;
    float phDerivative = (error - lastPhError) / dt;
    float pidOutput = (Kp * error) + (Ki * phIntegral) + (Kd * phDerivative);
    pidOutput = abs(pidOutput);

    static unsigned long lastCycleStart = 0;
    unsigned long cycleTime = currentMillis - lastCycleStart;
    if (cycleTime >= 10000) {
      lastCycleStart = currentMillis;
      cycleTime = 0;
    }

    unsigned long pumpDuration = min((unsigned long)pidOutput, 5000UL);

    if (error > 0) {
      if (cycleTime < pumpDuration) digitalWrite(PUMP_ACID_PIN, HIGH);
      else digitalWrite(PUMP_ACID_PIN, LOW);
      digitalWrite(PUMP_BASE_PIN, LOW);
    } else {
      if (cycleTime < pumpDuration) digitalWrite(PUMP_BASE_PIN, HIGH);
      else digitalWrite(PUMP_BASE_PIN, LOW);
      digitalWrite(PUMP_ACID_PIN, LOW);
    }

    lastPhError = error;
    lastUpdate = currentMillis;
  }
}

void controlFeeding() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastFeedingTime > (unsigned long)feedingInterval) {
    nutrientPumpActive = true;
    nutrientPumpStartTime = currentMillis;
    digitalWrite(PUMP_NUTRIENT_PIN, HIGH);
    lastFeedingTime = currentMillis;
  }
  if (nutrientPumpActive && (currentMillis - nutrientPumpStartTime >= FEEDING_DURATION_MS)) {
    digitalWrite(PUMP_NUTRIENT_PIN, LOW);
    nutrientPumpActive = false;
  }
}

void logData() {
  if (sensorError) return;

  // 1. Log to SPIFFS (Internal Storage)
  File file = SPIFFS.open("/log.csv", FILE_APPEND);
  if(file) {
    DateTime now = rtc.now();
    file.print(now.timestamp());
    file.print(",");
    file.print(currentPH);
    file.print(",");
    file.print(currentOD);
    file.print(",");
    file.print(currentTemp);
    file.print(",");
    file.println(growthRate);
    file.close();
  }

  // 2. Log to PLX-DAQ (Real-time Excel)
  Serial.print("DATA,TIME,");
  Serial.print(currentPH);
  Serial.print(",");
  Serial.print(currentOD);
  Serial.print(",");
  Serial.print(currentTemp);
  Serial.print(",");
  Serial.println(growthRate);
}

void setupMQTT() {
  mqttClient.setServer(mqttBroker.c_str(), MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
  static unsigned long lastReconnectAttempt = 0;
  if (millis() - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = millis();
    if (mqttClient.connect("BiofermenterClient", MQTT_USER, MQTT_PASS)) {
      mqttClient.subscribe(MQTT_TOPIC_PREFIX "/command");
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  if (doc.containsKey("command")) {
    String cmd = doc["command"];
    if (cmd == "feed") {
      nutrientPumpActive = true;
      nutrientPumpStartTime = millis();
      digitalWrite(PUMP_NUTRIENT_PIN, HIGH);
    }
  }
}

void emergencyStop() {
  digitalWrite(PUMP_ACID_PIN, LOW);
  digitalWrite(PUMP_BASE_PIN, LOW);
  digitalWrite(PUMP_NUTRIENT_PIN, LOW);
  digitalWrite(HEATER_PIN, LOW);
  analogWrite(STIRRER_PIN, 0);
}

void loadSettings() {
  File file = SPIFFS.open("/settings.json", FILE_READ);
  if(!file) return;
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, file);
  phTarget = doc["phTarget"] | PH_TARGET;
  mqttBroker = doc["mqttBroker"] | MQTT_BROKER;
  mqttEnabled = doc["mqttEnabled"] | false;
  phHysteresis = doc["phHysteresis"] | PH_HYSTERESIS;
  tempTarget = doc["tempTarget"] | TEMP_TARGET;
  stirrerSpeed = doc["stirrerSpeed"] | STIRRER_SPEED_DEFAULT;
  feedingInterval = doc["feedingInterval"] | FEEDING_INTERVAL_MS;
  Kp = doc["kp"] | 100.0;
  Ki = doc["ki"] | 0.0;
  Kd = doc["kd"] | 0.0;
  phOffset = doc["phOffset"] | 0.0;
  phSlope = doc["phSlope"] | 1.0;
  odZeroVoltage = doc["odZero"] | 0.0;
  file.close();
}

void saveSettings() {
  File file = SPIFFS.open("/settings.json", FILE_WRITE);
  if(!file) return;
  StaticJsonDocument<1024> doc;
  doc["phTarget"] = phTarget;
  doc["mqttBroker"] = mqttBroker;
  doc["mqttEnabled"] = mqttEnabled;
  doc["phHysteresis"] = phHysteresis;
  doc["tempTarget"] = tempTarget;
  doc["stirrerSpeed"] = stirrerSpeed;
  doc["feedingInterval"] = feedingInterval;
  doc["kp"] = Kp;
  doc["ki"] = Ki;
  doc["kd"] = Kd;
  doc["phOffset"] = phOffset;
  doc["phSlope"] = phSlope;
  doc["odZero"] = odZeroVoltage;
  serializeJson(doc, file);
  file.close();
}

void setupOTA() {
  ArduinoOTA.setHostname("biofermenter-esp32s3");
  ArduinoOTA.begin();
}

String getTelemetryJSON() {
  StaticJsonDocument<512> doc;
  DateTime now = rtc.now();
  doc["ph"] = currentPH;
  doc["od"] = currentOD;
  doc["ph_v"] = currentPH_V;
  doc["od_v"] = currentOD_V;
  doc["temp"] = currentTemp;
  doc["mu"] = growthRate;
  doc["fluo"] = currentFluo;
  doc["uv_v"] = currentUV_V;
  doc["timestamp"] = now.timestamp();
  doc["error"] = sensorError;
  String output;
  serializeJson(doc, output);
  return output;
}
