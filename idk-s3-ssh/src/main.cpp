#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <vector>
#include "libssh_esp32.h"
#include <libssh/libssh.h>

// Modes
enum class DeviceMode { CONFIG, SSH_CLIENT };
static DeviceMode current_device_mode = DeviceMode::CONFIG;

// Configuration settings
static String wifi_ssid = "";
static String wifi_pass = "";
static String ssh_host = "";
static uint32_t ssh_port = 22;
static String ssh_user = "";
static String ssh_pass = "";
static String ssh_cmd = "fastfetch";

// UI / Rendering
static M5Canvas canvas(&M5.Display);
// Custom colors (RGB565 format)
static constexpr uint16_t COLOR_CYBER_BG = 0x0802;   // Dark Purple
static constexpr uint16_t COLOR_CYBER_FG = 0x07FF;   // Neon Cyan
static constexpr uint16_t COLOR_ALERT_FG = 0xF81F;   // Neon Magenta
static constexpr uint16_t COLOR_MATRIX_BG = 0x0000;  // Deep Black
static constexpr uint16_t COLOR_SUCCESS_FG = 0x07E0; // Matrix Green

// Terminal display buffer
static std::vector<String> terminal_lines;
static constexpr int MAX_TERMINAL_LINES = 14;
static portMUX_TYPE terminal_mux = portMUX_INITIALIZER_UNLOCKED;

// Networking servers for Config Mode
static WebServer web_server(80);
static DNSServer dns_server;
static constexpr byte DNS_PORT = 53;

// Background SSH task handles
static TaskHandle_t ssh_task_handle = nullptr;
static bool stop_ssh_requested = false;
static String ssh_status_message = "Idle";
static bool ssh_connected = false;

// Load config from NVS Preferences
void load_configuration() {
    Preferences prefs;
    // Open in read/write mode (second arg false) to allow saving default settings if none exist
    prefs.begin("ssh-config", false);
    wifi_ssid = prefs.getString("ssid", "");
    wifi_pass = prefs.getString("pass", "");
    ssh_host = prefs.getString("host", "");
    ssh_port = prefs.getUInt("port", 0);
    ssh_user = prefs.getString("user", "");
    ssh_pass = prefs.getString("sshpass", "");
    ssh_cmd = prefs.getString("cmd", "");
    
    // Set default parameters for auto-connection out of the box
    if (wifi_ssid.isEmpty()) {
        wifi_ssid = "Quan Le";
        wifi_pass = "15032011";
        ssh_host = "192.168.1.5";
        ssh_port = 22;
        ssh_user = "truonglangquan";
        ssh_pass = "15031169";
        ssh_cmd = "fastfetch";
        
        // Write to flash so they persist
        prefs.putString("ssid", wifi_ssid);
        prefs.putString("pass", wifi_pass);
        prefs.putString("host", ssh_host);
        prefs.putUInt("port", ssh_port);
        prefs.putString("user", ssh_user);
        prefs.putString("sshpass", ssh_pass);
        prefs.putString("cmd", ssh_cmd);
    }
    prefs.end();
}

