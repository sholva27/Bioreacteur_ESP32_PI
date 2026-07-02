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
float currentPressure_V = 0.0; // Headspace pressure voltage
float currentFluo = 0.0; // NADH Fluorescence intensity (460nm)
float currentRibo = 0.0;  // Riboflavine intensity (520nm)
float growthRate = 0.0;  // Specific growth rate (mu)

float lastOD = 0.0;
unsigned long lastODTime = 0;

bool sensorError = false;
unsigned long lastSensorRead = 0;
unsigned long lastFeedingTime = 0;
unsigned long lastLogTime = 0;
unsigned long lastFluoRead = 0;

// Pump states (for non-blocking duration)
struct PumpState {
  int pin;
  bool active;
  unsigned long startTime;
  unsigned long duration;
};

PumpState acidPump = {PUMP_ACID_PIN, false, 0, 0};
PumpState basePump = {PUMP_BASE_PIN, false, 0, 0};
PumpState nutrientPump = {PUMP_NUTRIENT_PIN, false, 0, 0};

void startPump(PumpState &p, unsigned long duration) {
  if (duration > PUMP_MAX_MS) duration = PUMP_MAX_MS;
  p.duration = duration;
  p.startTime = millis();
  p.active = true;
  // Note: digitalWrite happens in updatePumps() to ensure single actuation authority
}

void updatePumps() {
  unsigned long now = millis();
  PumpState* pumps[] = {&acidPump, &basePump, &nutrientPump};
  for (int i = 0; i < 3; i++) {
    if (pumps[i]->active) {
      if (now - pumps[i]->startTime >= pumps[i]->duration) {
        digitalWrite(pumps[i]->pin, LOW);
        pumps[i]->active = false;
      } else {
        digitalWrite(pumps[i]->pin, HIGH);
      }
    } else {
      // Ensure pin is LOW if not active, but respect other control logic (e.g. PID)
      // Actually, for single authority, we should probably merge PID into this or
      // have PID only set the PumpState.
    }
  }
}

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
#if USE_ADS1115
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115.");
    sensorError = true;
  } else {
    Serial.println("ADS1115 Initialized.");
  }
#endif

#if USE_RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    sensorError = true;
  }
#endif

#if USE_TEMP_SENSOR
  sensors.begin();
  sensors.setWaitForConversion(false); // Non-blocking temperature reads
  sensors.requestTemperatures();       // Start first conversion
#endif

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

  // WiFi Setup (Non-blocking attempt)
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed (timeout). Running in offline mode.");
  }

  // Web Server Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getTelemetryJSON());
  });

  server.on("/download_log", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
    request->send(SPIFFS, "/log.csv", "text/csv", true);
  });

  server.on("/pump", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
    if(request->hasParam("type", true)) {
      String type = request->getParam("type", true)->value();
      long duration = FEEDING_DURATION_MS;
      if(request->hasParam("duration", true)) {
        duration = request->getParam("duration", true)->value().toInt();
      }

      if(type == "nutrient") startPump(nutrientPump, duration);
      else if(type == "acid") startPump(acidPump, duration);
      else if(type == "base") startPump(basePump, duration);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
    JsonDocument doc;
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

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return request->requestAuthentication();
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!request->authenticate(WEB_USER, WEB_PASS)) return;
    JsonDocument doc;
    deserializeJson(doc, (const char*)data, len);
    if(doc["phTarget"].is<float>()) phTarget = doc["phTarget"];
    if(doc["phHysteresis"].is<float>()) phHysteresis = doc["phHysteresis"];
    if(doc["tempTarget"].is<float>()) tempTarget = doc["tempTarget"];
    if(doc["stirrerSpeed"].is<int>()) stirrerSpeed = doc["stirrerSpeed"];
    if(doc["feedingInterval"].is<long>()) feedingInterval = doc["feedingInterval"];
    if(doc["kp"].is<float>()) Kp = doc["kp"];
    if(doc["ki"].is<float>()) Ki = doc["ki"];
    if(doc["kd"].is<float>()) Kd = doc["kd"];
    if(doc["mqttBroker"].is<String>()) mqttBroker = doc["mqttBroker"].as<String>();
    if(doc["mqttEnabled"].is<bool>()) mqttEnabled = doc["mqttEnabled"];
    if(doc["phOffset"].is<float>()) phOffset = doc["phOffset"];
    if(doc["phSlope"].is<float>()) phSlope = doc["phSlope"];
    if(doc["odZero"].is<float>()) odZeroVoltage = doc["odZero"];
    saveSettings();
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  setupOTA();
  setupMQTT();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
    if (mqttEnabled) {
      if (!mqttClient.connected()) reconnectMQTT();
      mqttClient.loop();
    }
  } else {
    // Reconnect logic
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      Serial.println("Attempting to reconnect to WiFi...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      lastReconnectAttempt = millis();
    }
  }
  unsigned long currentMillis = millis();

  // Sensor Reading (Non-blocking)
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    updateSensors();
    lastSensorRead = currentMillis;

    // Control logic dependent on fresh sensor data
