#ifndef INDEX_H
#define INDEX_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-C3 Monitor</title>
    <script src="https://unpkg.com/lucide@latest" async onerror="console.log('Icons not available (no internet)')"></script>
    <style>
        /* Hide icons gracefully if library doesn't load */
        [data-lucide] {
            display: inline-block;
        }
        [data-lucide]:empty {
            display: none;
        }
        :root {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen',
                'Ubuntu', 'Cantarell', 'Fira Sans', 'Droid Sans', 'Helvetica Neue', sans-serif;
            line-height: 1.5;
            font-weight: 400;
            font-synthesis: none;
            text-rendering: optimizeLegibility;
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            background: #f9fafb;
            color: #1f2937;
            min-height: 100vh;
            transition: all 0.3s ease;
        }

        body.dark-mode {
            background: #1a1a1a;
            color: #f3f4f6;
        }

        /* Scrollbar */
        ::-webkit-scrollbar {
            width: 10px;
            height: 10px;
        }

        ::-webkit-scrollbar-track {
            background: #f1f1f1;
            border-radius: 5px;
        }

        body.dark-mode ::-webkit-scrollbar-track {
            background: #2d2d2d;
        }

        ::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 5px;
        }

        ::-webkit-scrollbar-thumb:hover {
            background: #555;
        }

        button:focus-visible,
        input:focus-visible,
        select:focus-visible {
            outline: 2px solid #2563eb;
            outline-offset: 2px;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 24px;
        }

        .header {
            background: linear-gradient(135deg, #2563eb 0%, #1e40af 100%);
            border-radius: 12px;
            padding: 32px;
            margin-bottom: 24px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            animation: slideIn 0.5s ease;
        }

        body.dark-mode .header {
            background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
        }

        .header-content {
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 16px;
        }

        .header-title {
            color: white;
        }

        .header-title h1 {
            font-size: 28px;
            font-weight: 700;
            margin-bottom: 4px;
        }

        .header-title p {
            font-size: 14px;
            opacity: 0.9;
        }

        .header-actions {
            display: flex;
            gap: 12px;
            align-items: center;
        }

        .theme-toggle {
            background: rgba(255, 255, 255, 0.2);
            border: 1px solid rgba(255, 255, 255, 0.3);
            color: white;
            padding: 10px;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.2s ease;
            backdrop-filter: blur(10px);
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .theme-toggle:hover {
            background: rgba(255, 255, 255, 0.3);
            transform: scale(1.05);
        }

        .status-indicator {
            display: flex;
            align-items: center;
            gap: 8px;
            background: rgba(255, 255, 255, 0.2);
            border: 1px solid rgba(255, 255, 255, 0.3);
            padding: 8px 16px;
            border-radius: 8px;
            color: white;
            font-size: 14px;
            font-weight: 600;
            backdrop-filter: blur(10px);
        }

        .heartbeat {
            width: 8px;
            height: 8px;
            background: #10b981;
            border-radius: 50%;
            animation: pulse 2s ease-in-out infinite;
            box-shadow: 0 0 8px rgba(16, 185, 129, 0.6);
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.5; transform: scale(0.8); }
        }

        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-30px) scale(0.95);
            }
            to {
                opacity: 1;
                transform: translateY(0) scale(1);
            }
        }

        .tabs {
            display: flex;
            gap: 0;
            border-bottom: 2px solid #e5e7eb;
            margin-bottom: 24px;
            overflow-x: auto;
        }

        body.dark-mode .tabs {
            border-bottom-color: #404040;
        }

        .tab {
            padding: 16px 24px;
            border: none;
            background: transparent;
            font-size: 15px;
            font-weight: 600;
            color: #6b7280;
            border-bottom: 2px solid transparent;
            cursor: pointer;
            transition: all 0.2s ease;
            position: relative;
            top: 2px;
            white-space: nowrap;
        }

        body.dark-mode .tab {
            color: #9ca3af;
        }

        .tab.active {
            color: #2563eb;
            border-bottom-color: #2563eb;
        }

        body.dark-mode .tab.active {
            color: #3b82f6;
            border-bottom-color: #3b82f6;
        }

        .tab:hover:not(.active) {
            color: #1f2937;
        }

        body.dark-mode .tab:hover:not(.active) {
            color: #f3f4f6;
        }

        .tab-content {
            display: none;
            animation: fadeIn 0.3s ease;
        }

        .tab-content.active {
            display: block;
        }

        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 24px;
        }

        .card {
            background: white;
            border: 1px solid #e5e7eb;
            border-radius: 12px;
            padding: 24px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
            transition: all 0.2s ease;
        }

        body.dark-mode .card {
            background: #2d2d2d;
            border-color: #404040;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.3);
        }

        .card:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }

        body.dark-mode .card:hover {
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.4);
        }

        .card-header {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 16px;
        }

        .card-icon {
            width: 40px;
            height: 40px;
            background: linear-gradient(135deg, #2563eb 0%, #1e40af 100%);
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
        }

        body.dark-mode .card-icon {
            background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
        }

        .card-title {
            font-size: 18px;
            font-weight: 600;
            color: #1f2937;
        }

        body.dark-mode .card-title {
            color: #f3f4f6;
        }

        .metric-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid #f3f4f6;
        }

        body.dark-mode .metric-row {
            border-bottom-color: #3a3a3a;
        }

        .metric-row:last-child {
            border-bottom: none;
        }

        .metric-label {
            color: #6b7280;
            font-size: 14px;
            font-weight: 500;
        }

        body.dark-mode .metric-label {
            color: #9ca3af;
        }

        .metric-value {
            color: #1f2937;
            font-size: 15px;
            font-weight: 700;
            font-family: 'Courier New', monospace;
        }

        body.dark-mode .metric-value {
            color: #f3f4f6;
        }

        .btn {
            padding: 12px 24px;
            background: #2563eb;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
            display: inline-flex;
            align-items: center;
            gap: 8px;
        }

        body.dark-mode .btn {
            background: #3b82f6;
        }

        .btn:hover {
            background: #1e40af;
            transform: translateY(-1px);
            box-shadow: 0 4px 6px rgba(37, 99, 235, 0.3);
        }

        body.dark-mode .btn:hover {
            background: #2563eb;
        }

        .btn:active {
            transform: translateY(0);
        }

        .btn-secondary {
            background: #6b7280;
        }

        .btn-secondary:hover {
            background: #4b5563;
        }

        .network-list {
            max-height: 400px;
            overflow-y: auto;
        }

        .network-item {
            background: #f9fafb;
            padding: 16px;
            margin: 12px 0;
            border-radius: 8px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            cursor: pointer;
            transition: all 0.2s ease;
            border: 2px solid transparent;
        }

        body.dark-mode .network-item {
            background: #3a3a3a;
        }

        .network-item:hover {
            border-color: #2563eb;
            transform: translateX(4px);
            background: white;
        }

        body.dark-mode .network-item:hover {
            border-color: #3b82f6;
            background: #404040;
        }

        .network-info {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .network-name {
            font-weight: 600;
            color: #1f2937;
            font-size: 15px;
        }

        body.dark-mode .network-name {
            color: #f3f4f6;
        }

        .network-signal {
            color: #6b7280;
            font-size: 13px;
        }

        body.dark-mode .network-signal {
            color: #9ca3af;
        }

        .badge {
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
        }

        .badge-success {
            background: #f0fdf4;
            color: #15803d;
            border: 1px solid #bbf7d0;
        }

        body.dark-mode .badge-success {
            background: #064e3b;
            color: #6ee7b7;
            border: 1px solid #065f46;
        }

        .badge-warning {
            background: #fffbeb;
            color: #92400e;
            border: 1px solid #fcd34d;
        }

        body.dark-mode .badge-warning {
            background: #78350f;
            color: #fcd34d;
            border: 1px solid #92400e;
        }

        .badge-error {
            background: #fef2f2;
            color: #991b1b;
            border: 1px solid #fecaca;
        }

        body.dark-mode .badge-error {
            background: #7f1d1d;
            color: #fca5a5;
            border: 1px solid #991b1b;
        }

        .badge-info {
            background: #eff6ff;
            color: #1e40af;
            border: 1px solid #dbeafe;
        }

        body.dark-mode .badge-info {
            background: #1e3a8a;
            color: #93c5fd;
            border: 1px solid #1e40af;
        }

        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.5);
            backdrop-filter: blur(4px);
            z-index: 1000;
            align-items: center;
            justify-content: center;
            animation: fadeIn 0.2s ease;
        }

        .modal.active {
            display: flex;
        }

        .modal-content {
            background: white;
            padding: 32px;
            border-radius: 20px;
            max-width: 500px;
            width: 90%;
            box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1);
            animation: slideIn 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        }

        body.dark-mode .modal-content {
            background: #2d2d2d;
        }

        .modal-header {
            font-size: 20px;
            font-weight: 700;
            margin-bottom: 20px;
            color: #1f2937;
        }

        body.dark-mode .modal-header {
            color: #f3f4f6;
        }

        input, select {
            width: 100%;
            padding: 12px;
            margin: 8px 0;
            border: 2px solid #e5e7eb;
            border-radius: 8px;
            font-size: 14px;
            transition: all 0.2s ease;
            background: white;
            color: #1f2937;
        }

        body.dark-mode input,
        body.dark-mode select {
            background: #3a3a3a;
            border-color: #525252;
            color: #f3f4f6;
        }

        input:focus, select:focus {
            border-color: #2563eb;
            outline: none;
        }

        body.dark-mode input:focus,
        body.dark-mode select:focus {
            border-color: #3b82f6;
        }

        input::placeholder {
            color: #9ca3af;
        }

        .loading {
            text-align: center;
            color: #6b7280;
            padding: 24px;
            font-size: 14px;
        }

        body.dark-mode .loading {
            color: #9ca3af;
        }

        .code-block {
            background: #f9fafb;
            border: 1px solid #e5e7eb;
            border-radius: 8px;
            padding: 16px;
            font-family: 'Courier New', monospace;
            font-size: 13px;
            color: #1f2937;
            overflow-x: auto;
            margin: 12px 0;
        }

        body.dark-mode .code-block {
            background: #3a3a3a;
            border-color: #525252;
            color: #f3f4f6;
        }

        @media (max-width: 768px) {
            .container {
                padding: 16px;
            }

            .header {
                padding: 24px;
            }

            .header-title h1 {
                font-size: 24px;
            }

            .grid {
                grid-template-columns: 1fr;
            }

            .tab {
                padding: 12px 16px;
                font-size: 14px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-content">
                <div class="header-title">
                    <h1>ESP32-C3 Monitor</h1>
                    <p>System Dashboard & WiFi Manager</p>
                </div>
                <div class="header-actions">
                    <div class="status-indicator">
                        <div class="heartbeat"></div>
                        <span>Connected</span>
                    </div>
                    <button class="theme-toggle" onclick="toggleTheme()" title="Toggle dark mode">
                        <i data-lucide="moon" class="theme-icon-dark" style="width:20px;height:20px;display:none"></i>
                        <i data-lucide="sun" class="theme-icon-light" style="width:20px;height:20px"></i>
                    </button>
                </div>
            </div>
        </div>

        <div class="tabs">
            <button class="tab active" onclick="switchTab('dashboard')">
                Dashboard
            </button>
            <button class="tab" onclick="switchTab('wifi')">
                WiFi Networks
            </button>
            <button class="tab" onclick="switchTab('ota')">
                OTA Updates
            </button>
        </div>

        <div id="dashboard" class="tab-content active">
            <div class="grid">
                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">
                            <i data-lucide="activity" style="width:20px;height:20px"></i>
                        </div>
                        <h3 class="card-title">System Status</h3>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Uptime</span>
                        <span class="metric-value" id="uptime">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">CPU Frequency</span>
                        <span class="metric-value" id="cpuFreq">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Temperature</span>
                        <span class="metric-value" id="temperature">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Free Heap</span>
                        <span class="metric-value" id="freeHeap">Loading...</span>
                    </div>
                </div>

                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">
                            <i data-lucide="cpu" style="width:20px;height:20px"></i>
                        </div>
                        <h3 class="card-title">Hardware Info</h3>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Chip Model</span>
                        <span class="metric-value" id="chipModel">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Revision</span>
                        <span class="metric-value" id="chipRevision">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">CPU Cores</span>
                        <span class="metric-value" id="chipCores">Loading...</span>
                    </div>
                </div>

                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">
                            <i data-lucide="hard-drive" style="width:20px;height:20px"></i>
                        </div>
                        <h3 class="card-title">Flash Memory</h3>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Total Size</span>
                        <span class="metric-value" id="flashSize">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Sketch Size</span>
                        <span class="metric-value" id="sketchSize">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Free Space</span>
                        <span class="metric-value" id="freeSketchSpace">Loading...</span>
                    </div>
                </div>

                <div class="card">
                    <div class="card-header">
                        <div class="card-icon">
                            <i data-lucide="wifi" style="width:20px;height:20px"></i>
                        </div>
                        <h3 class="card-title">Network Status</h3>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">AP IP</span>
                        <span class="metric-value" id="apIP">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">Status</span>
                        <span id="staStatus"></span>
                    </div>
                    <div id="connectedInfo" style="display:none">
                        <div class="metric-row">
                            <span class="metric-label">Station IP</span>
                            <span class="metric-value" id="staIP"></span>
                        </div>
                        <div class="metric-row">
                            <span class="metric-label">Network</span>
                            <span class="metric-value" id="staSSID"></span>
                        </div>
                        <div class="metric-row">
                            <span class="metric-label">Signal</span>
                            <span class="metric-value" id="staRSSI"></span>
                        </div>
                    </div>
                </div>

                <div class="card" id="sensorCard">
                    <div class="card-header">
                        <div class="card-icon">
                            <i data-lucide="gauge" style="width:20px;height:20px"></i>
                        </div>
                        <h3 class="card-title">Environmental Sensors</h3>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">BMP280 Status</span>
                        <span id="bmpStatus">Loading...</span>
                    </div>
                    <div class="metric-row">
                        <span class="metric-label">AHT20 Status</span>
                        <span id="ahtStatus">Loading...</span>
                    </div>
                    <div id="sensorData" style="display:none">
                        <div class="metric-row">
                            <span class="metric-label">Temperature</span>
                            <span class="metric-value" id="sensorTemperature">-</span>
                        </div>
                        <div class="metric-row" id="pressureRow">
                            <span class="metric-label">Pressure</span>
                            <span class="metric-value" id="sensorPressure">-</span>
                        </div>
                        <div class="metric-row" id="humidityRow">
                            <span class="metric-label">Humidity</span>
                            <span class="metric-value" id="sensorHumidity">-</span>
                        </div>
                        <div class="metric-row" id="altitudeRow">
                            <span class="metric-label">Altitude</span>
                            <span class="metric-value" id="sensorAltitude">-</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div id="wifi" class="tab-content">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">
                        <i data-lucide="scan" style="width:20px;height:20px"></i>
                    </div>
                    <h3 class="card-title">Available Networks</h3>
                </div>
                <button class="btn" onclick="scanNetworks()">
                    <i data-lucide="refresh-cw" style="width:16px;height:16px"></i>
                    Scan WiFi Networks
                </button>
                <div class="network-list" id="networkList">
                    <div class="loading">Click "Scan WiFi Networks" to discover available networks</div>
                </div>
            </div>
        </div>

        <div id="ota" class="tab-content">
            <div class="card" id="otaCard">
                <div class="card-header">
                    <div class="card-icon">
                        <i data-lucide="upload-cloud" style="width:20px;height:20px"></i>
                    </div>
                    <h3 class="card-title">OTA Updates</h3>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Status</span>
                    <span class="badge badge-success">Enabled</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Hostname</span>
                    <span class="metric-value">esp32-monitor.local</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Device IP</span>
                    <span class="metric-value" id="otaIP">-</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Port</span>
                    <span class="metric-value">3232</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Password</span>
                    <span class="metric-value">admin</span>
                </div>
                <div style="margin-top: 16px;">
                    <p style="font-size: 14px; color: #6b7280; margin-bottom: 12px;">
                        <strong>Upload via PlatformIO:</strong>
                    </p>
                    <div class="code-block" id="otaCommand">
                        pio run -t upload --upload-port <span id="otaIPCommand">-</span>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="modal" id="connectModal">
        <div class="modal-content">
            <h2 class="modal-header">Connect to Network</h2>
            <input type="text" id="selectedSSID" readonly style="background: #f9fafb; cursor: not-allowed;">
            <input type="password" id="wifiPassword" placeholder="Enter WiFi password">
            <div style="display: flex; gap: 12px; margin-top: 16px;">
                <button class="btn" onclick="connectToNetwork()" style="flex: 1;">
                    <i data-lucide="wifi" style="width:16px;height:16px"></i>
                    Connect
                </button>
                <button class="btn btn-secondary" onclick="closeModal()" style="flex: 1;">
                    <i data-lucide="x" style="width:16px;height:16px"></i>
                    Cancel
                </button>
            </div>
        </div>
    </div>

    <script>
        // Theme System
        const theme = {
            current: localStorage.getItem('theme') || 'light',
            init() {
                if (this.current === 'dark') {
                    document.body.classList.add('dark-mode');
                    this.updateIcons();
                }
            },
            toggle() {
                this.current = this.current === 'light' ? 'dark' : 'light';
                document.body.classList.toggle('dark-mode');
                localStorage.setItem('theme', this.current);
                this.updateIcons();
            },
            updateIcons() {
                const isDark = this.current === 'dark';
                document.querySelector('.theme-icon-dark').style.display = isDark ? 'block' : 'none';
                document.querySelector('.theme-icon-light').style.display = isDark ? 'none' : 'block';
            }
        };

        function toggleTheme() {
            theme.toggle();
        }

        // Tab System
        function switchTab(tabName) {
            document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));

            event.target.classList.add('active');
            document.getElementById(tabName).classList.add('active');
        }

        // Data Management
        let selectedSSID = '';

        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    // System stats
                    document.getElementById('uptime').textContent = data.uptime;
                    document.getElementById('freeHeap').textContent = (data.freeHeap / 1024).toFixed(2) + ' KB';
                    document.getElementById('cpuFreq').textContent = data.cpuFreq + ' MHz';
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1) + ' °C';

                    // Chip info
                    document.getElementById('chipModel').textContent = data.chipModel;
                    document.getElementById('chipRevision').textContent = 'v' + data.chipRevision;
                    document.getElementById('chipCores').textContent = data.chipCores;

                    // Flash info
                    document.getElementById('flashSize').textContent = (data.flashSize / (1024 * 1024)).toFixed(2) + ' MB';
                    document.getElementById('sketchSize').textContent = (data.sketchSize / 1024).toFixed(2) + ' KB';
                    document.getElementById('freeSketchSpace').textContent = (data.freeSketchSpace / 1024).toFixed(2) + ' KB';

                    // Network info
                    document.getElementById('apIP').textContent = data.apIP;

                    // Environmental sensors data
                    const bmpConnected = data.bmpAvailable;
                    const ahtConnected = data.ahtAvailable;
                    const anySensorConnected = bmpConnected || ahtConnected;

                    // Update sensor status badges
                    document.getElementById('bmpStatus').innerHTML = bmpConnected
                        ? '<span class="badge badge-success">Connected</span>'
                        : '<span class="badge badge-error">Not Found</span>';

                    document.getElementById('ahtStatus').innerHTML = ahtConnected
                        ? '<span class="badge badge-success">Connected</span>'
                        : '<span class="badge badge-error">Not Found</span>';

                    // Show/hide sensor data section
                    if (anySensorConnected) {
                        document.getElementById('sensorData').style.display = 'block';

                        // Temperature (from AHT20 if available, otherwise from BMP280)
                        if (data.sensorTemperature !== undefined) {
                            document.getElementById('sensorTemperature').textContent = data.sensorTemperature.toFixed(1) + ' °C';
                        }

                        // Pressure (from BMP280)
                        if (bmpConnected && data.bmpPressure !== undefined) {
                            document.getElementById('pressureRow').style.display = 'flex';
                            document.getElementById('sensorPressure').textContent = data.bmpPressure.toFixed(1) + ' hPa';
                        } else {
                            document.getElementById('pressureRow').style.display = 'none';
                        }

                        // Humidity (from AHT20)
                        if (ahtConnected && data.ahtHumidity !== undefined) {
                            document.getElementById('humidityRow').style.display = 'flex';
                            document.getElementById('sensorHumidity').textContent = data.ahtHumidity.toFixed(1) + ' %';
                        } else {
                            document.getElementById('humidityRow').style.display = 'none';
                        }

                        // Altitude (calculated from BMP280)
                        if (bmpConnected && data.bmpAltitude !== undefined) {
                            document.getElementById('altitudeRow').style.display = 'flex';
                            document.getElementById('sensorAltitude').textContent = data.bmpAltitude.toFixed(1) + ' m';
                        } else {
                            document.getElementById('altitudeRow').style.display = 'none';
                        }
                    } else {
                        document.getElementById('sensorData').style.display = 'none';
                    }

                    if (data.staConnected) {
                        document.getElementById('staStatus').innerHTML = '<span class="badge badge-success">Connected</span>';
                        document.getElementById('connectedInfo').style.display = 'block';
                        document.getElementById('staIP').textContent = data.staIP;
                        document.getElementById('staSSID').textContent = data.staSSID;
                        document.getElementById('staRSSI').textContent = data.staRSSI + ' dBm';

                        // OTA info
                        document.getElementById('otaIP').textContent = data.staIP;
                        document.getElementById('otaIPCommand').textContent = data.staIP;
                    } else {
                        document.getElementById('staStatus').innerHTML = '<span class="badge badge-error">Disconnected</span>';
                        document.getElementById('connectedInfo').style.display = 'none';
                    }
                })
                .catch(error => console.error('Error:', error));
        }

        function scanNetworks() {
            const networkList = document.getElementById('networkList');
            networkList.innerHTML = '<div class="loading">Scanning networks...</div>';

            fetch('/scan')
                .then(response => response.json())
                .then(networks => {
                    let html = '';
                    networks.forEach(network => {
                        const signal = network.rssi;
                        let signalBadge = '';
                        let signalText = '';

                        if (signal > -50) {
                            signalBadge = 'badge-success';
                            signalText = 'Excellent';
                        } else if (signal > -60) {
                            signalBadge = 'badge-success';
                            signalText = 'Good';
                        } else if (signal > -70) {
                            signalBadge = 'badge-warning';
                            signalText = 'Fair';
                        } else {
                            signalBadge = 'badge-error';
                            signalText = 'Weak';
                        }

                        const lockIcon = network.encryption !== 0
                            ? '<i data-lucide="lock" style="width:16px;height:16px;color:#6b7280"></i>'
                            : '<i data-lucide="unlock" style="width:16px;height:16px;color:#6b7280"></i>';

                        html += `
                            <div class="network-item" onclick="selectNetwork('${network.ssid}')">
                                <div class="network-info">
                                    ${lockIcon}
                                    <div>
                                        <div class="network-name">${network.ssid}</div>
                                        <div class="network-signal">${signalText} (${network.rssi} dBm)</div>
                                    </div>
                                </div>
                                <span class="badge ${signalBadge}">${signalText}</span>
                            </div>
                        `;
                    });
                    networkList.innerHTML = html || '<div class="loading">No networks found</div>';
                    // Only call lucide if it's loaded (requires internet)
                    if (typeof lucide !== 'undefined') {
                        lucide.createIcons();
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    networkList.innerHTML = '<div class="loading">Error scanning networks</div>';
                });
        }

        function selectNetwork(ssid) {
            selectedSSID = ssid;
            document.getElementById('selectedSSID').value = ssid;
            document.getElementById('wifiPassword').value = '';
            document.getElementById('connectModal').classList.add('active');
        }

        function closeModal() {
            document.getElementById('connectModal').classList.remove('active');
        }

        function connectToNetwork() {
            const password = document.getElementById('wifiPassword').value;

            fetch('/connect?ssid=' + encodeURIComponent(selectedSSID) + '&password=' + encodeURIComponent(password))
                .then(response => response.text())
                .then(data => {
                    alert(data);
                    closeModal();
                    setTimeout(updateStatus, 3000);
                })
                .catch(error => {
                    alert('Error connecting to network');
                    console.error('Error:', error);
                });
        }

        // Initialize
        theme.init();
        updateStatus();
        setInterval(updateStatus, 2000);

        // Initialize Lucide icons (only if library loaded - requires internet)
        if (typeof lucide !== 'undefined') {
            lucide.createIcons();
        }

        // Close modal on outside click
        document.getElementById('connectModal').addEventListener('click', function(e) {
            if (e.target === this) {
                closeModal();
            }
        });
    </script>
</body>
</html>
)rawliteral";

#endif
