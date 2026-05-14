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

#include "config.h"
#include "dashboard.html.h"

// Hardware interfaces
Adafruit_ADS1115 ads;
RTC_DS3231 rtc;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);

// Global state
float phTarget = PH_TARGET;
float phHysteresis = PH_HYSTERESIS;
long feedingInterval = FEEDING_INTERVAL_MS;

float currentPH = 0.0;
float currentOD = 0.0;
float currentTemp = 0.0;
float growthRate = 0.0; // Specific growth rate (mu)

float lastOD = 0.0;
unsigned long lastODTime = 0;

bool sensorError = false;
unsigned long lastSensorRead = 0;
unsigned long lastFeedingTime = 0;
unsigned long lastLogTime = 0;

// Pump states (for non-blocking duration)
bool nutrientPumpActive = false;
unsigned long nutrientPumpStartTime = 0;

// PID constants for pH control
float Kp = 100.0, Ki = 0.0, Kd = 0.0;
float phIntegral = 0.0, lastPhError = 0.0;

// Function prototypes
void updateSensors();
void controlPH();
void controlFeeding();
void logData();
String getTelemetryJSON();
void emergencyStop();
void setupOTA();
void loadSettings();
void saveSettings();

void setup() {
  Serial.begin(115200);

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

  // Initialize Pins
  pinMode(PUMP_ACID_PIN, OUTPUT);
  pinMode(PUMP_BASE_PIN, OUTPUT);
  pinMode(PUMP_NUTRIENT_PIN, OUTPUT);
  pinMode(OD_LIGHT_PIN, OUTPUT);
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
    StaticJsonDocument<200> doc;
    doc["phTarget"] = phTarget;
    doc["phHysteresis"] = phHysteresis;
    doc["feedingInterval"] = feedingInterval;
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    StaticJsonDocument<200> doc;
    deserializeJson(doc, (const char*)data);
    if(doc.containsKey("phTarget")) phTarget = doc["phTarget"];
    if(doc.containsKey("phHysteresis")) phHysteresis = doc["phHysteresis"];
    if(doc.containsKey("feedingInterval")) feedingInterval = doc["feedingInterval"];
    saveSettings();
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  setupOTA();
}

void loop() {
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();

  // Sensor Reading (Non-blocking)
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    updateSensors();
    lastSensorRead = currentMillis;

    // Control logic dependent on fresh sensor data
    if (!sensorError) {
      controlPH();
    } else {
      emergencyStop();
    }
  }

  // Feeding logic (Non-blocking)
  controlFeeding();

  // Data Logging (Non-blocking)
  if (currentMillis - lastLogTime >= LOG_INTERVAL_MS) {
    logData();
    lastLogTime = currentMillis;
  }

  // Heartbeat LED
  static unsigned long lastToggle = 0;
  if (currentMillis - lastToggle >= 500) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastToggle = currentMillis;
  }
}

