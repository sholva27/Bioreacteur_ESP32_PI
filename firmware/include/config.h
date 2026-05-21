#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Pin Assignments (ESP32-S3)
#define I2C_SDA 8
#define I2C_SCL 9

#define PUMP_ACID_PIN 10
#define PUMP_BASE_PIN 11
#define PUMP_NUTRIENT_PIN 12
#define OD_LIGHT_PIN 13
#define TEMP_SENSOR_PIN 14
#define HEATER_PIN 15
#define STIRRER_PIN 16
#define TOUCH_BUTTON_PIN 17
#define FLUO_LED_PIN 18

#define STATUS_LED 21

// ADS1115 Channels
#define ADS_PH_CH 0
#define ADS_OD_CH 1
#define ADS_PRESSURE_CH 2
#define ADS_UV_CH 3

// System Constants
#define PH_TARGET 7.0
#define PH_HYSTERESIS 0.1
#define TEMP_TARGET 37.0
#define STIRRER_SPEED_DEFAULT 128
#define FEEDING_INTERVAL_MS 3600000 // 1 hour
#define FEEDING_DURATION_MS 5000     // 5 seconds
#define OD_CALIBRATION_FACTOR 1.0
#define SENSOR_READ_INTERVAL_MS 2000
#define LOG_INTERVAL_MS 60000        // 1 minute
#define FLUO_READ_INTERVAL_MS 5000   // 5 seconds

// MQTT Settings
#define MQTT_BROKER "Your_MQTT_Broker_IP"
#define MQTT_PORT 1883
#define MQTT_TOPIC_PREFIX "biofermenter/1"

// Debugging & Isolation (Set to 0 to disable a module for probe-by-probe testing)
#define DEBUG_SERIAL 1      // Main serial output
#define VERBOSE_ADC 0       // Print raw ADC voltages for every read
#define USE_ADS1115 1       // Set to 0 if I2C ADC is not connected
#define USE_RTC 1           // Set to 0 if DS3231 is not connected
#define USE_PH_PROBE 1
#define USE_OD_SENSOR 1
#define USE_TEMP_SENSOR 1
#define USE_PRESSURE_SENSOR 1
#define USE_UV_SENSOR 1
#define USE_ACTUATORS 1     // Set to 0 to disable all pumps/heaters/stirrers for safety

#endif
