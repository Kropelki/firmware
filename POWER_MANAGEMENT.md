# Power Management Optimization

This firmware includes comprehensive power management optimizations to maximize battery life for the ESP32 weather station.

## Power Management Features

### 1. Deep Sleep Mode
- **Primary power saving mechanism**: Puts ESP32 into deep sleep between measurements
- **Default interval**: 300 seconds (5 minutes)
- **Wake-up trigger**: Timer-based automatic wake-up

### 2. RTC GPIO Isolation
- **Function**: Isolates all RTC-capable GPIO pins during deep sleep
- **Purpose**: Prevents current leakage through GPIO pins
- **Implementation**: Automatically applied before entering deep sleep

### 3. CPU Frequency Scaling
- **Dynamic frequency adjustment**: CPU frequency is adjusted based on operation phase
- **High performance mode**: 240MHz for WiFi operations and data transmission
- **Low power mode**: Varies by power profile (40-160MHz)
- **Automatic switching**: Frequency changes automatically based on current task

### 4. Peripheral Power Management
- **Unused peripheral shutdown**: Disables SPI, I2S, extra UARTs, and other unused modules
- **Profile-dependent optimization**: Aggressiveness varies by power profile
- **Always disabled**: SPI, I2S0/1, UART1/2, SDMMC, PCNT, RMT, UHCI modules

### 5. WiFi Power Optimization
- **Profile-based power saving**: Different WiFi power modes based on configuration
- **Automatic shutdown**: WiFi completely disabled before deep sleep
- **Connection optimization**: WiFi power save during active operations

### 6. ADC Power Management
- **Power-down after use**: ADC is powered down immediately after voltage readings
- **Reduces standby consumption**: Minimizes power draw from analog components

### 7. Flash Memory Optimization
- **Deep sleep optimization**: Flash memory configured for low power during sleep
- **Profile-dependent**: More aggressive optimization in power-saving profiles

## Power Profiles

The firmware supports three configurable power profiles in `env.h`:

### POWER_PROFILE_AGGRESSIVE (1)
- **CPU Low Power**: 40MHz
- **WiFi Mode**: Maximum power save (WIFI_PS_MAX_MODEM)
- **Peripherals**: Most aggressive shutdown
- **Deep Sleep**: Maximum power optimization
- **Use Case**: Maximum battery life, slower operation

### POWER_PROFILE_BALANCED (2) - Default
- **CPU Low Power**: 80MHz
- **WiFi Mode**: Minimum power save (WIFI_PS_MIN_MODEM)
- **Peripherals**: Standard shutdown
- **Deep Sleep**: Balanced optimization
- **Use Case**: Good balance of performance and battery life

### POWER_PROFILE_PERFORMANCE (3)
- **CPU Low Power**: 160MHz
- **WiFi Mode**: No power save (WIFI_PS_NONE)
- **Peripherals**: Minimal shutdown
- **Deep Sleep**: Performance-oriented
- **Use Case**: Fastest operation, higher power consumption

## Configuration

To configure power management, edit the `env.h` file:

```c
// Set the desired power profile
#define POWER_PROFILE POWER_PROFILE_BALANCED

// Customize CPU frequencies if needed
#define CPU_FREQ_LOW_POWER 80
#define CPU_FREQ_HIGH_PERFORMANCE 240
#define CPU_FREQ_AGGRESSIVE 40
```

## Power Consumption Monitoring

The firmware includes power consumption monitoring features:

- **Active Time Logging**: Records how long the device stays active each cycle
- **Voltage Monitoring**: Tracks battery and solar panel voltages
- **Profile Logging**: Reports current power profile configuration
- **Power State Transitions**: Logs when CPU frequency and power modes change

## Expected Power Savings

Based on ESP32 power consumption characteristics:

- **Deep Sleep**: ~10-150µA (depending on profile and enabled features)
- **Active Operation**: Reduced by 20-40% through frequency scaling and peripheral management
- **WiFi Operations**: 10-30% reduction through power save modes
- **Overall**: Estimated 25-50% improvement in battery life compared to unoptimized firmware

## Implementation Details

### CPU Frequency Scaling
```c
// Low power mode for calculations and sensor reading
optimize_cpu_frequency(false);

// High performance mode for WiFi operations
optimize_cpu_frequency(true);
```

### Power Profile Usage
The power profile affects:
- CPU frequencies during low-power operations
- WiFi power save aggressiveness
- Peripheral shutdown strategy
- Deep sleep optimization level

### Monitoring and Debugging
All power management actions are logged to the serial output and included in the log buffer sent to the remote log server, allowing for power consumption analysis and optimization verification.