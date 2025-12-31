# ESP32-C3 Super Mini - System Monitor & WiFi Manager

A comprehensive monitoring and WiFi management system for the ESP32-C3 Super Mini board.

## Features

- 📊 Real-time system statistics monitoring
- 📡 WiFi Access Point mode with web dashboard
- 🔌 WiFi station mode for connecting to external networks
- 💾 **Persistent WiFi credentials storage (NVS) - survives power cycles!**
- 🔄 **Smart WiFi roaming - automatically switches to stronger access points**
- 📲 **OTA (Over-The-Air) updates - upload new firmware via WiFi**
- 🌐 **Deduplicated network scanning - shows one entry per SSID (perfect for WiFi repeaters)**
- 💡 LED status indicator (blinking = disconnected, solid = connected)
- 🌡️ Internal temperature sensor reading
- 📈 Chip information and statistics

## Hardware Requirements

- **ESP32-C3 Super Mini** board
- USB-C cable (preferably USB 3.0 for better power delivery)

## Important: WiFi Power Fix

### The Problem

The ESP32-C3 Super Mini has a **known hardware power supply design limitation** that prevents WiFi from functioning at default transmission power levels. Without this fix, the WiFi Access Point will not be visible to devices even though the code reports successful creation.

### The Solution

After initializing WiFi (both AP and Station modes), you **must** reduce the TX power to 8.5dBm or lower:

```cpp
// CRITICAL FIX for ESP32-C3 Super Mini
WiFi.setTxPower(WIFI_POWER_8_5dBm);
```

### Why This Happens

The ESP32-C3 Super Mini boards manufactured in 2024 have an inadequate power supply design that cannot provide enough power for the WiFi radio at higher transmission levels. This is a **hardware issue**, not a software bug.

### When to Apply This Fix

The fix must be applied **AFTER** WiFi initialization:

#### For Access Point Mode:
```cpp
WiFi.softAP(ssid, password, channel, hidden, max_connections);
WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Must be AFTER softAP()
```

#### For Station Mode:
```cpp
WiFi.begin(ssid, password);
WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Must be AFTER begin()
```

### Additional Recommendations

If WiFi still doesn't work after applying the power fix:

1. **Use a better USB power source**:
   - Connect to a USB 3.0 port (provides more power)
   - Use a powered USB hub
   - Avoid connecting through USB 2.0 hubs

2. **Use a high-quality USB cable**:
   - Cheap cables have higher resistance
   - Can't deliver adequate power to the board

3. **Check serial output**:
   - Look for "✓ WiFi TX Power set to 8.5dBm" message
   - Verify "✓ Access Point created successfully!"

### References

