#include "UI.h"

#include <esp_sleep.h>
#include <esp_system.h>

namespace {
const char* kDayNames[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
const char* kPeriods[7][7] = {
    {"Math", "Lit", "Phys", "Chem", "Bio", "Eng", "Lab"},
    {"Math", "Phys", "Chem", "History", "Civics", "Art", "Club"},
    {"Geo", "Math", "Lit", "Eng", "IT", "PE", "Read"},
    {"Phys", "Chem", "Bio", "Math", "Music", "IT", "Club"},
    {"Lit", "History", "Geo", "Math", "Eng", "PE", "Review"},
    {"Project", "Project", "Maker", "Maker", "Clean", "Plan", "Rest"},
    {"Rest", "Family", "Read", "Prep", "Walk", "Plan", "Sleep"},
};

const char* const kBuiltInIcon[] = {
    "       |||||",
    "      |||||||   ||",
    "    |||||||||| ||||",
    "   |||||||||||||||",
    "  ||||||||||||||",
    " ||||||||||||||||",
    "|||||||||||| |||||",
    "  |||||||||   |||",
    "   |||||      ||",
};

const char* const kSpaceArt[] = {
    "   .-''''-.       o",
    " .'  .--.  '.  .-.",
    "/  .'    '.  \\/   \\",
    "|  |  ()  |  |  o |",
    "\\  '.____.'  /\\   /",
    " '.        .'  '-'",
    "   '-.__.-'   orbit",
};

const char* const kQuantumArt[] = {
    " +  e-  .----.  +",
    "   \\   / psi  \\",
    "e---(  |  H   | )---e",
    "   /   \\______/  \\",
    "  qbit: |0>+|1>",
    "  spin:  up/down",
};

const char* const kMathArt[] = {
    " y = sin(x) + n",
    "      /\\      /\\",
    " ___ /  \\____/  \\__",
    " det(A)-lambda I=0",
    " sum 1/n^2 = pi^2/6",
    " fractal: z=z^2+c",
};

const char* const kCarArt[] = {
    "        _________",
    "   ____/ __ | __ \\____",
    " _/  _  /__\\|/__\\  _  \\_",
    "|_--| |----------| |--_|",
    "   O-'            '-O",
};

const char* const kWaveArt[] = {
    "  MEDIA STREAM",
    "  |#| |##| |###| |##|",
    "  .  .:  .::  .:  .",
    "  /\\/\\/\\/\\/\\/\\/\\/\\",
    "  << ascii frame >>",
};

uint8_t clampByte(int value) {
  if (value < 0) return 0;
  if (value > 10) return 10;
  return static_cast<uint8_t>(value);
}
}  // namespace

DesktopUI::DesktopUI(TFT_eSPI& tft) : _tft(tft), _img(&tft) {}

bool DesktopUI::begin() {
  _img.setColorDepth(16);
  _spriteReady = _img.createSprite(Config::SCREEN_W, Config::SCREEN_H) != nullptr;
  if (!_spriteReady) return false;
  _img.setTextFont(1);
  _img.setTextSize(1);
  _img.setTextWrap(false);
  for (uint8_t d = 0; d < 7; ++d) {
    for (uint8_t p = 0; p < 7; ++p) _timetable[d][p] = kPeriods[d][p];
  }
  return true;
}

void DesktopUI::setSdAvailable(bool available) { _sdAvailable = available; }

void DesktopUI::setSdIcon(const std::vector<String>& lines) { _sdIcon = lines; }

void DesktopUI::setNetworkInfo(const String& sta, const String& apIp) {
  _wifiStatus = sta;
  _apIp = apIp;
}

void DesktopUI::setClockInfo(const String& timeText, const String& dateText) {
  _timeText = timeText;
  _dateText = dateText;
}

void DesktopUI::setWeatherInfo(const String& weatherText) { _weatherText = weatherText; }

void DesktopUI::setNote(uint8_t index, const String& note) {
  if (index < 3) _notes[index] = note;
}

void DesktopUI::setTimetable(uint8_t day, uint8_t period, const String& text) {
  if (day < 7 && period < 7) _timetable[day][period] = text;
}

void DesktopUI::update(uint32_t now, const ButtonEvents& events) {
  handleGlobal(now, events);
  handleScreen(now, events);
  tickPomodoro(now);
  tickGames(now);
}

void DesktopUI::render(uint32_t now) {
  if (!_spriteReady) return;
  clear();
  switch (_screen) {
    case Config::ScreenState::Dashboard: renderDashboard(now); break;
    case Config::ScreenState::Timetable: renderTimetable(); break;
    case Config::ScreenState::Pomodoro: renderPomodoro(); break;
    case Config::ScreenState::AsciiShowcase: renderAsciiShowcase(now); break;
    case Config::ScreenState::MediaDisplay: renderMedia(now); break;
    case Config::ScreenState::Games: renderGames(now); break;
    case Config::ScreenState::Settings: renderSettings(); break;
    case Config::ScreenState::Count: break;
  }
  _img.pushSprite(0, 0);
}

void DesktopUI::handleGlobal(uint32_t now, const ButtonEvents& events) {
  if (events.up || events.down || events.left || events.right || events.center) _lastInteractionMs = now;
  if (_gameMode != GameMode::Menu && _screen == Config::ScreenState::Games) {
    if (events.left || events.right) _gameMode = GameMode::Menu;
    return;
  }
  if (events.left) {
    int s = static_cast<int>(_screen) - 1;
    if (s < 0) s = static_cast<int>(Config::ScreenState::Count) - 1;
    _screen = static_cast<Config::ScreenState>(s);
  } else if (events.right) {
    int s = (static_cast<int>(_screen) + 1) % static_cast<int>(Config::ScreenState::Count);
    _screen = static_cast<Config::ScreenState>(s);
  }
}

void DesktopUI::handleScreen(uint32_t now, const ButtonEvents& events) {
  switch (_screen) {
    case Config::ScreenState::Timetable:
      if (events.up) _timetableDay = (_timetableDay + 6) % 7;
      if (events.down) _timetableDay = (_timetableDay + 1) % 7;
      break;
    case Config::ScreenState::Pomodoro:
      if (events.center) {
        _pomodoroRunning = !_pomodoroRunning;
        _lastPomoTick = now;
      }
      break;
    case Config::ScreenState::AsciiShowcase:
      if (events.up) _asciiMode = (_asciiMode + 2) % 3;
      if (events.down) _asciiMode = (_asciiMode + 1) % 3;
      break;
    case Config::ScreenState::MediaDisplay:
      if (events.up || events.down) _mediaIndex = (_mediaIndex + 1) % 2;
      if (events.center) _mediaPlaying = !_mediaPlaying;
      break;
    case Config::ScreenState::Games:
      if (_gameMode == GameMode::Menu) {
        if (events.up) _gameSelect = (_gameSelect + 2) % 3;
        if (events.down) _gameSelect = (_gameSelect + 1) % 3;
        if (events.center) {
          _gameMode = static_cast<GameMode>(_gameSelect + 1);
          resetAutoplay(now);
        }
      } else if (_gameMode == GameMode::Tamagotchi) {
        if (events.up) _petFood = clampByte(_petFood + 1);
        if (events.down) _petClean = clampByte(_petClean + 1);
        if (events.center) _petMood = clampByte(_petMood + 1);
      } else if (events.up || events.down || events.center) {
        resetAutoplay(now);
      }
      break;
    case Config::ScreenState::Settings:
      if (events.up) _settingsSelect = (_settingsSelect + 2) % 3;
      if (events.down) _settingsSelect = (_settingsSelect + 1) % 3;
      if (events.center) {
        if (_settingsSelect == 0) {
          _powerIndex = (_powerIndex + 1) % 3;
        } else if (_settingsSelect == 1) {
          _dimIndex = (_dimIndex + 1) % 4;
          const uint8_t duty[4] = {36, 74, 126, 220};
          ledcWrite(0, duty[_dimIndex]);
        } else {
          _colorIndex = (_colorIndex + 1) % 17;
        }
      }
      break;
    default:
      break;
  }
}

void DesktopUI::tickPomodoro(uint32_t now) {
  if (!_pomodoroRunning) return;
  if (_lastPomoTick == 0) _lastPomoTick = now;
  while (now - _lastPomoTick >= 1000) {
    _lastPomoTick += 1000;
    if (_pomoRemain > 0) {
      --_pomoRemain;
    }
    if (_pomoRemain == 0) {
      _pomoPhase = (_pomoPhase == PomodoroPhase::Focus) ? PomodoroPhase::Rest : PomodoroPhase::Focus;
      _pomoRemain = (_pomoPhase == PomodoroPhase::Focus) ? Config::POMODORO_FOCUS_SEC : Config::POMODORO_REST_SEC;
    }
  }
}

void DesktopUI::tickGames(uint32_t now) {
  static uint32_t lastDecay = 0;
  if (now - lastDecay > 7000) {
    lastDecay = now;
    _petFood = clampByte(_petFood - 1);
    _petMood = clampByte(_petMood - 1);
    _petClean = clampByte(_petClean - 1);
  }
  if ((_gameMode == GameMode::Chess || _gameMode == GameMode::Dungeon) && (now / 9000) != ((now - Config::UI_FRAME_MS) / 9000)) {
    resetAutoplay(now);
  }
}

void DesktopUI::resetAutoplay(uint32_t now) {
  _gameSeed = esp_random() ^ micros() ^ now;
  randomSeed(_gameSeed);
}

void DesktopUI::clear() {
  _img.fillSprite(bg());
  _img.setTextColor(fg(), bg());
  _img.setTextFont(1);
  _img.setTextSize(1);
}

uint16_t DesktopUI::fg() const {
  if (reverseTheme()) return TFT_WHITE;
  static const uint16_t colors[16] = {
      TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_ORANGE,
      TFT_PINK, TFT_PURPLE, TFT_NAVY, TFT_DARKGREEN, TFT_MAROON, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY};
  return colors[_colorIndex % 16];
}

uint16_t DesktopUI::bg() const { return reverseTheme() ? TFT_BLACK : TFT_BLACK; }

uint16_t DesktopUI::mid() const { return reverseTheme() ? TFT_DARKGREY : TFT_DARKGREY; }

bool DesktopUI::reverseTheme() const { return _colorIndex == 16; }

void DesktopUI::drawFrame(int16_t x, int16_t y, int16_t w, int16_t h) {
  _img.drawRect(x, y, w, h, fg());
}

void DesktopUI::drawTitle(int16_t x, int16_t y, int16_t w, const String& title) {
  int16_t tw = static_cast<int16_t>(title.length() * 6 + 8);
  int16_t lx2 = x + (w - tw) / 2 - 4;
  int16_t rx1 = x + (w + tw) / 2 + 4;
  int16_t leftLen = lx2 - x;
  int16_t rightLen = x + w - rx1;
  if (leftLen > 0) _img.drawFastHLine(x, y + 5, leftLen, fg());
  if (rightLen > 0) _img.drawFastHLine(rx1, y + 5, rightLen, fg());
  _img.setTextDatum(TC_DATUM);
  _img.drawString(title, x + w / 2, y);
  _img.setTextDatum(TL_DATUM);
}

void DesktopUI::drawFooter(const String& hint) {
  _img.drawFastHLine(0, 122, Config::SCREEN_W, fg());
  _img.setTextDatum(MC_DATUM);
  _img.drawString(hint, Config::SCREEN_W / 2, 129);
  _img.setTextDatum(TL_DATUM);
}

void DesktopUI::drawLabelBox(int16_t x, int16_t y, int16_t w, int16_t h, const String& label) {
  drawFrame(x, y, w, h);
  int16_t labelW = label.length() * 6 + 8;
  _img.fillRect(x + 6, y, labelW, 8, bg());
  _img.drawString(label, x + 10, y - 1);
}

void DesktopUI::drawTextFit(const String& text, int16_t x, int16_t y, int16_t maxChars) {
  String out = text;
  if (out.length() > static_cast<uint16_t>(maxChars)) out = out.substring(0, maxChars - 1) + "~";
  _img.drawString(out, x, y);
}

void DesktopUI::drawProgress(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t value) {
  drawFrame(x, y, w, h);
  int16_t fillW = map(value, 0, 10, 0, w - 4);
  _img.fillRect(x + 2, y + 2, fillW, h - 4, fg());
}

void DesktopUI::drawBigDigits(const String& text, int16_t x, int16_t y, uint8_t scale) {
  static const uint8_t segs[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  int16_t cx = x;
  int16_t unit = 3 * scale;
  int16_t len = 12 * scale;
  int16_t wid = 8 * scale;
  for (uint16_t i = 0; i < text.length(); ++i) {
    char c = text[i];
    if (c == ':') {
      _img.fillRect(cx + 2 * scale, y + 8 * scale, 2 * scale, 2 * scale, fg());
      _img.fillRect(cx + 2 * scale, y + 20 * scale, 2 * scale, 2 * scale, fg());
      cx += 7 * scale;
      continue;
    }
    if (c < '0' || c > '9') {
      cx += 6 * scale;
      continue;
    }
    uint8_t s = segs[c - '0'];
    auto h = [&](int16_t ox, int16_t oy) { _img.fillRect(cx + ox, y + oy, wid, unit, fg()); };
    auto v = [&](int16_t ox, int16_t oy) { _img.fillRect(cx + ox, y + oy, unit, len, fg()); };
    if (s & 0x01) h(2 * scale, 0);
    if (s & 0x02) v(10 * scale, 2 * scale);
    if (s & 0x04) v(10 * scale, 16 * scale);
    if (s & 0x08) h(2 * scale, 28 * scale);
    if (s & 0x10) v(0, 16 * scale);
    if (s & 0x20) v(0, 2 * scale);
    if (s & 0x40) h(2 * scale, 14 * scale);
    cx += 15 * scale;
  }
}

void DesktopUI::drawAsciiLines(const char* const* lines, uint8_t count, int16_t x, int16_t y, uint8_t gap) {
  for (uint8_t i = 0; i < count; ++i) _img.drawString(lines[i], x, y + i * gap);
}

void DesktopUI::drawGridCell(uint8_t col, uint8_t row, char ch) {
  if (col >= 40 || row >= 16) return;
  _img.drawChar(ch, col * 6, row * 8, fg(), bg(), 1);
}

void DesktopUI::drawBuiltInIcon(int16_t x, int16_t y) {
  for (uint8_t r = 0; r < 9; ++r) {
    const char* line = kBuiltInIcon[r];
    for (uint8_t c = 0; line[c] != '\0'; ++c) {
      if (line[c] == '|') {
        uint8_t h = 4 + ((c + r) % 5);
        _img.fillRect(x + c * 4, y + r * 8 - h / 2, 3, h + 5, fg());
      }
    }
  }
}

void DesktopUI::drawStatusBar(const String& title, uint32_t now, bool autoHide) {
  if (autoHide && now - _lastInteractionMs > Config::STATUS_AUTOHIDE_MS) return;
  drawTitle(3, 3, 234, title);
}

void DesktopUI::renderDashboard(uint32_t now) {
  drawTitle(4, 3, 232, "IDK-TS DESKTOP");
  drawLabelBox(4, 17, 112, 33, "CLOCK");
  drawBigDigits(_timeText, 12, 22, 1);
  _img.drawString(_dateText, 14, 43);

  drawLabelBox(4, 53, 112, 31, "WEATHER");
  _img.drawString("P. CAT LAI, THU DUC", 10, 64);
  drawTextFit(_weatherText, 14, 74, 17);

  drawLabelBox(4, 87, 112, 31, "NOTES");
  drawTextFit("- " + _notes[0], 10, 96, 17);
  drawTextFit("- " + _notes[1], 10, 106, 17);

  drawLabelBox(121, 17, 115, 101, "BUILT-IN ICON");
  drawBuiltInIcon(135, 34);
  drawTextFit(_wifiStatus, 128, 96, 17);
  drawTextFit("AP " + _apIp, 128, 106, 17);
  drawFooter("[L/R] SCREENS  [C] ACTION");
}

void DesktopUI::renderTimetable() {
  drawTitle(4, 3, 232, String("TIMETABLE - ") + kDayNames[_timetableDay]);
  for (uint8_t i = 0; i < 7; ++i) {
    int16_t x = 8 + i * 33;
    if (i == _timetableDay) {
      _img.drawRect(x - 2, 18, 29, 16, fg());
      _img.fillTriangle(x + 10, 36, x + 5, 43, x + 15, 43, fg());
    }
    _img.drawString(kDayNames[i], x, 22);
  }
  _img.drawFastVLine(28, 48, 62, fg());
  for (uint8_t i = 0; i < 7; ++i) {
    char num[3];
    snprintf(num, sizeof(num), "%u", i + 1);
    _img.drawString(num, 14, 49 + i * 9);
    String item = String(i + 1) + ": " + _timetable[_timetableDay][i];
    drawTextFit(item, 39, 49 + i * 9, 27);
  }
  drawFooter("[U/D] DAYS  [L/R] SCREENS");
}

void DesktopUI::renderPomodoro() {
  drawTitle(4, 3, 232, "POMODORO CLOCK");
  uint16_t minutes = _pomoRemain / 60;
  uint16_t seconds = _pomoRemain % 60;
  char timeBuf[6];
  snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u", minutes, seconds);
  drawBigDigits(timeBuf, 22, 25, 2);
  _img.setTextDatum(MC_DATUM);
  _img.drawString(_pomoPhase == PomodoroPhase::Focus ? "FOCUS 25" : "REST 15", 76, 96);
  _img.drawString(_pomodoroRunning ? "[PAUSE]" : "[PLAY]", 151, 96);
  _img.drawString("[C] START/PAUSE", 120, 113);
  _img.setTextDatum(TL_DATUM);
}

void DesktopUI::renderAsciiShowcase(uint32_t now) {
  const char* title = _asciiMode == 0 ? "ASCII SHOWCASE [SPACE]" : (_asciiMode == 1 ? "ASCII SHOWCASE [QUANTUM]" : "ASCII SHOWCASE [MATH]");
  drawStatusBar(title, now, true);
  drawFrame(4, 16, 232, 103);
  uint8_t frame = (now / 90) & 0xFF;
  for (uint8_t r = 2; r < 15; ++r) {
    for (uint8_t c = 0; c < 40; ++c) {
      bool on = false;
      char ch = '.';
      if (_asciiMode == 0) {
        int wave = 8 + static_cast<int>(5.0f * sinf((c + frame) * 0.24f));
        on = abs(static_cast<int>(r) - wave) < 1 || ((c * 7 + r * 13 + frame) % 97 == 0);
        ch = on && ((c + frame) % 11 == 0) ? '*' : '~';
      } else if (_asciiMode == 1) {
        int cx = 20, cy = 8;
        int dx = c - cx, dy = (r - cy) * 2;
        int ring = dx * dx + dy * dy;
        on = (ring + frame * 3) % 37 < 5 || (ring > 55 && ring < 70) || (ring > 145 && ring < 165);
        ch = ((c + r + frame) & 3) ? '.' : 'o';
      } else {
        int y = 8 + static_cast<int>(4.0f * sinf((c * 0.35f) + frame * 0.08f));
        on = r == y || ((c * c + r * 5 + frame) % 61 == 0);
        ch = (r == y) ? '#' : '+';
      }
      if (on) drawGridCell(c, r, ch);
    }
  }
  if (_asciiMode == 0) {
    _img.drawString("[1] SPACE", 168, 54);
    _img.drawString("[2] QUANTUM", 168, 66);
    _img.drawString("[3] MATH", 168, 78);
  }
  drawFooter("[U/D] THEME  [L/R] SCREENS");
}

void DesktopUI::renderMedia(uint32_t now) {
  drawStatusBar(_mediaIndex == 0 ? "MEDIA PLAYER [car.gif]" : "MEDIA PLAYER [wave.asc]", now, true);
  drawFrame(5, 17, 230, 88);
  if (_mediaIndex == 0) {
    int16_t x = 48 + ((now / 180) % 11);
    drawAsciiLines(kCarArt, 5, x, 47, 9);
  } else {
    drawAsciiLines(kWaveArt, 5, 58, 38, 11);
  }
  _img.setTextDatum(MC_DATUM);
  _img.drawString(_mediaPlaying ? "[PLAY]  [NEXT]  [LOOP]" : "[PAUSE] [NEXT]  [LOOP]", 120, 115);
  _img.setTextDatum(TL_DATUM);
}

void DesktopUI::renderGames(uint32_t now) {
  drawTitle(4, 3, 232, "EXTERNAL MODE - GAMES");
  if (_gameMode == GameMode::Menu) {
    const char* names[3] = {"Tamagotchi", "Auto Chess", "Dungeon RPG"};
    for (uint8_t i = 0; i < 3; ++i) {
      int16_t x = 6 + i * 78;
      drawLabelBox(x, 22, 74, 83, String(i + 1) + ". " + names[i]);
      if (i == _gameSelect) _img.drawRect(x + 2, 24, 70, 79, fg());
    }
    _img.drawString("/\\_/\\", 24, 48);
    _img.drawString("o.o", 30, 58);
    drawProgress(17, 80, 54, 8, _petFood);
    for (uint8_t r = 0; r < 6; ++r) {
      for (uint8_t c = 0; c < 6; ++c) {
        if ((r + c) & 1) _img.fillRect(95 + c * 7, 48 + r * 6, 7, 6, fg());
        else _img.drawRect(95 + c * 7, 48 + r * 6, 7, 6, fg());
      }
    }
    _img.drawString("AI", 109, 90);
    _img.drawString("+--+--+", 169, 50);
    _img.drawString("|##  @|", 169, 60);
    _img.drawString("|  ## |", 169, 70);
    _img.drawString("+--+--+", 169, 80);
  } else if (_gameMode == GameMode::Tamagotchi) {
    drawLabelBox(8, 23, 224, 90, "TAMAGOTCHI MANUAL");
    const char* const pet[] = {"    /\\_/\\", "   ( o.o )", "    > ^ <", "  feed clean play"};
    drawAsciiLines(pet, 4, 24, 42, 11);
    _img.drawString("FOOD", 125, 42); drawProgress(160, 41, 58, 8, _petFood);
    _img.drawString("MOOD", 125, 58); drawProgress(160, 57, 58, 8, _petMood);
    _img.drawString("WASH", 125, 74); drawProgress(160, 73, 58, 8, _petClean);
    _img.drawString("U=FOOD D=CLEAN C=PLAY", 58, 101);
  } else if (_gameMode == GameMode::Chess) {
    drawLabelBox(8, 23, 224, 90, "AUTO CHESS - AUTO PLAY");
    uint8_t offset = (_gameSeed + now / 800) % 8;
    for (uint8_t r = 0; r < 8; ++r) {
      for (uint8_t c = 0; c < 8; ++c) {
        if ((r + c) & 1) _img.fillRect(33 + c * 10, 35 + r * 8, 10, 8, fg());
      }
    }
    _img.setTextColor(bg(), fg());
    _img.drawChar(34 + offset * 10, 36 + ((now / 1200) % 6) * 8, 'K', bg(), fg(), 1);
    _img.setTextColor(fg(), bg());
    _img.drawString("seed", 137, 46);
    _img.drawNumber(_gameSeed & 0xFFFF, 137, 58);
    _img.drawString("new path on end", 119, 82);
  } else {
    drawLabelBox(8, 23, 224, 90, "AUTO DUNGEON RPG");
    uint8_t px = 0, py = 0;
    for (uint8_t y = 0; y < 5; ++y) {
      for (uint8_t x = 0; x < 9; ++x) {
        bool wall = ((x * 13 + y * 7 + _gameSeed) % 5) == 0;
        int16_t sx = 36 + x * 14, sy = 40 + y * 12;
        if (wall) _img.fillRect(sx, sy, 12, 10, fg());
        else _img.drawRect(sx, sy, 12, 10, fg());
      }
    }
    px = (now / 700 + _gameSeed) % 9;
    py = (now / 1400 + (_gameSeed >> 8)) % 5;
    _img.fillRect(39 + px * 14, 43 + py * 12, 6, 5, mid());
    _img.drawString("AI path reset", 129, 102);
  }
  drawFooter("[U/D] MENU  [C] ENTER  [L/R] BACK");
}

void DesktopUI::renderSettings() {
  drawTitle(4, 3, 232, "SETTINGS");
  drawLabelBox(8, 22, 224, 94, "OPTIONS");
  const char* rows[3] = {"Power:", "Dim Time:", "UI Color:"};
  const char* powers[3] = {"Restart", "Deep Sleep", "Turn Off"};
  for (uint8_t i = 0; i < 3; ++i) {
    int16_t y = 39 + i * 24;
    if (i == _settingsSelect) {
      _img.fillRect(16, y - 2, 208, 14, fg());
      _img.setTextColor(bg(), fg());
    }
    _img.drawString(rows[i], 22, y);
    if (i == 0) {
      _img.drawString(powers[_powerIndex], 88, y);
    } else if (i == 1) {
      const char* dims[4] = {"10s", "20s", "30s", "1m"};
      _img.drawString(dims[_dimIndex], 88, y);
    } else if (i == 2) {
      String colorName = _colorIndex == 16 ? String("Reverse") : String("Color ") + String(_colorIndex);
      _img.drawString(colorName, 88, y);
    }
    if (i == _settingsSelect) _img.setTextColor(fg(), bg());
  }
  _img.drawString("Power row cycles action label", 26, 103);
  drawFooter("[U/D] SELECT  [C] CHANGE");
}
