#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_ADS1X15.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPIFFS.h>

#include "config.h"
#include "dashboard.html.h"

// Hardware interfaces
Adafruit_ADS1115 ads;
RTC_DS3231 rtc;
AsyncWebServer server(80);

// Global state
float currentPH = 0.0;
float currentOD = 0.0;
bool sensorError = false;
unsigned long lastSensorRead = 0;
unsigned long lastFeedingTime = 0;
unsigned long lastLogTime = 0;

// Pump states (for non-blocking duration)
bool nutrientPumpActive = false;
unsigned long nutrientPumpStartTime = 0;

// Function prototypes
void updateSensors();
void controlPH();
void controlFeeding();
void logData();
String getTelemetryJSON();
void emergencyStop();

void setup() {
  Serial.begin(115200);

  // Initialize Filesystem
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

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

  server.begin();
}

void loop() {
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

  sensorError = false; // Reset if readings successful
}

void controlPH() {
  if (currentPH > (PH_TARGET + PH_HYSTERESIS)) {
    digitalWrite(PUMP_ACID_PIN, HIGH);
    digitalWrite(PUMP_BASE_PIN, LOW);
  } else if (currentPH < (PH_TARGET - PH_HYSTERESIS)) {
    digitalWrite(PUMP_ACID_PIN, LOW);
    digitalWrite(PUMP_BASE_PIN, HIGH);
  } else {
    digitalWrite(PUMP_ACID_PIN, LOW);
    digitalWrite(PUMP_BASE_PIN, LOW);
  }
}

void controlFeeding() {
  unsigned long currentMillis = millis();

  // Trigger scheduled feeding
  if (currentMillis - lastFeedingTime > FEEDING_INTERVAL_MS) {
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
  file.println(currentOD);
  file.close();
}

void emergencyStop() {
  digitalWrite(PUMP_ACID_PIN, LOW);
  digitalWrite(PUMP_BASE_PIN, LOW);
  digitalWrite(PUMP_NUTRIENT_PIN, LOW);
}

String getTelemetryJSON() {
  StaticJsonDocument<200> doc;
  DateTime now = rtc.now();

  doc["ph"] = currentPH;
  doc["od"] = currentOD;
  doc["timestamp"] = now.timestamp();
  doc["error"] = sensorError;

  String output;
  serializeJson(doc, output);
  return output;
}
