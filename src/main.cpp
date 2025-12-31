/*
 * ESP32-C3 Super Mini - System Monitor & WiFi Manager
 *
 * CRITICAL: WiFi Power Fix Required for ESP32-C3 Super Mini
 * =========================================================
 * The ESP32-C3 Super Mini has a hardware power supply design limitation
 * that prevents WiFi from working at default transmission power levels.
 *
 * SYMPTOM: WiFi Access Point not visible on devices, even though serial
 *          output reports "Access Point created successfully"
 *
 * SOLUTION: Call WiFi.setTxPower(WIFI_POWER_8_5dBm) immediately AFTER
 *           WiFi.softAP() or WiFi.begin()
 *
 * WHY: The board's power delivery circuit cannot supply enough power for
 *      the WiFi radio at higher transmission levels (>8.5dBm)
 *
 * This is a HARDWARE issue specific to ESP32-C3 Super Mini boards,
 * not a software bug. Documented in multiple GitHub issues:
 * - https://github.com/espressif/arduino-esp32/issues/6551
 * - https://github.com/luc-github/ESP3D/issues/1009
 *
 * Additional recommendations if WiFi still doesn't work:
 * - Use USB 3.0 port (provides more power than USB 2.0)
 * - Use high-quality USB cable (cheap cables have higher resistance)
 * - Use powered USB hub
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include "esp_task_wdt.h"  // Watchdog timer for freeze protection
#include "index.h"  // HTML page content

// Watchdog timer configuration (10 seconds)
#define WDT_TIMEOUT 10

// Web server on port 80
WebServer server(80);

// NVS storage for WiFi credentials
Preferences preferences;

// BMP280 Sensor Configuration
// I2C pins for ESP32-C3 Super Mini
#define I2C_SDA 6
#define I2C_SCL 7
#define BMP280_ADDRESS 0x76  // or 0x77 depending on your module

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
bool bmpAvailable = false;
bool ahtAvailable = false;

// Sensor readings
float currentTemperature = 0.0;
float currentPressure = 0.0;
float currentAltitude = 0.0;
float currentHumidity = 0.0;

// AP Configuration
const char* ap_ssid = "ESP32-Monitor";
const char* ap_password = "12345678";  // WPA2 password (min 8 chars) - more compatible than open network

// Variables for WiFi
String sta_ssid = "";
String sta_password = "";
bool sta_connected = false;

// OTA update flag
bool otaInProgress = false;

// Uptime tracking
unsigned long startTime = 0;

// LED Configuration (ESP32-C3 Super Mini onboard LED)
const int LED_PIN = 8;  // GPIO8 for ESP32-C3 Super Mini blue LED
unsigned long lastLedBlink = 0;
bool ledState = false;

// Heartbeat LED pattern (when connected to WiFi)
// Pattern: ON-OFF-ON-long pause (like: ♥ lub-dub ... pause ... ♥ lub-dub ...)
unsigned long lastHeartbeat = 0;
int heartbeatStep = 0;  // 0-3: controls heartbeat sequence (4 steps)
const int HEARTBEAT_PATTERN[] = {100, 100, 100, 1500};  // ms for each step: beat1, pause, beat2, long pause

// WiFi Roaming Configuration with Exponential Backoff
const int RSSI_THRESHOLD = -75;  // Reconnect if signal drops below this (dBm)
const int RSSI_IMPROVEMENT = 10;  // Switch if another AP is this much stronger (dBm)

// Progressive interval using exponential backoff: 15s → 30s → 60s → 120s
const unsigned long ROAMING_INTERVAL_MIN = 15000;   // Start at 15 seconds
const unsigned long ROAMING_INTERVAL_MAX = 120000;  // Max 2 minutes
unsigned long currentRoamingInterval = ROAMING_INTERVAL_MIN;  // Current interval (increases on failure)
unsigned long lastRoamingCheck = 0;

// Function prototypes
void setupAccessPoint();
void setupOTA();
void setupMDNS();
void setupBMP280();
void setupAHT20();
void readBMP280();
void readAHT20();
void loadWiFiCredentials();
void saveWiFiCredentials(String ssid, String password);
void connectToWiFi();
void checkWiFiRoaming();
void handleRoot();
void handleScan();
void handleConnect();
void handleStatus();
String getUptime();
float getTemperature();
String getHTMLPage();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // OTA improvements test - firmware version 1.1
  startTime = millis();

  // Initialize watchdog timer to prevent system freezes
  // If the system freezes for more than 10 seconds, it will auto-reset
  esp_task_wdt_init(WDT_TIMEOUT, true);  // 10 sec timeout, panic on timeout
  esp_task_wdt_add(NULL);  // Add current thread to watchdog
  Serial.println("✓ Watchdog timer enabled (10s timeout)");

  // POWER SAVING: Reduce CPU frequency to 80 MHz (minimum for WiFi operation)
  // This reduces power consumption by ~30-40% while maintaining WiFi functionality
  // 160 MHz → 80 MHz saves approximately 20-30 mA
  setCpuFrequencyMhz(80);
  Serial.println("✓ CPU Frequency reduced to 80 MHz for power saving");

  // Setup LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Start with LED off

  // Give sensors time to power up and stabilize after CPU frequency change
  delay(500);

  // Initialize BMP280 sensor
  setupBMP280();

  // Initialize AHT20 sensor
  setupAHT20();

  Serial.println("\n=================================");
  Serial.println("ESP32-C3 Super Mini - Monitor");
  Serial.println("=================================");
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  Serial.println("=================================\n");

  // Initialize NVS
  preferences.begin("wifi-creds", false);

  // Load saved WiFi credentials
  loadWiFiCredentials();

  // Setup Access Point (always active as fallback)
  setupAccessPoint();

  // Give AP time to stabilize
  delay(500);

  // Setup mDNS on AP interface (works immediately, no WiFi needed)
  // This makes http://esp32-monitor.local/ work on the AP network (192.168.4.x)
  setupMDNS();

  // Try to connect to saved WiFi network
  if (sta_ssid.length() > 0) {
    Serial.println("Attempting to connect to saved WiFi...");
    connectToWiFi();
  } else {
    Serial.println("No saved WiFi credentials - AP mode only\n");
  }

  // Setup OTA updates and reinitialize mDNS for Station mode
  // mDNS will now work on BOTH AP interface (192.168.4.x) and Station (home WiFi)
  if (sta_connected) {
    setupMDNS();  // Reinitialize for dual-interface operation
    setupOTA();
  }

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/connect", handleConnect);
  server.on("/status", handleStatus);

  // Start web server
  server.begin();
  Serial.println("✓ Web server started!");

  // Print connection info
  Serial.println("\n========== CONNECTION INFO ==========");
  Serial.println("To connect:");
  Serial.println("1. Connect to WiFi: " + String(ap_ssid));
  Serial.println("2. Password: " + String(ap_password));
  Serial.println("3. Open browser to: http://192.168.4.1");
  Serial.println("=====================================\n");

  if (sta_connected) {
    Serial.println("Also accessible at: http://" + WiFi.localIP().toString());
  }

  Serial.println("Setup complete! LED should be blinking...\n");
}

void loop() {
  // Feed the watchdog timer to prevent auto-reset
  esp_task_wdt_reset();

  // Skip web server handling during OTA update
  if (!otaInProgress) {
    server.handleClient();
  }

  // Handle OTA updates (only when connected to WiFi)
  // Note: mDNS runs automatically in background on ESP32
  if (sta_connected) {
    ArduinoOTA.handle();
  }

  // Read sensor data (every 5 seconds to reduce I2C bus stress)
  // Skip sensor readings during OTA update to prevent interference
  static unsigned long lastSensorRead = 0;
  if (!otaInProgress && millis() - lastSensorRead >= 5000) {
    lastSensorRead = millis();

    // Feed watchdog before potentially slow I2C operations
    esp_task_wdt_reset();

    readBMP280();
    readAHT20();
  }

  // Check WiFi connection status
  if (sta_ssid.length() > 0 && WiFi.status() != WL_CONNECTED && sta_connected) {
    sta_connected = false;
    Serial.println("WiFi connection lost!");
  } else if (sta_ssid.length() > 0 && WiFi.status() == WL_CONNECTED && !sta_connected) {
    sta_connected = true;
    Serial.println("WiFi reconnected!");
    Serial.println("Station IP: " + WiFi.localIP().toString());
    // Setup mDNS and OTA after reconnection
    setupMDNS();
    setupOTA();
  }

  // WiFi roaming check - only when connected
  if (sta_connected) {
    checkWiFiRoaming();
  }

  // LED control logic
  unsigned long currentMillis = millis();

  if (otaInProgress) {
    // OTA in progress: Fast flashing (100ms interval) - very visible!
    if (currentMillis - lastLedBlink >= 100) {
      lastLedBlink = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
  } else if (sta_connected) {
    // Connected to WiFi: Heartbeat pattern (♥ lub-dub ... pause ... ♥ lub-dub)
    if (currentMillis - lastHeartbeat >= HEARTBEAT_PATTERN[heartbeatStep]) {
      lastHeartbeat = currentMillis;

      // Steps: ON(100ms)-OFF(100ms)-ON(100ms)-OFF(1500ms)-loop
      // This creates: lub ... dub ... long pause (like a real heartbeat!)
      if (heartbeatStep == 0 || heartbeatStep == 2) {
        digitalWrite(LED_PIN, HIGH);  // ON for "lub" (step 0) and "dub" (step 2)
      } else {
        digitalWrite(LED_PIN, LOW);   // OFF for pauses (steps 1 and 3)
      }

      heartbeatStep = (heartbeatStep + 1) % 4;  // Cycle through 4 steps
    }
  } else {
    // Not connected: LED blinks (500ms interval)
    if (currentMillis - lastLedBlink >= 500) {
      lastLedBlink = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
  }
}

void setupAccessPoint() {
  Serial.println("Setting up Access Point...");

  // Disconnect any previous connections
  WiFi.disconnect(true);
  delay(100);

  // Set mode to AP+STA
  WiFi.mode(WIFI_AP_STA);
  delay(100);

  // Configure Access Point
  // Channel 1, Hidden=false, Max connections=4
  // Using channel 1 for maximum compatibility with Mac/iPhone
  bool result = WiFi.softAP(ap_ssid, ap_password, 1, 0, 4);

  if (result) {
    Serial.println("✓ Access Point created successfully!");
  } else {
    Serial.println("✗ Failed to create Access Point!");
  }

  // CRITICAL FIX for ESP32-C3 Super Mini:
  // Reduce WiFi TX power to 8.5dBm due to hardware power supply limitations
  // This MUST be called AFTER WiFi.softAP()
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  Serial.println("✓ WiFi TX Power set to 8.5dBm (ESP32-C3 hardware limitation)");

  // POWER SAVING: Enable WiFi Modem Sleep Mode
  // This allows the WiFi radio to sleep between DTIM beacons while maintaining connection
  // Reduces power consumption by ~60-75% (80 mA → 15-25 mA)
  // Web server remains accessible with minimal latency increase (~50-100ms)
  WiFi.setSleep(true);  // Enable WIFI_PS_MIN_MODEM
  Serial.println("✓ WiFi Modem Sleep enabled for power saving");

  // Configure AP IP address (192.168.4.1)
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  if (WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("✓ AP IP configured successfully");
  } else {
    Serial.println("✗ Failed to configure AP IP");
  }

  delay(100);

  IPAddress IP = WiFi.softAPIP();
  Serial.println("\n--- Access Point Details ---");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("IP Address: ");
  Serial.println(IP);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("---------------------------\n");
}

void setupOTA() {
  // Configure OTA hostname
  ArduinoOTA.setHostname("ESP32-Monitor");

  // Set OTA password for security
  ArduinoOTA.setPassword("admin");

  // Port defaults to 3232
  ArduinoOTA.setPort(3232);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }

    Serial.println("\n========================================");
    Serial.println("       OTA UPDATE STARTING");
    Serial.println("========================================");
    Serial.println("Type: " + type);
    Serial.println("\nFreeing resources for OTA...");

    // Set OTA flag to stop sensor readings and other tasks
    otaInProgress = true;

    // CRITICAL: Stop web server to free TCP buffers and memory
    // This prevents TCP buffer overflow during OTA
    server.stop();
    Serial.println("✓ Web server stopped");

    // CRITICAL: Disable WiFi modem sleep for stable connection during OTA
    // Power saving can cause disconnections during upload
    WiFi.setSleep(false);
    Serial.println("✓ WiFi modem sleep disabled");

    // Stop I2C sensors to prevent interrupts
    // Frequent sensor readings can interfere with OTA timing
    Wire.end();
    Serial.println("✓ I2C bus closed");

    // LED will flash rapidly (100ms) during OTA - handled in loop()
    Serial.println("✓ LED will flash rapidly during OTA");

    Serial.print("\nFree heap before OTA: ");
    Serial.print(ESP.getFreeHeap() / 1024);
    Serial.println(" KB");
    Serial.println("Ready for upload...\n");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n--- OTA Update Complete ---");
    Serial.println("Rebooting...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));

    // Only print every 10%
    if (percent != lastPercent && percent % 10 == 0) {
      Serial.printf("Progress: %u%%\n", percent);
      lastPercent = percent;
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("\n--- OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    Serial.println("--- OTA Error End ---\n");
  });

  ArduinoOTA.begin();

  Serial.println("\n--- OTA Update Enabled ---");
  Serial.println("Hostname: ESP32-Monitor");
  Serial.println("IP Address: " + WiFi.localIP().toString());
  Serial.println("Port: 3232");
  Serial.println("Password: admin");
  Serial.println("\nTo upload via OTA:");
  Serial.println("pio run -t upload --upload-port " + WiFi.localIP().toString());
  Serial.println("--- OTA Ready ---\n");
}

void setupMDNS() {
  Serial.println("\n--- Initializing mDNS ---");

  // IMPORTANT: Always stop mDNS service first if it's running
  // This ensures clean state when called from loop() after WiFi connects
  MDNS.end();
  delay(100);  // Give time for cleanup

  // Start mDNS service with retry logic
  bool success = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("mDNS attempt ");
    Serial.print(attempt);
    Serial.print("/3... ");

    if (MDNS.begin("esp32-monitor")) {
      success = true;
      Serial.println("SUCCESS!");
      break;
    } else {
      Serial.println("failed");
      if (attempt < 3) {
        delay(500);  // Wait before retry
      }
    }
  }

  if (success) {
    // Add service to mDNS-SD
    MDNS.addService("http", "tcp", 80);

    Serial.println("\n✓ mDNS Ready!");
    Serial.println("Hostname: esp32-monitor.local");
    Serial.println("Access via: http://esp32-monitor.local");
    Serial.println("--- mDNS Initialized ---\n");
  } else {
    Serial.println("\n✗ mDNS failed after 3 attempts");
    Serial.println("Device still accessible via IP address");
    Serial.println("Try rebooting to enable mDNS\n");
  }
}

void loadWiFiCredentials() {
  sta_ssid = preferences.getString("ssid", "");
  sta_password = preferences.getString("password", "");

  if (sta_ssid.length() > 0) {
    Serial.println("Loaded WiFi credentials from NVS");
    Serial.println("SSID: " + sta_ssid);
  } else {
    Serial.println("No saved WiFi credentials found");
  }
}

void saveWiFiCredentials(String ssid, String password) {
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  Serial.println("WiFi credentials saved to NVS");
}

void connectToWiFi() {
  if (sta_ssid.length() == 0) {
    Serial.println("No SSID provided");
    return;
  }

  Serial.print("Connecting to WiFi: ");
  Serial.println(sta_ssid);

  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());

  // CRITICAL FIX for ESP32-C3 Super Mini:
  // Set TX power after WiFi.begin() for station mode
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // POWER SAVING: Enable WiFi Modem Sleep for station mode
  WiFi.setSleep(true);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    sta_connected = true;
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("BSSID: ");
    Serial.println(WiFi.BSSIDstr());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("✓ WiFi Modem Sleep active (station mode)");
  } else {
    sta_connected = false;
    Serial.println("\nFailed to connect to WiFi");
  }
}

void checkWiFiRoaming() {
  unsigned long currentMillis = millis();

  // Only check at current progressive interval (exponential backoff)
  if (currentMillis - lastRoamingCheck < currentRoamingInterval) {
    return;
  }

  lastRoamingCheck = currentMillis;

  int currentRSSI = WiFi.RSSI();
  String currentBSSID = WiFi.BSSIDstr();

  // If signal is good, reset to minimum interval and skip scan
  if (currentRSSI > RSSI_THRESHOLD) {
    // Signal improved - reset backoff timer for next time
    if (currentRoamingInterval != ROAMING_INTERVAL_MIN) {
      currentRoamingInterval = ROAMING_INTERVAL_MIN;
      Serial.println("✓ WiFi signal good - reset roaming interval to 15s");
    }
    return;  // No scan needed
  }

  Serial.println("\n--- WiFi Roaming Check ---");
  Serial.print("Current RSSI: ");
  Serial.print(currentRSSI);
  Serial.println(" dBm (weak signal)");
  Serial.print("Current interval: ");
  Serial.print(currentRoamingInterval / 1000);
  Serial.println("s");
  Serial.println("Scanning for better AP...");

  // Scan for networks
  int n = WiFi.scanNetworks();

  int bestRSSI = currentRSSI;
  String bestBSSID = "";

  // Find the best AP with our SSID
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == sta_ssid) {
      int rssi = WiFi.RSSI(i);
      String bssid = WiFi.BSSIDstr(i);

      Serial.print("Found AP: ");
      Serial.print(bssid);
      Serial.print(" with RSSI: ");
      Serial.print(rssi);
      Serial.println(" dBm");

      // Check if this AP is significantly better
      if (rssi > bestRSSI + RSSI_IMPROVEMENT) {
        bestRSSI = rssi;
        bestBSSID = bssid;
      }
    }
  }

  // If we found a better AP, switch to it
  if (bestBSSID.length() > 0 && bestBSSID != currentBSSID) {
    Serial.print("✓ Switching to better AP: ");
    Serial.print(bestBSSID);
    Serial.print(" (");
    Serial.print(bestRSSI);
    Serial.println(" dBm)");

    // Disconnect and reconnect to force AP selection
    WiFi.disconnect();
    delay(100);
    connectToWiFi();

    // Successfully switched - reset interval to minimum
    currentRoamingInterval = ROAMING_INTERVAL_MIN;
    Serial.println("✓ Reset roaming interval to 15s after successful switch");
  } else {
    Serial.println("✗ No better AP found, staying connected");

    // No better AP found - increase interval (exponential backoff)
    unsigned long newInterval = currentRoamingInterval * 2;
    if (newInterval > ROAMING_INTERVAL_MAX) {
      newInterval = ROAMING_INTERVAL_MAX;
    }

    if (newInterval != currentRoamingInterval) {
      currentRoamingInterval = newInterval;
      Serial.print("⏱  Increased roaming interval to ");
      Serial.print(currentRoamingInterval / 1000);
      Serial.println("s (exponential backoff)");
    } else {
      Serial.print("⏱  Staying at max interval: ");
      Serial.print(currentRoamingInterval / 1000);
      Serial.println("s");
    }
  }

  Serial.println("--- Roaming Check Complete ---\n");
}

String getUptime() {
  unsigned long uptime = millis() - startTime;
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  String result = "";
  if (days > 0) result += String(days) + "d ";
  if (hours > 0) result += String(hours) + "h ";
  if (minutes > 0) result += String(minutes) + "m ";
  result += String(seconds) + "s";

  return result;
}

float getTemperature() {
  // ESP32-C3 has a built-in temperature sensor
  // Note: Measures chip internal temperature, not ambient temperature
  // Accuracy: ±1°C to ±3°C depending on range
  // WiFi usage increases chip temperature due to power consumption

  // Using Arduino ESP32 built-in function
  // Returns temperature in Celsius
  float temp_celsius = temperatureRead();

  return temp_celsius;
}

void setupBMP280() {
  Serial.println("\n--- BMP280 Sensor Setup ---");

  // Initialize I2C with custom pins for ESP32-C3 Super Mini
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(50);  // 50ms timeout to prevent freezing on I2C errors

  // Give I2C bus time to initialize
  delay(100);

  Serial.print("Attempting to initialize BMP280 at address 0x");
  Serial.println(BMP280_ADDRESS, HEX);

  // Try to initialize the sensor
  if (!bmp.begin(BMP280_ADDRESS)) {
    Serial.println("✗ Could not find BMP280 sensor!");
    Serial.println("  Check wiring:");
    Serial.println("  - VCC to 3V3");
    Serial.println("  - GND to GND");
    Serial.println("  - SDA to GPIO6");
    Serial.println("  - SCL to GPIO7");
    Serial.println("  Trying alternate address 0x77...");

    // Try alternate I2C address
    if (!bmp.begin(0x77)) {
      Serial.println("✗ BMP280 not found at 0x77 either!");
      Serial.println("--- BMP280 Setup Failed ---\n");
      bmpAvailable = false;
      return;
    } else {
      Serial.println("✓ BMP280 found at address 0x77!");
    }
  } else {
    Serial.println("✓ BMP280 found at address 0x76!");
  }

  // Configure sensor settings for best accuracy
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode */
                  Adafruit_BMP280::SAMPLING_X2,      /* Temperature oversampling */
                  Adafruit_BMP280::SAMPLING_X16,     /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,       /* Filtering */
                  Adafruit_BMP280::STANDBY_MS_500);  /* Standby time */

  bmpAvailable = true;
  Serial.println("✓ BMP280 configured successfully!");
  Serial.println("--- BMP280 Ready ---\n");

  // Take initial reading
  readBMP280();
}