// Save config to NVS Preferences
void save_configuration(String ssid, String pass, String host, uint32_t port, String user, String sshpass, String cmd) {
    Preferences prefs;
    prefs.begin("ssh-config", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.putString("host", host);
    prefs.putUInt("port", port);
    prefs.putString("user", user);
    prefs.putString("sshpass", sshpass);
    prefs.putString("cmd", cmd);
    prefs.end();
}

// Add line to circular terminal scroll buffer
void add_terminal_line(String line) {
    portENTER_CRITICAL(&terminal_mux);
    terminal_lines.push_back(line);
    while (terminal_lines.size() > MAX_TERMINAL_LINES) {
        terminal_lines.erase(terminal_lines.begin());
    }
    portEXIT_CRITICAL(&terminal_mux);
}

// Clean up incoming data (removes most ANSI and formatting codes)
void process_incoming_terminal_data(const char* data, int len) {
    static String current_line = "";
    for (int i = 0; i < len; i++) {
        char c = data[i];
        if (c == '\r') {
            continue;
        } else if (c == '\n') {
            add_terminal_line(current_line);
            current_line = "";
        } else if (c == 27) { // Escape character (ANSI)
            // Skip ANSI sequences (e.g. \033[1;31m)
            while (i < len && data[i] != 'm' && data[i] != 'J' && data[i] != 'H' && data[i] != 'K' && data[i] != 'G') {
                i++;
            }
        } else if (c >= 32 && c <= 126) {
            current_line += c;
        }
    }
}

// Captive Portal HTML configuration page
const char* SETUP_HTML = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>M5Stick S3 SSH Setup</title>
  <style>
    body {
      background-color: #060713;
      color: #00F5FF;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      margin: 0;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .container {
      background-color: #0C0020;
      border: 2px solid #4D0099;
      border-radius: 10px;
      padding: 30px;
      max-width: 400px;
      width: 100%;
      box-shadow: 0 0 15px #4D0099;
    }
    h1 {
      color: #FF007F;
      text-align: center;
      margin-bottom: 30px;
      text-shadow: 0 0 8px #FF007F;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #00F5FF;
      font-weight: bold;
    }
    input[type="text"], input[type="password"], input[type="number"] {
      width: 100%;
      padding: 10px;
      margin-bottom: 20px;
      border: 1px solid #4D0099;
      border-radius: 5px;
      background-color: #12002e;
      color: #39FF14;
      box-sizing: border-box;
      outline: none;
    }
    input:focus {
      border-color: #00F5FF;
      box-shadow: 0 0 5px #00F5FF;
    }
    input[type="submit"] {
      width: 100%;
      padding: 12px;
      background-color: #FF007F;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
      text-shadow: 0 0 5px white;
      box-shadow: 0 0 10px #FF007F;
    }
    input[type="submit"]:hover {
      background-color: #ff3399;
      box-shadow: 0 0 15px #ff3399;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>SSH FIRMWARE SETUP</h1>
    <form action="/save" method="POST">
      <label>WiFi SSID</label>
      <input type="text" name="ssid" placeholder="Enter WiFi Name" required>
      
      <label>WiFi Password</label>
      <input type="password" name="pass" placeholder="Enter WiFi Password">
      
      <label>SSH Host (IP or ngrok URL)</label>
      <input type="text" name="host" placeholder="e.g. 0.tcp.ap.ngrok.io" required>
      
      <label>SSH Port</label>
      <input type="number" name="port" value="22" required>
      
      <label>SSH Username</label>
      <input type="text" name="user" placeholder="Enter SSH User" required>
      
      <label>SSH Password</label>
      <input type="password" name="sshpass" placeholder="Enter SSH Password" required>
      
      <label>Startup Command</label>
      <input type="text" name="cmd" value="fastfetch" placeholder="e.g. uname -a or htop">
      
      <input type="submit" value="SAVE & CONNECT">
    </form>
  </div>
</body>
</html>
)rawhtml";

// Webserver save handler
void handle_save() {
    String ssid = web_server.arg("ssid");
    String pass = web_server.arg("pass");
    String host = web_server.arg("host");
    uint32_t port = web_server.arg("port").toInt();
    String user = web_server.arg("user");
    String sshpass = web_server.arg("sshpass");
    String cmd = web_server.arg("cmd");
    
    save_configuration(ssid, pass, host, port, user, sshpass, cmd);
    
    web_server.send(200, "text/html", R"html(
    <html>
      <body style="background-color:#060713;color:#39FF14;font-family:sans-serif;text-align:center;padding:50px;">
        <h1 style="text-shadow:0 0 10px #39FF14;">CREDENTIALS SAVED!</h1>
        <p style="color:#00F5FF;font-size:20px;">M5Stick S3 is rebooting to establish secure SSH tunnel...</p>
      </body>
    </html>
    )html");
    
    delay(1000);
    ESP.restart();
}

// Webserver home page handler
void handle_home() {
    web_server.send(200, "text/html", SETUP_HTML);
}

// Setup captive portal AP
void start_config_portal() {
    current_device_mode = DeviceMode::CONFIG;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("M5Stick-SSH-Setup");
    
    dns_server.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    
    web_server.on("/", handle_home);
    web_server.on("/save", HTTP_POST, handle_save);
    web_server.onNotFound([]() {
        web_server.send(200, "text/html", SETUP_HTML); // Redirect all requests
    });
    web_server.begin();
    
    Serial.println("Config portal active on AP: M5Stick-SSH-Setup");
}

// Background FreeRTOS SSH Client Worker Task
void ssh_worker_task(void *pvParameters) {
    ssh_status_message = "WiFi Connect...";
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
        if (stop_ssh_requested) goto EXIT_TASK;
        delay(500);
        retries++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        ssh_status_message = "WiFi Connect Failed!";
        add_terminal_line("[ERR] WiFi Connection Timeout!");
        delay(3000);
        stop_ssh_requested = true;
        goto EXIT_TASK;
    }
    
    add_terminal_line("[SYS] WiFi Connected! IP: " + WiFi.localIP().toString());
    
    while (!stop_ssh_requested) {
        ssh_connected = false;
        ssh_status_message = "Connecting SSH...";
        add_terminal_line("[SYS] Establishing secure SSH tunnel...");
        add_terminal_line("[SYS] Host: " + ssh_host + ":" + String(ssh_port));
        
        libssh_begin();
        ssh_session session = ssh_new();
        if (!session) {
            add_terminal_line("[ERR] Session init failed.");
            delay(5000);
            continue;
        }
        
        int strict_host_key = 0;
        ssh_options_set(session, SSH_OPTIONS_HOST, ssh_host.c_str());
        ssh_options_set(session, SSH_OPTIONS_PORT, &ssh_port);
        ssh_options_set(session, SSH_OPTIONS_USER, ssh_user.c_str());
        ssh_options_set(session, SSH_OPTIONS_STRICTHOSTKEYCHECK, &strict_host_key);
        
        int timeout_ms = 10000;
        ssh_options_set(session, SSH_OPTIONS_TIMEOUT_USEC, &timeout_ms);
        
        if (ssh_connect(session) != SSH_OK) {
            add_terminal_line("[ERR] SSH Connect error: " + String(ssh_get_error(session)));
            ssh_free(session);
            delay(5000);
            continue;
        }
        
        ssh_status_message = "Authenticating...";
        if (ssh_userauth_password(session, nullptr, ssh_pass.c_str()) != SSH_AUTH_SUCCESS) {
            add_terminal_line("[ERR] Auth error: " + String(ssh_get_error(session)));
            ssh_disconnect(session);
            ssh_free(session);
            delay(5000);
            continue;
        }
        
        add_terminal_line("[SYS] SSH Auth Success!");
        ssh_status_message = "Opening shell...";
        
        ssh_channel channel = ssh_channel_new(session);
        if (!channel || ssh_channel_open_session(channel) != SSH_OK) {
            add_terminal_line("[ERR] Channel opening failed.");
            if (channel) ssh_channel_free(channel);
            ssh_disconnect(session);
            ssh_free(session);
            delay(5000);
            continue;
        }
        
        // PTY sizing (widescreen text grid width 40, height 14)
        if (ssh_channel_request_pty_size(channel, "vt100", 40, 14) != SSH_OK) {
            add_terminal_line("[WARN] PTY request failed.");
        }
        
        if (ssh_channel_request_shell(channel) != SSH_OK) {
            add_terminal_line("[ERR] Shell request failed.");
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            ssh_disconnect(session);
            ssh_free(session);
            delay(5000);
            continue;
        }
        
        ssh_connected = true;
        ssh_status_message = "Connected";
        add_terminal_line("[SYS] Remote shell spawned successfully.");
        
        // Send startup command
        if (!ssh_cmd.isEmpty()) {
            add_terminal_line("[SYS] Run: " + ssh_cmd);
            String full_cmd = ssh_cmd + "\n";
            ssh_channel_write(channel, full_cmd.c_str(), full_cmd.length());
        }
        
        char buffer[256];
        while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel) && !stop_ssh_requested) {
            if (WiFi.status() != WL_CONNECTED) {
                add_terminal_line("[ERR] WiFi connection lost.");
                break;
            }
            
            // Non-blocking read
            int nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
            if (nbytes > 0) {
                process_incoming_terminal_data(buffer, nbytes);
            }
            
            delay(10);
        }
        
        ssh_connected = false;
        ssh_status_message = "Disconnected";
        add_terminal_line("[WARN] Remote shell session closed.");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        
        if (stop_ssh_requested) break;
        delay(5000); // Retry delay
    }
    