void updateSensors() {
  unsigned long currentMillis = millis();

  // Read Temperature
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  if (temp != DEVICE_DISCONNECTED_C) {
    currentTemp = temp;
  } else {
    sensorError = true;
  }

  // Read pH
  int16_t phRaw = ads.readADC_SingleEnded(ADS_PH_CH);
  if (phRaw == -1) { // Basic check for ADS failure
     sensorError = true;
     return;
  }
  // 0.0001875 is the voltage step for +/- 6.144V range
  currentPH = 7.0 + ((float)phRaw * 0.0001875);

  // Read OD
  digitalWrite(OD_LIGHT_PIN, HIGH);
  // We'll use a small delay here as it's very short and inside a 2s interval loop
  // but for pure non-blocking, we could split this into two states.
  delayMicroseconds(500);
  int16_t odRaw = ads.readADC_SingleEnded(ADS_OD_CH);
  digitalWrite(OD_LIGHT_PIN, LOW);

  // OD Calculation with guard
  if (odRaw > 10) { // Avoid division by zero or log of zero
    currentOD = log10(32768.0 / (float)odRaw) * OD_CALIBRATION_FACTOR;
  } else {
    currentOD = 4.0; // Max out at high density
  }

  // Growth Rate Estimation (Specific Growth Rate mu)
  // mu = (ln(OD2) - ln(OD1)) / (t2 - t1)
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

  // Proportional control with hysteresis
  if (abs(error) < phHysteresis) {
    digitalWrite(PUMP_ACID_PIN, LOW);
    digitalWrite(PUMP_BASE_PIN, LOW);
    phIntegral = 0; // Reset integral when in target
    return;
  }

  // Simple PWM-like control for pumps to prevent overshooting
  // We'll use a fixed cycle of 10 seconds for pump titration
  static unsigned long lastCycleStart = 0;
  unsigned long cycleTime = millis() - lastCycleStart;
  if (cycleTime >= 10000) {
    lastCycleStart = millis();
    cycleTime = 0;
  }

  // Duration proportional to error (max 5s per 10s cycle)
  unsigned long pumpDuration = min((unsigned long)(abs(error) * Kp), 5000UL);

  if (error > 0) { // Too basic, need acid
    if (cycleTime < pumpDuration) digitalWrite(PUMP_ACID_PIN, HIGH);
    else digitalWrite(PUMP_ACID_PIN, LOW);
    digitalWrite(PUMP_BASE_PIN, LOW);
  } else { // Too acidic, need base
    if (cycleTime < pumpDuration) digitalWrite(PUMP_BASE_PIN, HIGH);
    else digitalWrite(PUMP_BASE_PIN, LOW);
    digitalWrite(PUMP_ACID_PIN, LOW);
  }
}

void controlFeeding() {
  unsigned long currentMillis = millis();

  // Trigger scheduled feeding
  if (currentMillis - lastFeedingTime > (unsigned long)feedingInterval) {
    nutrientPumpActive = true;
    nutrientPumpStartTime = currentMillis;
    digitalWrite(PUMP_NUTRIENT_PIN, HIGH);
    lastFeedingTime = currentMillis;
  }

  // Handle pump duration
  if (nutrientPumpActive && (currentMillis - nutrientPumpStartTime >= FEEDING_DURATION_MS)) {
    digitalWrite(PUMP_NUTRIENT_PIN, LOW);
    nutrientPumpActive = false;
  }
}

void logData() {
  if (sensorError) return;

  File file = SPIFFS.open("/log.csv", FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }

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

void emergencyStop() {
  digitalWrite(PUMP_ACID_PIN, LOW);
  digitalWrite(PUMP_BASE_PIN, LOW);
  digitalWrite(PUMP_NUTRIENT_PIN, LOW);
}

void loadSettings() {
  File file = SPIFFS.open("/settings.json", FILE_READ);
  if(!file) return;
  StaticJsonDocument<200> doc;
  deserializeJson(doc, file);
  phTarget = doc["phTarget"] | PH_TARGET;
  phHysteresis = doc["phHysteresis"] | PH_HYSTERESIS;
  feedingInterval = doc["feedingInterval"] | FEEDING_INTERVAL_MS;
  file.close();
}

void saveSettings() {
  File file = SPIFFS.open("/settings.json", FILE_WRITE);
  if(!file) return;
  StaticJsonDocument<200> doc;
  doc["phTarget"] = phTarget;
  doc["phHysteresis"] = phHysteresis;
  doc["feedingInterval"] = feedingInterval;
  serializeJson(doc, file);
  file.close();
}

void setupOTA() {
  ArduinoOTA.setHostname("biofermenter-esp32s3");
  // ArduinoOTA.setPassword("admin"); // Optional

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
    else type = "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
}

String getTelemetryJSON() {
  StaticJsonDocument<200> doc;
  DateTime now = rtc.now();

  doc["ph"] = currentPH;
  doc["od"] = currentOD;
  doc["temp"] = currentTemp;
  doc["mu"] = growthRate;
  doc["timestamp"] = now.timestamp();
  doc["error"] = sensorError;

  String output;
  serializeJson(doc, output);
  return output;
}