void readBMP280() {
  if (!bmpAvailable) {
    return;
  }

  // Read sensor values
  currentTemperature = bmp.readTemperature();
  currentPressure = bmp.readPressure() / 100.0F;  // Convert Pa to hPa (mbar)
  currentAltitude = bmp.readAltitude(1013.25);     // 1013.25 = sea-level pressure in hPa

  // Optional: Print to serial for debugging
  // Uncomment the lines below if you want to see sensor readings in serial monitor
  /*
  Serial.print("Temperature: ");
  Serial.print(currentTemperature);
  Serial.print(" °C, Pressure: ");
  Serial.print(currentPressure);
  Serial.print(" hPa, Altitude: ");
  Serial.print(currentAltitude);
  Serial.println(" m");
  */
}

void setupAHT20() {
  Serial.println("\n--- AHT20 Sensor Setup ---");

  // Add delay to ensure AHT20 has time to be ready
  delay(100);

  Serial.println("Attempting to initialize AHT20 sensor...");

  // Try to initialize the sensor (AHT20 is at address 0x38)
  if (!aht.begin()) {
    Serial.println("✗ Could not find AHT20 sensor!");
    Serial.println("  Check wiring (same as BMP280):");
    Serial.println("  - VCC to 3V3");
    Serial.println("  - GND to GND");
    Serial.println("  - SDA to GPIO6");
    Serial.println("  - SCL to GPIO7");
    Serial.println("--- AHT20 Setup Failed ---\n");
    ahtAvailable = false;
    return;
  }

  ahtAvailable = true;
  Serial.println("✓ AHT20 sensor found!");
  Serial.println("✓ AHT20 configured successfully!");
  Serial.println("--- AHT20 Ready ---\n");
}

