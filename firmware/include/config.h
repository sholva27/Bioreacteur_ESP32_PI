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

#define STATUS_LED 21

// ADS1115 Channels
#define ADS_PH_CH 0
#define ADS_OD_CH 1

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

// MQTT Settings
#define MQTT_BROKER "Your_MQTT_Broker_IP"
#define MQTT_PORT 1883
#define MQTT_TOPIC_PREFIX "biofermenter/1"

#endif