#if USE_ACTUATORS
    if (!sensorError) {
      controlPH();
      controlTemp();
    } else {
      emergencyStop();
    }
#endif
  }

  // Manual touch button logic
  if (digitalRead(TOUCH_BUTTON_PIN) == HIGH && !nutrientPump.active) {
    startPump(nutrientPump, FEEDING_DURATION_MS);
  }

  // Pump service (Non-blocking)
  updatePumps();

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
  // Read Metabolic Activity via Fluorescence (340nm exc)
  digitalWrite(FLUO_LED_PIN, HIGH);
  delayMicroseconds(1000); // Allow LED and sensor to stabilize

  // AS7341 Spectral Sensor Logic (I2C) - Preferred for LAB cultures (NADH + Riboflavin)
  // These are logic hooks; implementation requires Adafruit_AS7341 library.
  /*
  as7341.setLED(true); // Internal LED or use external via FLUO_LED_PIN
  currentFluo = as7341.readChannel(AS7341_CHANNEL_460nm); // F3 (NADH)
  currentRibo = as7341.readChannel(AS7341_CHANNEL_520nm); // F5 (Riboflavin)
  as7341.setLED(false);
  */

  // Note: ADS1115 Channels 2 and 3 are used for Pressure and UV.
  // If using analog fluorescence sensors, prioritize channels in config.h.

  digitalWrite(FLUO_LED_PIN, LOW);
}

bool checkI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

void updateSensors() {
  unsigned long currentMillis = millis();
  sensorError = false; // Reset error flag at start of cycle

#if USE_ADS1115
  if (!checkI2C(0x48)) {
    Serial.println("ADS1115 I2C disconnect detected!");
    sensorError = true;
  }
#endif

#if USE_TEMP_SENSOR
  // Read Temperature result from previous request
  float temp = sensors.getTempCByIndex(0);
#elif USE_MLX90614
  // Logic hook for MLX90614 (Adafruit_MLX90614 library)
  // float temp = mlx.readObjectTempC();
  float temp = currentTemp; // Placeholder
#endif

#if USE_TEMP_SENSOR || USE_MLX90614
  if (temp != DEVICE_DISCONNECTED_C && temp > 0) {
    currentTemp = temp;
  } else {
    sensorError = true;
  }
  #if USE_TEMP_SENSOR
  // Request temperature for next cycle
  sensors.requestTemperatures();
  #endif
#endif

#if USE_ADS1115
  if (!sensorError) {
    #if USE_UV_SENSOR
    // Read UV Intensity (ADS Channel 3)
    int16_t uvRaw = ads.readADC_SingleEnded(ADS_UV_CH);
    currentUV_V = (float)uvRaw * 0.0001875;
    if (VERBOSE_ADC) Serial.printf("UV_V: %.4fV (%d)\n", currentUV_V, uvRaw);
    #endif

    #if USE_PRESSURE_SENSOR
    // Read Pressure (ADS Channel 2)
    int16_t pressureRaw = ads.readADC_SingleEnded(ADS_PRESSURE_CH);
    currentPressure_V = (float)pressureRaw * 0.0001875;
    if (VERBOSE_ADC) Serial.printf("PRES_V: %.4fV (%d)\n", currentPressure_V, pressureRaw);
    #endif

    #if USE_PH_PROBE
    // Read pH
    int16_t phRaw = ads.readADC_SingleEnded(ADS_PH_CH);
    currentPH_V = (float)phRaw * 0.0001875;
    // Nernstian formula: pH = 7.0 + (V_7.0 - V_measured) / (Slope_V)
    // We use phSlope as a multiplier for the default slope (e.g., 1.0 = 59.16 mV/pH)
    // We use phOffset as a shift in pH units.
    float effectiveSlope_V = (PH_SLOPE_MV / 1000.0) * phSlope;
    currentPH = 7.0 + (PH_VMID - currentPH_V) / effectiveSlope_V + phOffset;

    if (VERBOSE_ADC) Serial.printf("PH_V: %.4fV -> PH: %.2f\n", currentPH_V, currentPH);
    #endif

  } else {
    sensorError = true;
  }
#endif

#if USE_ADS1115
  #if USE_OD_SENSOR
  // Read OD
  digitalWrite(OD_LIGHT_PIN, HIGH);
  delayMicroseconds(500);
  int16_t odRaw = ads.readADC_SingleEnded(ADS_OD_CH);
  digitalWrite(OD_LIGHT_PIN, LOW);

  // OD Calculation with guard
  currentOD_V = (float)odRaw * 0.0001875;
  if (VERBOSE_ADC) Serial.printf("OD_V: %.4fV\n", currentOD_V);
  if (currentOD_V > 0.001) {
    if (odZeroVoltage > 0.1) {
      currentOD = log10(odZeroVoltage / currentOD_V) * OD_CALIBRATION_FACTOR;
    } else {
      currentOD = 0.0;
    }
  } else {
    currentOD = 4.0; // Max out at high density
  }
  #endif
#endif

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
}