EXIT_TASK:
    ssh_status_message = "Off";
    ssh_connected = false;
    WiFi.disconnect(true);
    ssh_task_handle = nullptr;
    vTaskDelete(NULL);
}

void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = false;
    cfg.internal_rtc = false;
    cfg.internal_mic = false;
    cfg.internal_spk = false;
    M5.begin(cfg);
    
    M5.Display.setBaseColor(TFT_BLACK);
    M5.Display.setRotation(1); // 240x135 widescreen landscape
    M5.Display.setBrightness(128);
    
    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(240, 135);
    
    load_configuration();
    
    // Check if configuration exists
    // If not, or if Button A is held on boot, start Config Portal AP
    M5.update();
    bool btn_held = M5.BtnA.isPressed();
    
    if (wifi_ssid.isEmpty() || btn_held) {
        canvas.fillSprite(COLOR_CYBER_BG);
        canvas.setTextColor(COLOR_ALERT_FG);
        canvas.setFont(&fonts::Font0);
        canvas.setTextSize(2);
        canvas.drawCenterString("CONFIG AP MODE", 120, 20);
        
        canvas.setTextSize(1);
        canvas.setTextColor(COLOR_CYBER_FG);
        canvas.drawCenterString("WiFi AP: M5Stick-SSH-Setup", 120, 55);
        canvas.drawCenterString("Open http://192.168.4.1", 120, 75);
        
        canvas.setTextColor(COLOR_SUCCESS_FG);
        canvas.drawCenterString("Configure Wifi & SSH tunnel", 120, 105);
        canvas.pushSprite(0, 0);
        
        start_config_portal();
    } else {
        current_device_mode = DeviceMode::SSH_CLIENT;
        add_terminal_line("[SYS] Initializing SSH Client...");
        
        // Spawn SSH worker task on Core 1 (Core 0 handles OS/radio, Core 1 handles UI and user processes)
        xTaskCreatePinnedToCore(
            ssh_worker_task,
            "ssh_task",
            1024 * 16, // Stack size (16KB required for cryptography/SSH handshake)
            NULL,
            2,         // Priority
            &ssh_task_handle,
            1          // Core 1
        );
    }
}

