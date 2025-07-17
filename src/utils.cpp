#include "utils.h"
#include "env.h"

#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/adc.h"
#include "esp_wifi.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "driver/periph_ctrl.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

String log_buffer = "";

/**
 * Logs a message to both the serial output and an internal log buffer
 */
void serial_log(String message)
{
    log_buffer += message + "\n";
    Serial.println(message);
}

/**
 * Calculates the dew point temperature from temperature and humidity
 *
 * Uses the Magnus formula to calculate the dew point temperature. The dew point is the temperature
 * the air needs to be cooled to (at constant pressure) in order to produce a relative humidity of
 * 100%.
 *
 * Reference: https://en.wikipedia.org/wiki/Dew_point
 *
 * @param temperature Temperature in degrees Celsius
 * @param humidity Relative humidity as a percentage (0-100)
 * @return Dew point temperature in degrees Celsius
 */
float calculate_dew_point(float temperature, float humidity)
{
    const float b = 17.62;
    const float c = 243.12;
    float alpha = ((b * temperature) / (c + temperature)) + log(humidity / 100.0);
    return (c * alpha) / (b - alpha);
}

/**
 * Isolates all RTC-capable GPIO pins to reduce power consumption
 *
 * This function isolates all GPIO pins that can be controlled by the RTC during
 * deep sleep mode. Isolation prevents current leakage and reduces power consumption
 * when the ESP32 enters deep sleep mode. This is essential for battery-powered
 * applications to maximize battery life.
 */
void isolate_all_rtc_gpio()
{
    const gpio_num_t rtc_gpio_list[] = { GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_4, GPIO_NUM_12,
        GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_32,
        GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39 };

    for (int i = 0; i < sizeof(rtc_gpio_list) / sizeof(rtc_gpio_list[0]); i++) {
        rtc_gpio_isolate(rtc_gpio_list[i]);
    }
}

/**
 * Establishes a WiFi connection using credentials from env.h
 */
void connect_to_wifi()
{
    serial_log("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            serial_log("\nWiFi connected!");
            serial_log(WiFi.localIP().toString());
            return;
        }
        if (WiFi.status() == WL_NO_SSID_AVAIL) {
            Serial.println("SSID not found!");
            Serial.println("Entering deep sleep for 5 minutes.");
            esp_sleep_enable_timer_wakeup(300000000);
            esp_deep_sleep_start();
        }
        delay(500);
        serial_log(".");
    }
    
    Serial.println("Response: " + String(WiFi.status()));
    ESP.restart();
}

/**
 * Sends accumulated log messages to a remote log server
 *
 * @note Connection failures are logged but do not block execution
 */
void send_log()
{
    WiFiClient client;
    if (client.connect(LOG_SERVER_HOST, LOG_SERVER_PORT)) {
        int contentLength = log_buffer.length();
        client.print(String("POST ") + LOG_SERVER_PATH + " HTTP/1.1\r\n" + "Host: "
            + LOG_SERVER_HOST + "\r\n" + "Content-Type: text/plain\r\n" + "Content-Length: "
            + String(contentLength) + "\r\n" + "Connection: close\r\n\r\n" + log_buffer);

        delay(10);
        client.stop();

        serial_log("Log sent synchronously (no response expected).");
    } else {
        serial_log("Failed to connect to the log server.");
    }
}

/**
 * Sends weather sensor data to a remote database via HTTP GET request
 *
 * @param temperature Temperature reading in degrees Celsius
 * @param humidity Relative humidity as a percentage (0-100)
 * @param pressure Atmospheric pressure in appropriate units
 * @param dew_point Calculated dew point temperature in degrees Celsius
 * @param illumination Light level reading from light sensor
 * @param battery_voltage Current battery voltage level
 * @param solar_panel_voltage Solar panel output voltage
 */
void send_to_database(float temperature, float humidity, float pressure, float dew_point,
    float illumination, float battery_voltage, float solar_panel_voltage)
{
    HTTPClient http;
    String url = String(TEST_SERVER_HOST) + ":" + String(TEST_SERVER_PORT) + "/api/weather"
        + "?temperature=" + String(temperature, 2) + "&dew_point=" + String(dew_point, 2)
        + "&humidity=" + String(humidity, 1) + "&illumination=" + String(illumination, 1)
        + "&pressure=" + String(pressure, 2) + "&battery_voltage=" + String(battery_voltage, 2)
        + "&solar_panel_voltage=" + String(solar_panel_voltage, 2);

    serial_log("Sending to: " + url);
    http.begin(url);
    int http_code = http.GET();
    if (http_code > 0) {
        serial_log("Response: " + http.getString());
    } else {
        serial_log("Error on sending request");
    }
    http.end();
}

/**
 * Optimizes CPU frequency for different operation phases
 * 
 * @param high_performance If true, sets CPU to maximum frequency for WiFi/sensor operations.
 *                        If false, reduces CPU frequency to save power during low-intensity operations.
 */