void controlPH() {
  float error = currentPH - phTarget;
  unsigned long currentMillis = millis();
  static unsigned long lastUpdate = 0;
  float dt = (currentMillis - lastUpdate) / 1000.0;

  if (abs(error) < phHysteresis) {
    // If not manually active, ensure OFF
    if (!acidPump.active) digitalWrite(PUMP_ACID_PIN, LOW);
    if (!basePump.active) digitalWrite(PUMP_BASE_PIN, LOW);
    phIntegral = 0;
    lastPhError = error;
    lastUpdate = currentMillis;
    return;
  }

  if (dt > 0) {
    phIntegral += error * dt;
    phIntegral = constrain(phIntegral, -50.0, 50.0); // Simple anti-windup cap
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

    if (error > 0) { // Need Acid
      if (!acidPump.active) {
        if (cycleTime < pumpDuration) digitalWrite(PUMP_ACID_PIN, HIGH);
        else digitalWrite(PUMP_ACID_PIN, LOW);
      }
      if (!basePump.active) digitalWrite(PUMP_BASE_PIN, LOW);
    } else { // Need Base
      if (!basePump.active) {
        if (cycleTime < pumpDuration) digitalWrite(PUMP_BASE_PIN, HIGH);
        else digitalWrite(PUMP_BASE_PIN, LOW);
      }
      if (!acidPump.active) digitalWrite(PUMP_ACID_PIN, LOW);
    }

    lastPhError = error;
    lastUpdate = currentMillis;
  }
}

void controlFeeding() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastFeedingTime > (unsigned long)feedingInterval) {
    startPump(nutrientPump, FEEDING_DURATION_MS);
    lastFeedingTime = currentMillis;
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
  mqttClient.setBufferSize(512);
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
  JsonDocument doc;
  deserializeJson(doc, payload, length);
  if (doc["command"].is<String>()) {
    String cmd = doc["command"];
    if (cmd == "feed") {
      startPump(nutrientPump, FEEDING_DURATION_MS);
    }
  }
}

void emergencyStop() {
  digitalWrite(PUMP_ACID_PIN, LOW);
  digitalWrite(PUMP_BASE_PIN, LOW);
  digitalWrite(PUMP_NUTRIENT_PIN, LOW);
  digitalWrite(HEATER_PIN, LOW);
  analogWrite(STIRRER_PIN, 0);

  acidPump.active = false;
  basePump.active = false;
  nutrientPump.active = false;
}

void loadSettings() {
  // Logic Hook: Load high-frequency state from AT24C256 EEPROM if available
  // e.g., currentPumpRuntime = eeprom.read(ADDR_PUMP_STATE);

  File file = SPIFFS.open("/settings.json", FILE_READ);
  if(!file) return;
  JsonDocument doc;
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
  // Logic Hook: Save high-frequency state to AT24C256 EEPROM
  // e.g., eeprom.write(ADDR_PUMP_STATE, currentPumpRuntime);

  File file = SPIFFS.open("/settings.json", FILE_WRITE);
  if(!file) return;
  JsonDocument doc;
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
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.begin();
}

String getTelemetryJSON() {
  JsonDocument doc;
  DateTime now = rtc.now();
  doc["ph"] = currentPH;
  doc["od"] = currentOD;
  doc["ph_v"] = currentPH_V;
  doc["od_v"] = currentOD_V;
  doc["temp"] = currentTemp;
  doc["mu"] = growthRate;
  doc["fluo"] = currentFluo;
  doc["ribo"] = currentRibo;
  doc["uv_v"] = currentUV_V;
  doc["pres_v"] = currentPressure_V;
  doc["timestamp"] = now.timestamp();
  doc["error"] = sensorError;
  String output;
  serializeJson(doc, output);
  return output;
}
