#include <M5Unified.h>
#include <cmath>
#include <cctype>
#include <vector>

#include <idk_ui.h>
#include "periodic_data.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct ElemCount {
  String symbol;
  int count;
};

bool g_telex_mode = false;
String g_status = "Ready";


constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;

const ElementInfo* findElementBySymbol(const String& sym) {
  const size_t count = sizeof(kElements) / sizeof(kElements[0]);
  for (size_t i = 0; i < count; ++i) {
    if (sym == kElements[i].symbol) return &kElements[i];
  }
  return nullptr;
}

const ElementInfo* findElementByNumber(int number) {
  const size_t count = sizeof(kElements) / sizeof(kElements[0]);
  for (size_t i = 0; i < count; ++i) {
    if (kElements[i].atomic_number == number) return &kElements[i];
  }
  return nullptr;
}

void addCount(std::vector<ElemCount>& out, const String& sym, int count) {
  if (count == 0) return;
  for (auto& ec : out) {
    if (ec.symbol == sym) { ec.count += count; return; }
  }
  out.push_back({sym, count});
}

int parseNumber(const String& s, int& pos) {
  int value = 0;
  int start = pos;
  while (pos < s.length() && isdigit(static_cast<unsigned char>(s[pos]))) {
    value = value * 10 + (s[pos] - '0');
    pos++;
  }
  if (pos == start) return 1;
  return value;
}

bool parseFormulaGroup(const String& s, int& pos, std::vector<ElemCount>& out, String& err) {
  while (pos < s.length()) {
    char c = s[pos];
    if (c == ')') return true;
    if (c == '(') {
      pos++;
      std::vector<ElemCount> inner;
      if (!parseFormulaGroup(s, pos, inner, err)) return false;
      if (pos >= s.length() || s[pos] != ')') { err = "Missing )"; return false; }
      pos++;
      int mult = parseNumber(s, pos);
      for (const auto& ec : inner) addCount(out, ec.symbol, ec.count * mult);
      continue;
    }
    if (c == '.' || c == ' ' || c == '\t') { pos++; continue; }
    if (isdigit(static_cast<unsigned char>(c))) { (void)parseNumber(s, pos); continue; }
    if (isupper(static_cast<unsigned char>(c))) {
      String sym = ""; sym += c;
      if (pos + 1 < s.length() && islower(static_cast<unsigned char>(s[pos + 1]))) {
        sym += s[pos + 1]; pos += 2;
      } else {
        pos += 1;
      }
      int mult = parseNumber(s, pos);
      addCount(out, sym, mult);
      continue;
    }
    err = "Bad char"; return false;
  }
  return true;
}

bool parseFormula(const String& input, std::vector<ElemCount>& out, String& err) {
  out.clear();
  String s = input; s.replace(" ", "");
  int pos = 0;
  if (!parseFormulaGroup(s, pos, out, err)) return false;
  if (pos < s.length() && s[pos] != ')') { err = "Extra chars"; return false; }
  return !out.empty();
}

void showElementInfo(const ElementInfo* info) {
  if (!info) return;
  String out = "";
  out += info->symbol; out += " - "; out += info->name;
  out += "\nZ = "; out += String(info->atomic_number);
  out += "\nMass = "; out += formatFloat(info->atomic_mass, 4);
  if (strlen(info->oxidation_states) > 0) { out += "\nOx = "; out += info->oxidation_states; }
  showResult("Element", out);
}