void optimize_cpu_frequency(bool high_performance)
{
    if (high_performance) {
        // Set CPU to maximum frequency for WiFi and sensor operations
        setCpuFrequencyMhz(CPU_FREQ_HIGH_PERFORMANCE);
        serial_log("CPU frequency set to " + String(CPU_FREQ_HIGH_PERFORMANCE) + "MHz for high performance operations");
    } else {
        // Reduce CPU frequency based on power profile
        uint8_t low_power_freq;
        switch (POWER_PROFILE) {
            case POWER_PROFILE_AGGRESSIVE:
                low_power_freq = CPU_FREQ_AGGRESSIVE;
                break;
            case POWER_PROFILE_PERFORMANCE:
                low_power_freq = 160; // Higher baseline for performance profile
                break;
            default: // POWER_PROFILE_BALANCED
                low_power_freq = CPU_FREQ_LOW_POWER;
                break;
        }
        setCpuFrequencyMhz(low_power_freq);
        serial_log("CPU frequency set to " + String(low_power_freq) + "MHz for power saving (profile: " + String(POWER_PROFILE) + ")");
    }
}

/**
 * Disables unused peripherals to reduce power consumption
 * 
 * This function disables peripherals that are not used by the weather station
 * to minimize power consumption during active operation. The aggressiveness
 * depends on the configured power profile.
 */
void disable_unused_peripherals()
{
    // Always disable these unused peripherals
    periph_module_disable(PERIPH_SPI_MODULE);       // SPI not used
    periph_module_disable(PERIPH_I2S0_MODULE);      // I2S not used
    periph_module_disable(PERIPH_I2S1_MODULE);      // I2S not used
    periph_module_disable(PERIPH_UART1_MODULE);     // Only UART0 used for Serial
    periph_module_disable(PERIPH_UART2_MODULE);     // Not used
    periph_module_disable(PERIPH_SDMMC_MODULE);     // SD card not used
    periph_module_disable(PERIPH_PCNT_MODULE);      // Pulse counter not used
    periph_module_disable(PERIPH_RMT_MODULE);       // Remote control not used
    periph_module_disable(PERIPH_UHCI0_MODULE);     // UHCI not used
    periph_module_disable(PERIPH_UHCI1_MODULE);     // UHCI not used
    
    // Additional power saving for aggressive mode
    if (POWER_PROFILE == POWER_PROFILE_AGGRESSIVE) {
        periph_module_disable(PERIPH_LEDC_MODULE);   // LED PWM not used in aggressive mode
        serial_log("Unused peripherals disabled for aggressive power optimization");
    } else if (POWER_PROFILE == POWER_PROFILE_BALANCED) {
        periph_module_disable(PERIPH_LEDC_MODULE);   // LED PWM not used
        serial_log("Unused peripherals disabled for balanced power optimization");
    } else {
        serial_log("Basic unused peripherals disabled for performance mode");
    }
}

/**
 * Enables WiFi power save mode to reduce power consumption during WiFi operations
 * The aggressiveness of power saving depends on the configured power profile.
 */
void enable_wifi_power_save()
{
    switch (POWER_PROFILE) {
        case POWER_PROFILE_AGGRESSIVE:
            esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
            serial_log("WiFi aggressive power save mode enabled");
            break;
        case POWER_PROFILE_PERFORMANCE:
            esp_wifi_set_ps(WIFI_PS_NONE);
            serial_log("WiFi power save disabled for performance mode");
            break;
        default: // POWER_PROFILE_BALANCED
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
            serial_log("WiFi balanced power save mode enabled");
            break;
    }
}

/**
 * Optimizes ADC power consumption
 * 
 * Powers down ADC components after voltage measurements are complete
 * to reduce standby power consumption.
 */
void optimize_adc_power()
{
    // Power down ADC after measurements
    adc_power_release();
    serial_log("ADC powered down for energy optimization");
}

/**
 * Prepares the system for deep sleep with comprehensive power optimizations
 * 
 * This function performs additional power optimizations beyond the existing
 * RTC GPIO isolation and WiFi shutdown. The level of optimization depends
 * on the configured power profile.
 */
void prepare_for_deep_sleep()
{
    // Always optimize flash power during deep sleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    
    // Configure RTC peripherals based on power profile
    if (POWER_PROFILE == POWER_PROFILE_AGGRESSIVE) {
        // Most aggressive power saving
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
        serial_log("System prepared for aggressive deep sleep optimization");
    } else if (POWER_PROFILE == POWER_PROFILE_BALANCED) {
        // Balanced optimization
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
        serial_log("System prepared for balanced deep sleep optimization");
    } else {
        // Performance mode - minimal deep sleep optimization
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
        serial_log("System prepared for performance-oriented deep sleep");
    }
}

/**
 * Logs the current power profile configuration for debugging and monitoring
 */
void log_power_profile()
{
    String profile_name;
    switch (POWER_PROFILE) {
        case POWER_PROFILE_AGGRESSIVE:
            profile_name = "AGGRESSIVE";
            break;
        case POWER_PROFILE_PERFORMANCE:
            profile_name = "PERFORMANCE";
            break;
        default:
            profile_name = "BALANCED";
            break;
    }
    
    serial_log("Power Profile: " + profile_name + " (" + String(POWER_PROFILE) + ")");
    serial_log("CPU Frequencies - Low: " + String(
        POWER_PROFILE == POWER_PROFILE_AGGRESSIVE ? CPU_FREQ_AGGRESSIVE : 
        POWER_PROFILE == POWER_PROFILE_PERFORMANCE ? 160 : CPU_FREQ_LOW_POWER
    ) + "MHz, High: " + String(CPU_FREQ_HIGH_PERFORMANCE) + "MHz");
}