void readAHT20() {
  if (!ahtAvailable) {
    return;
  }

  // Read sensor values
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // Use AHT20 temperature (more accurate than BMP280)
  // If you prefer BMP280 temperature, comment out the line below
  currentTemperature = temp.temperature;
  currentHumidity = humidity.relative_humidity;

  // Optional: Print to serial for debugging
  // Uncomment the lines below if you want to see sensor readings in serial monitor
  /*
  Serial.print("AHT20 - Temperature: ");
  Serial.print(currentTemperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(currentHumidity);
  Serial.println(" %");
  */
}

void handleRoot() {
  String html = getHTMLPage();
  server.send(200, "text/html", html);
}

void handleScan() {
  Serial.println("\n--- WiFi Scan Requested ---");

  // IMPORTANT: Always delete previous scan results first
  // This prevents "Scan already in progress" errors
  WiFi.scanDelete();
  delay(100);  // Give it time to clean up

  Serial.println("Starting WiFi scan...");

  // Use BLOCKING scan for reliability in AP+STA mode
  // Async mode can hang in AP+STA configuration on ESP32-C3
  int n = WiFi.scanNetworks(false, true);  // blocking mode, show hidden networks

  if (n < 0) {
    Serial.println("✗ Scan failed! Error code: " + String(n));
    WiFi.scanDelete();
    server.send(500, "application/json", "{\"error\":\"Scan failed\"}");
    return;
  }

  Serial.println("✓ Scan complete! Found " + String(n) + " networks");

  // Deduplicate networks - keep only the strongest signal for each SSID
  // Using a simple map-like structure with arrays
  String uniqueSSIDs[n > 0 ? n : 1];  // Prevent zero-size array
  int uniqueRSSI[n > 0 ? n : 1];
  int uniqueEncryption[n > 0 ? n : 1];
  int uniqueCount = 0;

  for (int i = 0; i < n; i++) {
    String currentSSID = WiFi.SSID(i);
    int currentRSSI = WiFi.RSSI(i);
    int currentEncryption = WiFi.encryptionType(i);

    // Skip empty SSIDs (hidden networks)
    if (currentSSID.length() == 0) continue;

    // Check if this SSID already exists
    bool found = false;
    for (int j = 0; j < uniqueCount; j++) {
      if (uniqueSSIDs[j] == currentSSID) {
        // Found duplicate - keep the one with stronger signal (higher RSSI)
        if (currentRSSI > uniqueRSSI[j]) {
          uniqueRSSI[j] = currentRSSI;
          uniqueEncryption[j] = currentEncryption;
        }
        found = true;
        break;
      }
    }

    // If not found, add as new unique entry
    if (!found) {
      uniqueSSIDs[uniqueCount] = currentSSID;
      uniqueRSSI[uniqueCount] = currentRSSI;
      uniqueEncryption[uniqueCount] = currentEncryption;
      uniqueCount++;
    }
  }

  // Build JSON response with deduplicated networks
  String json = "[";
  for (int i = 0; i < uniqueCount; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + uniqueSSIDs[i] + "\",";
    json += "\"rssi\":" + String(uniqueRSSI[i]) + ",";
    json += "\"encryption\":" + String(uniqueEncryption[i]);
    json += "}";
  }
  json += "]";

  server.send(200, "application/json", json);

  // Clean up scan results
  WiFi.scanDelete();

  Serial.println("✓ Sent " + String(uniqueCount) + " unique networks to client");
  Serial.println("--- WiFi Scan Complete ---\n");
}

