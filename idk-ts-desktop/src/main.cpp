#include <Arduino.h>
#include <AnimatedGIF.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>
#include <vector>

#include "Config.h"
#include "UI.h"

TFT_eSPI tft;
DesktopUI ui(tft);
AnimatedGIF gif;
WebServer server(80);
Preferences prefs;

struct ButtonDebounce {
  uint8_t pin;
  bool stable = true;
  bool lastRead = true;
  uint32_t changedAt = 0;
  bool fell = false;

  explicit ButtonDebounce(uint8_t gpio) : pin(gpio) {}
};

ButtonDebounce buttons[] = {
    ButtonDebounce(Config::BTN_UP),
    ButtonDebounce(Config::BTN_DOWN),
    ButtonDebounce(Config::BTN_LEFT),
    ButtonDebounce(Config::BTN_RIGHT),
    ButtonDebounce(Config::BTN_CENTER),
};

uint32_t lastFrame = 0;
uint32_t lastClockSync = 0;
uint32_t lastWeatherFetch = 0;
String wifiStatus = "WIFI BOOT";
String apIpText = "0.0.0.0";

void setupBacklight() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(Config::PIN_TFT_BL, 0);
  ledcWrite(0, 220);
}

ButtonEvents readButtons(uint32_t now) {
  ButtonEvents events;
  bool* targets[] = {&events.up, &events.down, &events.left, &events.right, &events.center};
  for (uint8_t i = 0; i < static_cast<uint8_t>(Config::Button::Count); ++i) {
    ButtonDebounce& b = buttons[i];
    b.fell = false;
    bool raw = digitalRead(b.pin);
    if (raw != b.lastRead) {
      b.lastRead = raw;
      b.changedAt = now;
    }
    if ((now - b.changedAt) >= Config::DEBOUNCE_MS && raw != b.stable) {
      b.stable = raw;
      if (b.stable == LOW) b.fell = true;
    }
    *targets[i] = b.fell;
  }
  return events;
}

std::vector<String> loadIconFromSd() {
  std::vector<String> lines;
  File f = SD.open("/icon.txt", FILE_READ);
  if (!f) return lines;
  while (f.available() && lines.size() < 12) {
    String line = f.readStringUntil('\n');
    line.replace("\r", "");
    if (line.length() > 18) line = line.substring(0, 18);
    lines.push_back(line);
  }
  f.close();
  return lines;
}

bool initSd() {
  SPI.begin(Config::SD_SCK, Config::SD_MISO, Config::SD_MOSI, Config::SD_CS);
  return SD.begin(Config::SD_CS, SPI, 20000000);
}

String htmlEscape(const String& src) {
  String out;
  out.reserve(src.length() + 8);
  for (uint16_t i = 0; i < src.length(); ++i) {
    char c = src[i];
    if (c == '&') out += F("&amp;");
    else if (c == '<') out += F("&lt;");
    else if (c == '>') out += F("&gt;");
    else if (c == '"') out += F("&quot;");
    else out += c;
  }
  return out;
}

void loadEditableData() {
  prefs.begin("desktop", false);
  for (uint8_t i = 0; i < 3; ++i) {
    String key = "note" + String(i);
    if (prefs.isKey(key.c_str())) ui.setNote(i, prefs.getString(key.c_str(), ""));
  }
  for (uint8_t d = 0; d < 7; ++d) {
    for (uint8_t p = 0; p < 7; ++p) {
      String key = "d" + String(d) + "p" + String(p);
      if (prefs.isKey(key.c_str())) ui.setTimetable(d, p, prefs.getString(key.c_str(), ""));
    }
  }
}

