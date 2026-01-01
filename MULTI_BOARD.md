# Multi-Board Support for ESP32 Monitor

This project now supports multiple ESP32 board variants with easy switching via PlatformIO environments.

## Supported Boards

### ESP32-C3 Super Mini / DevKitM-1
- **Environment**: `esp32c3`
- **Key Features**:
  - WiFi TX power limited to 8.5dBm (hardware power supply limitation)
  - CPU frequency set to 80 MHz for power saving
  - GPIO pins: LED=8, I2C_SDA=6, I2C_SCL=7
  - GPIO8 is a critical strapping pin (see board_config.h for details)

### ESP32 WROOM-32 / DevKit
- **Environment**: `esp32wroom`
- **Key Features**:
  - Full WiFi TX power available (19.5dBm maximum)
  - CPU frequency set to 160 MHz for standard performance
  - GPIO pins: LED=2, I2C_SDA=21, I2C_SCL=22
  - Standard ESP32 DevKit configuration

## How to Use

### Compiling for Specific Board

```bash
# For ESP32-C3
pio run -e esp32c3

# For ESP32 WROOM-32
pio run -e esp32wroom
```

### Uploading to Board

```bash
# USB Upload for ESP32-C3
pio run -e esp32c3 -t upload

# USB Upload for ESP32 WROOM-32
pio run -e esp32wroom -t upload

# OTA Upload (after connected to WiFi)
pio run -e esp32c3 -t upload --upload-port 192.168.x.x
pio run -e esp32wroom -t upload --upload-port 192.168.x.x
```

### Monitoring Serial Output

```bash
# For ESP32-C3
pio device monitor -e esp32c3

# For ESP32 WROOM-32
pio device monitor -e esp32wroom
```

## Architecture

### Board-Specific Configuration
All board-specific settings are centralized in [include/board_config.h](include/board_config.h):

- GPIO pin assignments (LED, I2C)
- WiFi TX power settings
- CPU frequency
- Board-specific notes and limitations

### Conditional Compilation
The build system uses preprocessor directives to select the correct configuration:

- `-DBOARD_ESP32C3` → Selects ESP32-C3 configuration
- `-DBOARD_ESP32_WROOM` → Selects ESP32 WROOM-32 configuration

### LED Control Macros
Board-agnostic LED control is achieved using macros:

```c
LED_ON()   // Turn LED on (handles active-low vs active-high)
LED_OFF()  // Turn LED off
```

## Key Differences Between Boards

| Feature | ESP32-C3 | ESP32 WROOM-32 |
|---------|----------|----------------|
| WiFi TX Power | 8.5 dBm (limited) | 19.5 dBm (full) |
| CPU Frequency | 80 MHz | 160 MHz |
| LED GPIO | 8 (strapping pin!) | 2 |
| I2C SDA | GPIO 6 | GPIO 21 |
| I2C SCL | GPIO 7 | GPIO 22 |
| LED Active | Active-LOW | Active-LOW |

## Important Notes

### ESP32-C3 Specific
- **WiFi Power Limitation**: The ESP32-C3 Super Mini has a hardware power supply design limitation requiring reduced WiFi TX power. See [board_config.h](include/board_config.h) for full documentation.
- **GPIO8 Strapping Pin**: Must be HIGH during boot. The code ensures this before reboots (especially during OTA updates).

### ESP32 WROOM-32 Specific
- Can use full WiFi power for maximum range
- Standard GPIO assignments compatible with most DevKit boards

## Adding New Board Variants

To add support for a new ESP32 variant:

1. Add a new environment in `platformio.ini`:
```ini
[env:newboard]
platform = espressif32
board = your-board-name
framework = arduino
build_flags = -DBOARD_NEWBOARD
```

2. Add configuration block in `include/board_config.h`:
```c
#elif defined(BOARD_NEWBOARD)
    #define BOARD_NAME "NewBoard"
    #define LED_PIN xx
    #define I2C_SDA xx
    #define I2C_SCL xx
    // ... other settings
#endif
```

3. Test compilation: `pio run -e newboard`

## Firmware Version

Current version: **1.4.0** (multi-board support)

## Technical Details

- **Framework**: Arduino ESP32
- **Platform**: Espressif 32
- **Libraries**: ArduinoOTA, Adafruit BMP280, Adafruit AHT20
- **Features**: WiFi Manager, OTA Updates, Environmental Sensors, Web Interface
