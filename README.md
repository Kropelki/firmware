# ESP32 Weather Station Firmware

This repository contains the firmware code for an ESP32-based weather station that measures environmental parameters and transmits data to multiple cloud services. The firmware is written in Arduino C++ and interfaces with multiple sensors to provide weather monitoring.

---

**Multi-sensor data collection:**

- Temperature and humidity (AHT20 sensor)
- Atmospheric pressure (BMP280 sensor)
- Light intensity (BH1750 sensor)
- Calculated dew point

**Data transmission to multiple services:**

- InfluxDB Cloud for time-series data storage
- Weather Underground for public weather sharing
- Custom local server for testing/development
- Local log server for debugging

**Power management:**

- Deep sleep mode between measurements
- Configurable measurement intervals
- Solar panel and battery voltage monitoring
- **Enhanced power optimization:**
  - CPU frequency scaling (40-240MHz based on operation)
  - Unused peripheral shutdown (SPI, I2S, extra UARTs, etc.)
  - WiFi power save modes
  - ADC power management
  - Configurable power profiles (Aggressive/Balanced/Performance)
  - RTC GPIO isolation and comprehensive deep sleep optimization

---

| Parameter | Description | Default |
|-----------|-------------|---------|
| `CYCLE_TIME_SEC` | Measurement interval in seconds | 300 |
| `WIFI_SSID` | WiFi network name | - |
| `WIFI_PASSWORD` | WiFi password | - |
| `INFLUXDB_API_TOKEN` | InfluxDB authentication token | - |
| `WEATHER_UNDERGROUND_STATION_ID` | Weather Underground station ID | - |
| `WEATHER_UNDERGROUND_API_KEY` | Weather Underground API key | - |
| `POWER_PROFILE` | Power management profile (1=Aggressive, 2=Balanced, 3=Performance) | 2 |

For detailed power management configuration, see [POWER_MANAGEMENT.md](POWER_MANAGEMENT.md).

---

InfluxDB Line Protocol

```lp
weather temperature=25.30,humidity=65.2,pressure=1013.25,illumination=450.5,dew_point=18.1,battery_voltage=3.85,solar_panel_voltage=4.12
```

---

The weather station operates in cycles:

1. **Wake up** from deep sleep
2. **Initialize sensors** and WiFi connection
3. **Read sensor data:**
   - Temperature and humidity
   - Atmospheric pressure
   - Light intensity
   - Battery and solar panel voltages
4. **Calculate derived values** (dew point)
5. **Transmit data** to configured services
6. **Send logs** to log server
7. **Enter deep sleep** for the configured interval