void loop() {
    M5.update();
    
    if (current_device_mode == DeviceMode::CONFIG) {
        dns_server.processNextRequest();
        web_server.handleClient();
        delay(1);
    } else {
        // SSH monitor display loop
        
        // Tapping Button A forces restart back to Captive Portal Config AP
        if (M5.BtnA.wasPressed()) {
            stop_ssh_requested = true;
            canvas.fillSprite(COLOR_CYBER_BG);
            canvas.setTextColor(COLOR_ALERT_FG);
            canvas.setFont(&fonts::Font0);
            canvas.setTextSize(2);
            canvas.drawCenterString("RESETTING DEVICE...", 120, 50);
            canvas.pushSprite(0, 0);
            
            // Wait for SSH thread to teardown safely
            while (ssh_task_handle != nullptr) {
                delay(10);
            }
            
            // Clean up config to force config portal on next boot
            Preferences prefs;
            prefs.begin("ssh-config", false);
            prefs.clear();
            prefs.end();
            
            delay(500);
            ESP.restart();
        }
        
        // Render screen double-buffered
        canvas.fillSprite(COLOR_CYBER_BG);
        
        // Header
        canvas.fillRect(0, 0, 240, 14, 0x1804); // Translucent deep bar
        canvas.drawFastHLine(0, 14, 240, 0x4D00); // Glowing borders
        
        canvas.setTextColor(0xFFFF);
        canvas.setFont(&fonts::Font0);
        canvas.setTextSize(1);
        canvas.drawString("SYS-MONITOR v1.0", 6, 3);
        
        // Status text with glowing color indicator
        if (ssh_connected) {
            canvas.setTextColor(COLOR_SUCCESS_FG);
            canvas.drawRightString("[CONNECTED]", 234, 3);
        } else {
            canvas.setTextColor(COLOR_ALERT_FG);
            canvas.drawRightString("[" + ssh_status_message + "]", 234, 3);
        }
        
        // Print scrolled terminal lines
        portENTER_CRITICAL(&terminal_mux);
        int y_pos = 18;
        canvas.setTextColor(COLOR_CYBER_FG);
        for (const auto& line : terminal_lines) {
            canvas.drawString(line, 6, y_pos);
            y_pos += 8; // character height is 8 pixels at size 1
        }
        portEXIT_CRITICAL(&terminal_mux);
        
        // Render double buffer canvas to LCD display
        canvas.pushSprite(0, 0);
        delay(33); // Cap UI rendering at 30 FPS
    }
}