- [ESP32-C3 AP visibility issue - GitHub](https://github.com/espressif/arduino-esp32/issues/6551)
- [ESP3D WiFi bug - ESP32-C3 boards](https://github.com/luc-github/ESP3D/issues/1009)
- [Let's Control It forum discussion](https://www.letscontrolit.com/forum/viewtopic.php?t=10661)

## Available ESP32-C3 Sensors & Statistics

### Built-in Temperature Sensor ✅
- **Function**: `temperatureRead()`
- **Range**: -40°C to 125°C
- **Accuracy**: ±1°C to ±3°C (depending on range)
- **Note**: Measures chip internal temperature, not ambient temperature
- **Limitation**: WiFi usage increases chip temperature due to power consumption

### Hall Effect Sensor ❌
- **NOT available on ESP32-C3**
- Only available on original ESP32 (not C3, S2, or S3)

### Chip Information ✅
Available via ESP class:
- `ESP.getChipModel()` - Chip model name
- `ESP.getChipRevision()` - Silicon revision number
- `ESP.getChipCores()` - Number of CPU cores
- `ESP.getCpuFreqMHz()` - CPU frequency in MHz
- `ESP.getFlashChipSize()` - Flash size in bytes
- `ESP.getSketchSize()` - Current sketch size
- `ESP.getFreeSketchSpace()` - Free flash space for sketches
- `ESP.getFreeHeap()` - Free RAM
- `ESP.getSdkVersion()` - SDK version string

### Voltage Monitoring ⚠️
- No direct internal voltage sensor
- Can use ADC on GPIO pins for external voltage monitoring
- ADC0-ADC4 available on specific GPIO pins

## Project Structure

```
esp32-monitor/
├── platformio.ini       # PlatformIO configuration
├── include/
│   └── index.h         # HTML dashboard (PROGMEM)
├── src/
│   └── main.cpp        # Main application code
└── README.md           # This file
```

## Connection Instructions

### First Boot (Access Point Mode)

1. Power on the ESP32-C3 Super Mini
2. Blue LED should start **blinking** (indicates ready but not connected)
3. Look for WiFi network: **ESP32-Monitor**
4. Connect using password: **12345678**
5. Open browser to: **http://192.168.4.1**

### Dashboard Features

The web dashboard displays:
- **System Statistics**:
  - Uptime (days, hours, minutes, seconds)
  - Free heap memory (KB)
  - CPU frequency (MHz)
  - Internal chip temperature (°C)
  - Flash size and usage
  - Chip model and revision

- **Network Status**:
  - Access Point IP address
  - Station connection status
  - Station IP address (when connected)
  - Connected network SSID and signal strength

- **WiFi Management**:
  - Scan for available networks
  - **Deduplicated network list** - shows only one entry per SSID (strongest signal)
  - Perfect for environments with WiFi repeaters
  - Connect to any WiFi network
  - Automatic credential saving (persists across reboots)

### Connecting to External WiFi

1. On the dashboard, click **"Scan WiFi Networks"**
2. Click on your desired network
3. Enter the password
4. Click **"Connect"**
5. **Credentials are automatically saved to NVS flash memory**
6. Blue LED changes to **solid blue** when connected
7. Dashboard shows the new IP address on your network

### 🔐 Password Persistence (Survives Power Off!)

**Your WiFi credentials are permanently saved!**

- Stored in **NVS (Non-Volatile Storage)** flash memory
- ✅ Survives power cycles and reboots
- ✅ Survives firmware updates (OTA or USB)
- ✅ Automatically reconnects on boot
- 📝 Stored values: SSID and password
- 🔒 Secure flash storage

**To change WiFi network:**
1. Connect to the ESP32-Monitor AP (192.168.4.1)
2. Scan and select a new network
3. New credentials will replace the old ones

## 🔄 WiFi Roaming (Automatic AP Switching)

**Perfect for homes/offices with WiFi repeaters!**

### How It Works

When you have multiple access points with the same network name (SSID), the ESP32 will:

1. **Connect to the strongest signal** when you first join the network
2. **Monitor signal strength** every 15 seconds while connected
3. **Automatically switch** to a stronger AP when:
   - Current signal drops below -75 dBm (weak signal), AND
   - Another AP with the same SSID is at least 10 dBm stronger
4. **Seamless transition** - maintains connection while roaming

### Benefits

- ✅ No manual intervention needed when moving around
- ✅ Always connected to the best available AP
- ✅ Prevents staying connected to distant, weak APs
- ✅ Works with WiFi mesh networks and repeaters
- ✅ Serial output shows roaming decisions in real-time

### Customizing Roaming Behavior

Edit these values in [main.cpp](src/main.cpp) lines 59-62:

```cpp
const int RSSI_THRESHOLD = -75;           // When to consider roaming (dBm)
const int RSSI_IMPROVEMENT = 10;          // Minimum improvement to switch (dBm)
const unsigned long ROAMING_CHECK_INTERVAL = 15000;  // Check interval (ms)
```

**RSSI_THRESHOLD** (-75 dBm default):
- Lower values (e.g., -80) = More tolerant of weak signals
- Higher values (e.g., -70) = More aggressive roaming

**RSSI_IMPROVEMENT** (10 dBm default):
- Lower values (e.g., 5) = Switch more frequently
- Higher values (e.g., 15) = Only switch to much better APs

**ROAMING_CHECK_INTERVAL** (15000 ms default):
- Lower values = More responsive but more scanning
- Higher values = Less scanning but slower response

### Roaming Serial Output Example

```
--- WiFi Roaming Check ---
Current RSSI: -78 dBm (weak signal)
Scanning for better AP...
Found AP: XX:XX:XX:XX:XX:01 with RSSI: -77 dBm
Found AP: XX:XX:XX:XX:XX:02 with RSSI: -62 dBm
Switching to better AP: XX:XX:XX:XX:XX:02 (-62 dBm)
Connecting to WiFi: MyNetwork
..
Connected to WiFi!
IP address: 192.168.1.100
BSSID: XX:XX:XX:XX:XX:02
Signal strength: -62 dBm
--- Roaming Check Complete ---
```

## 📲 OTA (Over-The-Air) Updates

**Update firmware wirelessly without USB cable!**

### Prerequisites

1. ESP32 must be connected to your WiFi network (not just AP mode)
2. Your computer must be on the same network
3. Know the ESP32's IP address (shown in web dashboard)

### OTA Information

- **Hostname**: ESP32-Monitor
- **Port**: 3232
- **Password**: admin (changeable in code)
- **Security**: Password-protected

### Method 1: PlatformIO CLI

```bash
# Replace <ESP32_IP> with actual IP (e.g., 192.168.1.100)
pio run -t upload --upload-port <ESP32_IP>
```

Example:
```bash
pio run -t upload --upload-port 192.168.1.100
```

### Method 2: PlatformIO IDE

1. Open `platformio.ini`
2. Update `upload_port` with your ESP32's IP address:
   ```ini
   upload_port = 192.168.1.100  ; Your ESP32's IP
   ```
3. Click "Upload" button in PlatformIO
4. Enter password: `admin`

### Method 3: Arduino IDE

1. After first USB upload, device appears in: Tools > Port > Network ports
2. Select "ESP32-Monitor at <IP_ADDRESS>"
3. Click "Upload"
4. Enter password when prompted: `admin`

### OTA Update Process

When uploading via OTA, serial output will show:

```
--- OTA Update Started ---
Type: sketch
Progress: 10%
Progress: 20%
Progress: 30%
...
Progress: 100%
--- OTA Update Complete ---
Rebooting...
```

### OTA Web Dashboard

When connected to WiFi, the web interface shows an "OTA Updates" section with:
- Current device IP
- Upload command for PlatformIO
- Port and password information

### Changing OTA Password

Edit [main.cpp](src/main.cpp) line 254:

```cpp
ArduinoOTA.setPassword("your_new_password");
```

Also update in `platformio.ini`:

```ini
upload_flags =
    --port=3232
    --auth=your_new_password
```

### LED Status Indicator

- **Blinking (500ms interval)**: System ready, not connected to external WiFi
- **Solid Blue**: Connected to external WiFi network
- **Off**: ESP32 not powered or booting

## Building and Uploading

### Prerequisites

```bash
# Install PlatformIO CLI
pip3 install platformio
```

### Build

```bash
cd esp32-monitor
pio run
```

### Upload

```bash
pio run -t upload
```

### Serial Monitor

```bash
pio device monitor
```

Or use the combined command:
```bash
pio run -t upload && pio device monitor
```

## Serial Output Example

```
=================================
ESP32-C3 Super Mini - Monitor
=================================
Chip Model: ESP32-C3
Chip Revision: 4
CPU Frequency: 160 MHz
Free Heap: 327 KB
=================================

Setting up Access Point...
✓ Access Point created successfully!
✓ WiFi TX Power set to 8.5dBm (ESP32-C3 hardware limitation)
✓ AP IP configured successfully

--- Access Point Details ---
SSID: ESP32-Monitor
Password: 12345678
IP Address: 192.168.4.1
MAC Address: XX:XX:XX:XX:XX:XX
---------------------------

✓ Web server started!

========== CONNECTION INFO ==========
To connect:
1. Connect to WiFi: ESP32-Monitor
2. Password: 12345678
3. Open browser to: http://192.168.4.1
=====================================

Setup complete! LED should be blinking...
```

## Troubleshooting

### WiFi Network Not Visible

1. **Verify TX power fix is applied**:
   - Check serial output for "✓ WiFi TX Power set to 8.5dBm"

2. **Try better power source**:
   - Use USB 3.0 port directly on computer
   - Use powered USB hub
   - Try different USB cable

3. **Check serial output**:
   - Look for "✗ Failed to create Access Point!"
   - Check for crash/reboot loops

### Can't Connect to WiFi Network

1. **Verify password is correct**
2. **Check WiFi network is 2.4 GHz** (ESP32-C3 doesn't support 5 GHz)
3. **Check serial output for connection errors**

### LED Not Blinking

1. **Verify LED_PIN is correct** (GPIO 8 for ESP32-C3 Super Mini)
2. **Check serial output** to ensure code is running
3. **Try pressing RESET button**

### WiFi Roaming Not Working

1. **Verify multiple APs exist** with the same SSID (check WiFi scan results)
2. **Monitor serial output** for roaming check messages
3. **Check signal strength** - roaming only triggers below -75 dBm by default
4. **Wait 15 seconds** - roaming checks happen at intervals
5. **Adjust thresholds** if needed (see Customizing Roaming Behavior section)

### OTA Upload Fails

1. **Verify WiFi connection** - device must be on same network as computer
2. **Check IP address** - use current IP from web dashboard
3. **Verify password** - default is "admin"
4. **Check firewall** - allow port 3232
5. **Try ping test**: `ping <ESP32_IP>`
6. **Serial output** shows OTA errors if connection fails

### Duplicate Network Names in Scan

**This is now fixed!** The scanner deduplicates networks, showing only the strongest signal for each SSID. If you still see duplicates, they are actually different network names.

## Pin Configuration

- **LED_PIN**: GPIO 8 (onboard blue LED)
- **USB**: Native USB-Serial/JTAG (no external USB-to-serial chip needed)

## License

This project is provided as-is for educational and development purposes.

## Credits

- WiFi power fix discovered through community forums and GitHub issues
- Temperature sensor implementation based on ESP-IDF documentation
- Dashboard design inspired by modern web UI practices
