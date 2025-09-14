#include <Adafruit_ADS1X15.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <WiFi.h>
#include <Wire.h>
#include <memory>

#include "env.h"
#include "influxdb.h"
#include "measurement.h"
#include "utils.h"
#include "wunderground.h"

#define MOSFET_PIN 12

void setup()
{
    unsigned long startTime = millis();
    btStop();

    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, HIGH);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    Serial.begin(115200);
    while (!Serial) {
        delay(20);
    }

    Wire.begin(21, 22); // SDA, SCL
    connect_to_wifi();

    Adafruit_ADS1115 ads_sensor; // ADS1115: measures analog inputs
    Adafruit_AHTX0 aht_sensor; // AHT20: measures temperature and humidity
    Adafruit_BMP280 bmp_sensor; // BMP280: measures pressure
    BH1750 light_meter; // BH1750: measures illumination
    Measurement measurement; // holds all sensor data

    measurement.read_sensors_and_voltage(bmp_sensor, aht_sensor, light_meter, ads_sensor);
    measurement.remove_invalid_measurements();
    measurement.calculate_derived_values();
    measurement.print_all_values();

    unsigned long activeTime = (millis() - startTime) / 1000;

    if (SEND_TO_EXTERNAL_SERVICES) {
        if (measurement.has_sensor_data()) {
            send_to_wunderground(measurement);
            send_to_influx_db(measurement);
        } else {
            serial_log("No sensor data available - skipping external services.");
        }
    } else {
        serial_log("External services sending is disabled.");
    }

    send_log();

    isolate_all_rtc_gpio();
    WiFi.mode(WIFI_OFF);

    unsigned long sleepTime = (activeTime < CYCLE_TIME_SEC) ? ((CYCLE_TIME_SEC - activeTime))
                                                            : (CYCLE_TIME_SEC); // ensure we don't get huge sleep times
    serial_log("Entering deep sleep for " + String(sleepTime) + " seconds...");

    esp_sleep_enable_timer_wakeup(sleepTime * 1000000); // convert to microseconds
    esp_deep_sleep_start();
}

void loop() { }
