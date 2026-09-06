#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "Config.h"

struct ButtonEvents {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool center = false;
};

class DesktopUI {
 public:
  explicit DesktopUI(TFT_eSPI& tft);

  bool begin();
  void setSdAvailable(bool available);
  void setSdIcon(const std::vector<String>& lines);
  void setNetworkInfo(const String& sta, const String& apIp);
  void setClockInfo(const String& timeText, const String& dateText);
  void setWeatherInfo(const String& weatherText);
  void setNote(uint8_t index, const String& note);
  void setTimetable(uint8_t day, uint8_t period, const String& text);
  void update(uint32_t now, const ButtonEvents& events);
  void render(uint32_t now);

  Config::ScreenState screen() const { return _screen; }

 private:
  enum class GameMode : uint8_t { Menu, Tamagotchi, Chess, Dungeon };
  enum class PomodoroPhase : uint8_t { Focus, Rest };

  TFT_eSPI& _tft;
  TFT_eSprite _img;
  bool _spriteReady = false;
  bool _sdAvailable = false;
  std::vector<String> _sdIcon;
  String _wifiStatus = "WIFI ...";
  String _apIp = "0.0.0.0";
  String _timeText = "--:--";
  String _dateText = "-- --- ----";
  String _weatherText = "CAT LAI: updating";
  String _notes[3] = {"Open AP setup", "Edit notes", "Sync Vietnam time"};
  String _timetable[7][7];

  Config::ScreenState _screen = Config::ScreenState::Dashboard;
  uint32_t _lastInteractionMs = 0;

  uint8_t _timetableDay = 1;
  bool _pomodoroRunning = false;
  PomodoroPhase _pomoPhase = PomodoroPhase::Focus;
  uint32_t _pomoRemain = Config::POMODORO_FOCUS_SEC;
  uint32_t _lastPomoTick = 0;

  uint8_t _asciiMode = 0;
  uint8_t _mediaIndex = 0;
  bool _mediaPlaying = true;
  uint8_t _gameSelect = 0;
  GameMode _gameMode = GameMode::Menu;
  uint32_t _gameSeed = 1;
  uint8_t _petFood = 7;
  uint8_t _petMood = 8;
  uint8_t _petClean = 6;
  uint8_t _settingsSelect = 0;
  uint8_t _powerIndex = 0;
  uint8_t _dimIndex = 1;
  uint8_t _colorIndex = 16;

  void handleGlobal(uint32_t now, const ButtonEvents& events);
  void handleScreen(uint32_t now, const ButtonEvents& events);
  void tickPomodoro(uint32_t now);
  void tickGames(uint32_t now);
  void resetAutoplay(uint32_t now);

  void clear();
  uint16_t fg() const;
  uint16_t bg() const;
  uint16_t mid() const;
  bool reverseTheme() const;
  void drawFrame(int16_t x, int16_t y, int16_t w, int16_t h);
  void drawTitle(int16_t x, int16_t y, int16_t w, const String& title);
  void drawFooter(const String& hint);
  void drawLabelBox(int16_t x, int16_t y, int16_t w, int16_t h, const String& label);
  void drawTextFit(const String& text, int16_t x, int16_t y, int16_t maxChars);
  void drawProgress(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t value);
  void drawBigDigits(const String& text, int16_t x, int16_t y, uint8_t scale);
  void drawAsciiLines(const char* const* lines, uint8_t count, int16_t x, int16_t y, uint8_t gap = 8);
  void drawStatusBar(const String& title, uint32_t now, bool autoHide);
  void drawGridCell(uint8_t col, uint8_t row, char ch);
  void drawBuiltInIcon(int16_t x, int16_t y);

  void renderDashboard(uint32_t now);
  void renderTimetable();
  void renderPomodoro();
  void renderAsciiShowcase(uint32_t now);
  void renderMedia(uint32_t now);
  void renderGames(uint32_t now);
  void renderSettings();
};
