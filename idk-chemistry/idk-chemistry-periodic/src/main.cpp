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

void actionLookup() {
  String query = "";
  if (!keyboardInput(query, "Element (H/8)", false, true, g_telex_mode, 16, false)) return;
  query.trim();
  if (query.isEmpty()) return;

  const ElementInfo* info = nullptr;
  bool allDigits = true;
  for (int i = 0; i < query.length(); ++i) {
    if (!isdigit(static_cast<unsigned char>(query[i]))) { allDigits = false; break; }
  }
  if (allDigits) {
    info = findElementByNumber(query.toInt());
  } else {
    String sym = "";
    sym += static_cast<char>(toupper(static_cast<unsigned char>(query[0])));
    if (query.length() > 1) sym += static_cast<char>(tolower(static_cast<unsigned char>(query[1])));
    info = findElementBySymbol(sym);
  }

  if (!info) { showResult("Element", "Not found"); return; }
  String out = "";
  out += info->symbol; out += " - "; out += info->name;
  out += "\nZ = "; out += String(info->atomic_number);
  out += "\nMass = "; out += formatFloat(info->atomic_mass, 4);
  if (strlen(info->oxidation_states) > 0) { out += "\nOx = "; out += info->oxidation_states; }
  showResult("Element", out);
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
    {"Lookup element", actionLookup},
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
