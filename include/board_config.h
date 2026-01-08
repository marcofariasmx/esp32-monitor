#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Board Configuration
// This file centralizes all board-specific settings for different ESP32 variants
// Use PlatformIO build flags to select the board type

#if defined(BOARD_ESP32C3)
    // ============================================
    // ESP32-C3 Super Mini / DevKitM-1 Configuration
    // ============================================
    //
    // CRITICAL: WiFi Power Fix Required for ESP32-C3 Super Mini
    // =========================================================
    // The ESP32-C3 Super Mini has a hardware power supply design limitation
    // that prevents WiFi from working at default transmission power levels.
    //
    // SYMPTOM: WiFi Access Point not visible on devices, even though serial
    //          output reports "Access Point created successfully"
    //
    // SOLUTION: Call WiFi.setTxPower(WIFI_POWER_8_5dBm) immediately AFTER
    //           WiFi.softAP() or WiFi.begin()
    //
    // WHY: The board's power delivery circuit cannot supply enough power for
    //      the WiFi radio at higher transmission levels (>8.5dBm)
    //
    // This is a HARDWARE issue specific to ESP32-C3 Super Mini boards,
    // not a software bug. Documented in multiple GitHub issues:
    // - https://github.com/espressif/arduino-esp32/issues/6551
    // - https://github.com/luc-github/ESP3D/issues/1009
    //
    // Additional recommendations if WiFi still doesn't work:
    // - Use USB 3.0 port (provides more power than USB 2.0)
    // - Use high-quality USB cable (cheap cables have higher resistance)
    // - Use powered USB hub
    // ============================================

    #define BOARD_NAME "ESP32-C3"
    #define BOARD_FULL_NAME "ESP32-C3 Super Mini"

    // LED Configuration
    // CRITICAL: GPIO8 is a strapping pin on ESP32-C3!
    // ================================================
    // GPIO8 determines boot mode on ESP32-C3:
    // - GPIO8 HIGH during power-up = Normal boot mode (required!)
    // - GPIO8 LOW during power-up = Wrong boot mode (device gets stuck)
    //
    // SOLUTION: Always ensure GPIO8 is HIGH before any restart/reboot
    // - LED_OFF() macro sets GPIO8 to HIGH (safe state)
    // - LED_ON() macro sets GPIO8 to LOW (only safe when running)
    //
    // This is why we use LED_OFF() before reboots in OTA code.
    #define LED_PIN 8
    #define LED_ACTIVE_LOW true  // ESP32-C3 Super Mini LED is active-LOW

    // I2C Configuration
    #define I2C_SDA 6
    #define I2C_SCL 7

    // WiFi Power Configuration
    // ESP32-C3 Super Mini has hardware power supply limitations
    // MUST use reduced TX power (8.5dBm) or WiFi AP won't be visible
    #define WIFI_NEEDS_POWER_REDUCTION true
    #define WIFI_TX_POWER WIFI_POWER_8_5dBm

    // Power Management
    #define CPU_FREQ_MHZ 80  // Reduce to 80 MHz for power saving

    // Board-specific notes
    #define BOARD_NOTES "WiFi TX power limited to 8.5dBm due to hardware power supply design"

#elif defined(BOARD_ESP32_WROOM)
    // ============================================
    // ESP32 WROOM-32 / DevKit Configuration
    // ============================================

    #define BOARD_NAME "ESP32"
    #define BOARD_FULL_NAME "ESP32 WROOM-32"

    // LED Configuration
    // Most ESP32 DevKit boards have built-in LED on GPIO2
    #define LED_PIN 2
    #define LED_ACTIVE_LOW true  // Most DevKit LEDs are active-LOW

    // I2C Configuration (ESP32 default pins)
    #define I2C_SDA 21
    #define I2C_SCL 22

    // WiFi Power Configuration
    // ESP32 WROOM-32 can use full WiFi power
    #define WIFI_NEEDS_POWER_REDUCTION false
    #define WIFI_TX_POWER WIFI_POWER_19_5dBm  // Maximum power for best range

    // Power Management
    #define CPU_FREQ_MHZ 80  // Reduced for power saving (can use 160 or 240 for more performance)

    // Board-specific notes
    #define BOARD_NOTES "Full WiFi TX power available (19.5dBm max)"

#else
    #error "No board type defined! Use -DBOARD_ESP32C3 or -DBOARD_ESP32_WROOM in platformio.ini"
#endif

// Common Configuration (applies to all boards)
// Note: BMP280_ADDRESS is defined in Adafruit_BMP280.h as 0x77
// We use 0x76 as the primary address and 0x77 as fallback in setupBMP280()
#define FIRMWARE_VERSION "2.1.0"  // AP disable feature - hide ESP32 WiFi when connected to router
#define WDT_TIMEOUT 10  // Watchdog timer timeout in seconds

// Power Management Configuration Type (chip-specific)
// ESP32-C3 uses esp_pm_config_esp32c3_t, ESP32 uses esp_pm_config_esp32_t
#if defined(BOARD_ESP32C3)
    #define PM_CONFIG_TYPE esp_pm_config_esp32c3_t
#elif defined(BOARD_ESP32_WROOM)
    #define PM_CONFIG_TYPE esp_pm_config_esp32_t
#else
    #define PM_CONFIG_TYPE esp_pm_config_esp32_t  // Default fallback
#endif

// Common WiFi Settings
#define AP_SSID "ESP32-Monitor"
#define AP_PASSWORD "12345678"

// OTA Configuration
#define OTA_HOSTNAME "ESP32-Monitor"
#define OTA_PASSWORD "admin"
#define OTA_PORT 3232

// Helper macros for LED control (handles active-low vs active-high)
#if LED_ACTIVE_LOW
    #define LED_ON()  digitalWrite(LED_PIN, LOW)
    #define LED_OFF() digitalWrite(LED_PIN, HIGH)
#else
    #define LED_ON()  digitalWrite(LED_PIN, HIGH)
    #define LED_OFF() digitalWrite(LED_PIN, LOW)
#endif

#endif // BOARD_CONFIG_H