void actionLookup() {
  const size_t total = sizeof(kElements) / sizeof(kElements[0]);
  constexpr int kCols = 6;
  constexpr int kRows = 4;
  constexpr int kPerPage = kCols * kRows;
  const int totalPages = (total + kPerPage - 1) / kPerPage;
  int page = 0;
  int sel = 0;

  auto clampSelection = [&](int& p, int& s) {
    if (p < 0) p = 0;
    if (p >= totalPages) p = totalPages - 1;
    int pageStart = p * kPerPage;
    int pageCount = static_cast<int>(total - pageStart);
    if (pageCount > kPerPage) pageCount = kPerPage;
    if (pageCount <= 0) { s = 0; return; }
    if (s < 0) s = 0;
    if (s >= pageCount) s = pageCount - 1;
  };

  auto draw = [&]() {
    clampSelection(page, sel);
    const int screenW = M5.Display.width();
    const int screenH = M5.Display.height();
    const int titleY = 2;
    const int gridY = 16;
    const int footerY = screenH - 11;
    const int gridH = footerY - gridY - 2;
    const int cellW = screenW / kCols;
    const int cellH = gridH / kRows;

    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(2, titleY);
    M5.Display.print("Periodic Table");

    String pageInfo = String(page + 1) + "/" + String(totalPages);
    int pw = M5.Display.textWidth(pageInfo.c_str());
    M5.Display.setCursor(screenW - pw - 2, titleY);
    M5.Display.print(pageInfo);

    const int pageStart = page * kPerPage;
    for (int i = 0; i < kPerPage; ++i) {
      int idx = pageStart + i;
      if (idx >= static_cast<int>(total)) break;
      int row = i / kCols;
      int col = i % kCols;
      const int x = col * cellW;
      const int y = gridY + row * cellH;
      const bool selected = (i == sel);
      uint16_t bg = selected ? TFT_WHITE : TFT_BLACK;
      uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;
      M5.Display.fillRect(x, y, cellW, cellH, bg);
      M5.Display.drawRect(x, y, cellW, cellH, TFT_DARKGREY);

      const char* label = kElements[idx].symbol;
      int tw = M5.Display.textWidth(label);
      int th = M5.Display.fontHeight();
      int tx = x + (cellW - tw) / 2;
      int ty = y + (cellH - th) / 2;
      M5.Display.setTextColor(fg, bg);
      M5.Display.setCursor(tx, ty);
      M5.Display.print(label);
    }

    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.setCursor(2, footerY);
    M5.Display.print("A:Select  B:Next  PWR:Prev");
    M5.Display.setCursor(2, footerY + 9);
    M5.Display.print("Hold A:Back");
  };

  draw();
  bool longAHandled = false;
  while (true) {
    M5.update();

    if (M5.BtnB.wasPressed()) {
      int pageStart = page * kPerPage;
      int pageCount = static_cast<int>(total - pageStart);
      if (pageCount > kPerPage) pageCount = kPerPage;
      sel++;
      if (sel >= pageCount) {
        page = (page + 1) % totalPages;
        sel = 0;
      }
      draw();
    }
    if (M5.BtnPWR.wasPressed()) {
      sel--;
      if (sel < 0) {
        page = (page - 1 + totalPages) % totalPages;
        int pageStart = page * kPerPage;
        int pageCount = static_cast<int>(total - pageStart);
        if (pageCount > kPerPage) pageCount = kPerPage;
        sel = pageCount > 0 ? pageCount - 1 : 0;
      }
      draw();
    }

    if (M5.BtnA.pressedFor(600) && !longAHandled) {
      longAHandled = true;
      return;
    }
    if (M5.BtnA.wasReleased()) {
      longAHandled = false;
    }
    if (M5.BtnA.wasPressed() && !longAHandled) {
      int idx = page * kPerPage + sel;
      if (idx >= 0 && idx < static_cast<int>(total)) {
        showElementInfo(&kElements[idx]);
      }
      draw();
    }
    delay(8);
  }
}

void actionMolarMass() {
  String formula = "";
  if (!keyboardInput(formula, "Formula", false, true, g_telex_mode, 80, true)) return;
  formula.trim();
  if (formula.isEmpty()) return;
  std::vector<ElemCount> counts;
  String err;
  if (!parseFormula(formula, counts, err)) { showResult("Molar", "Bad formula"); return; }
  double total = 0.0;
  for (const auto& ec : counts) {
    const ElementInfo* info = findElementBySymbol(ec.symbol);
    if (!info) { showResult("Molar", "Unknown element"); return; }
    total += info->atomic_mass * ec.count;
  }
  showResult("Molar", "M = " + formatFloat(total, 4) + " g/mol");
}

MenuItem kMenu[] = {
    {"Periodic table", actionLookup},
    {"Molar mass", actionMolarMass},
};
constexpr int kMenuCount = sizeof(kMenu) / sizeof(kMenu[0]);

void ensureScroll() {
  if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
  if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
}

void drawMenu() {
  ensureScroll();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-chemistry-periodic");

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 14);
  M5.Display.print(g_status);

  int y = 28;
  for (int i = 0; i < kMenuLines; ++i) {
    int idx = g_menu_scroll + i;
    if (idx >= kMenuCount) break;
    bool sel = (idx == g_menu_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    M5.Display.print(kMenu[idx].title);
    y += 16;
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print("A:Select  B:Next  PWR:Prev");
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status);
  drawMenu();
}

void loop() {
  M5.update();
  if (M5.BtnB.wasPressed()) {
    g_menu_idx = (g_menu_idx + 1) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnPWR.wasPressed()) {
    g_menu_idx = (g_menu_idx - 1 + kMenuCount) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnA.wasPressed()) {
    kMenu[g_menu_idx].action();
    drawMenu();
  }
  delay(10);
}