void handleRoot() {
  String page = F("<!doctype html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
                  "<style>body{font-family:monospace;background:#050505;color:#eee;margin:16px}"
                  "input{width:100%;box-sizing:border-box;margin:3px 0 8px;padding:8px;background:#111;color:#fff;border:1px solid #777}"
                  "button{padding:10px 14px;background:#fff;color:#000;border:0;font-weight:bold}"
                  ".grid{display:grid;grid-template-columns:repeat(7,1fr);gap:6px}@media(max-width:800px){.grid{grid-template-columns:1fr}}</style>"
                  "</head><body><h2>idk-ts-desktop</h2><form method=POST action=/save><h3>Notes</h3>");
  for (uint8_t i = 0; i < 3; ++i) {
    String key = "note" + String(i);
    page += "N" + String(i + 1) + "<input name='" + key + "' value='" + htmlEscape(prefs.getString(key.c_str(), "")) + "'>";
  }
  page += F("<h3>Timetable</h3><div class=grid>");
  const char* days[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  for (uint8_t d = 0; d < 7; ++d) {
    page += "<div><b>";
    page += days[d];
    page += "</b>";
    for (uint8_t p = 0; p < 7; ++p) {
      String key = "d" + String(d) + "p" + String(p);
      page += "<input name='" + key + "' value='" + htmlEscape(prefs.getString(key.c_str(), "")) + "'>";
    }
    page += "</div>";
  }
  page += F("</div><button type=submit>Save</button></form></body></html>");
  server.send(200, "text/html", page);
}

void handleSave() {
  for (uint8_t i = 0; i < 3; ++i) {
    String key = "note" + String(i);
    String val = server.arg(key);
    if (val.length() > 28) val = val.substring(0, 28);
    prefs.putString(key.c_str(), val);
    ui.setNote(i, val);
  }
  for (uint8_t d = 0; d < 7; ++d) {
    for (uint8_t p = 0; p < 7; ++p) {
      String key = "d" + String(d) + "p" + String(p);
      String val = server.arg(key);
      if (val.length() > 18) val = val.substring(0, 18);
      prefs.putString(key.c_str(), val);
      ui.setTimetable(d, p, val);
    }
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "saved");
}

void setupNetwork() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(Config::AP_SSID, Config::AP_PASS);
  apIpText = WiFi.softAPIP().toString();
  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASS);
  configTzTime(Config::TZ_VIETNAM, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  wifiStatus = "WIFI CONNECTING";
  ui.setNetworkInfo(wifiStatus, apIpText);
}

void updateClockUi() {
  struct tm info;
  if (getLocalTime(&info, 5)) {
    char tbuf[6];
    char dbuf[18];
    strftime(tbuf, sizeof(tbuf), "%H:%M", &info);
    strftime(dbuf, sizeof(dbuf), "%a %d %b %Y", &info);
    ui.setClockInfo(tbuf, dbuf);
  }
}

void updateWifiStatus() {
  static wl_status_t lastStatus = WL_IDLE_STATUS;
  wl_status_t current = WiFi.status();
  if (current != lastStatus) {
    lastStatus = current;
    if (current == WL_CONNECTED) {
      wifiStatus = "STA " + WiFi.localIP().toString();
    } else {
      wifiStatus = "WIFI CONNECTING";
    }
    ui.setNetworkInfo(wifiStatus, apIpText);
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/weather?lat=") + String(Config::CAT_LAI_LAT, 4) +
               "&lon=" + String(Config::CAT_LAI_LON, 4) +
               "&appid=" + Config::WEATHER_API_KEY + "&units=metric&lang=vi";
  http.setTimeout(3500);
  if (!http.begin(url)) return;
  int code = http.GET();
  if (code == 200) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream());
    if (!err) {
      float temp = doc["main"]["temp"] | 0.0f;
      int humidity = doc["main"]["humidity"] | 0;
      const char* desc = doc["weather"][0]["main"] | "Weather";
      String weather = String(static_cast<int>(roundf(temp))) + "C " + String(humidity) + "% " + desc;
      ui.setWeatherInfo(weather);
    }
  } else {
    ui.setWeatherInfo("weather HTTP " + String(code));
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(150);

  pinMode(Config::BTN_UP, INPUT_PULLUP);
  pinMode(Config::BTN_DOWN, INPUT_PULLUP);
  pinMode(Config::BTN_LEFT, INPUT_PULLUP);
  pinMode(Config::BTN_RIGHT, INPUT_PULLUP);
  pinMode(Config::BTN_CENTER, INPUT_PULLUP);
  pinMode(Config::SD_CS, OUTPUT);
  digitalWrite(Config::SD_CS, HIGH);
  pinMode(Config::PIN_TFT_POWER, OUTPUT);
  digitalWrite(Config::PIN_TFT_POWER, HIGH);

  setupBacklight();

  bool sdReady = initSd();
  std::vector<String> sdIcon;
  if (sdReady) sdIcon = loadIconFromSd();

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  bool uiReady = ui.begin();
  ui.setSdAvailable(sdReady);
  loadEditableData();
  setupNetwork();
  updateClockUi();

  Serial.println();
  Serial.println("idk-ts-desktop");
  Serial.printf("UI sprite: %s\n", uiReady ? "ready" : "failed");
  Serial.printf("SD: %s\n", sdReady ? "ready" : "missing");
  Serial.printf("AP: %s pass=%s\n", apIpText.c_str(), Config::AP_PASS);

  lastFrame = millis();
}

void loop() {
  uint32_t now = millis();
  server.handleClient();
  updateWifiStatus();
  if (now - lastClockSync >= 1000) {
    lastClockSync = now;
    updateClockUi();
  }
  if (now - lastWeatherFetch >= Config::WEATHER_REFRESH_MS || lastWeatherFetch == 0) {
    lastWeatherFetch = now;
    fetchWeather();
  }
  ButtonEvents events = readButtons(now);
  ui.update(now, events);

  if (now - lastFrame >= Config::UI_FRAME_MS) {
    lastFrame = now;
    ui.render(now);
  }
}