void handleConnect() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    sta_ssid = server.arg("ssid");
    sta_password = server.arg("password");

    Serial.println("Received connection request:");
    Serial.println("SSID: " + sta_ssid);

    saveWiFiCredentials(sta_ssid, sta_password);

    server.send(200, "text/plain", "Connecting to " + sta_ssid + "...");

    delay(1000);
    connectToWiFi();
  } else {
    server.send(400, "text/plain", "Missing SSID or password");
  }
}

void handleStatus() {
  String json = "{";

  // System statistics
  json += "\"uptime\":\"" + getUptime() + "\",";
  json += "\"temperature\":" + String(getTemperature(), 1) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"totalHeap\":" + String(ESP.getHeapSize()) + ",";
  json += "\"cpuFreq\":" + String(ESP.getCpuFreqMHz()) + ",";

  // Sensor data (BMP280 + AHT20)
  json += "\"bmpAvailable\":" + String(bmpAvailable ? "true" : "false") + ",";
  json += "\"ahtAvailable\":" + String(ahtAvailable ? "true" : "false") + ",";
  if (bmpAvailable || ahtAvailable) {
    json += "\"sensorTemperature\":" + String(currentTemperature, 1) + ",";
  }
  if (bmpAvailable) {
    json += "\"bmpPressure\":" + String(currentPressure, 1) + ",";
    json += "\"bmpAltitude\":" + String(currentAltitude, 1) + ",";
  }
  if (ahtAvailable) {
    json += "\"ahtHumidity\":" + String(currentHumidity, 1) + ",";
  }

  // Chip information
  json += "\"chipModel\":\"" + String(ESP.getChipModel()) + "\",";
  json += "\"chipRevision\":" + String(ESP.getChipRevision()) + ",";
  json += "\"chipCores\":" + String(ESP.getChipCores()) + ",";

  // Flash information
  json += "\"flashSize\":" + String(ESP.getFlashChipSize()) + ",";
  json += "\"sketchSize\":" + String(ESP.getSketchSize()) + ",";
  json += "\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace()) + ",";

  // Network information
  json += "\"apIP\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"staConnected\":" + String(sta_connected ? "true" : "false") + ",";
  json += "\"staIP\":\"" + (sta_connected ? WiFi.localIP().toString() : "N/A") + "\",";
  json += "\"staSSID\":\"" + (sta_connected ? WiFi.SSID() : "N/A") + "\",";
  json += "\"staRSSI\":" + String(sta_connected ? WiFi.RSSI() : 0);

  json += "}";

  server.send(200, "application/json", json);
}

String getHTMLPage() {
  return String(INDEX_HTML);
}
