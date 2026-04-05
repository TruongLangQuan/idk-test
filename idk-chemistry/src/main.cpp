#include <M5Unified.h>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <vector>

#include <idk_ui.h>
#include "generated/vi12_font.h"
#include "periodic_data.h"

namespace {

constexpr int kMenuLines = 6;
constexpr int kMaxCompounds = 6;
constexpr int kMaxCoeff = 8;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct ElemCount {
  String symbol;
  int count;
};

int gcdInt(int a, int b);
bool dfsBalance(int idx, int n, int m, const std::vector<std::vector<int>>& mat,
                std::vector<int>& coeffs, bool& found);

bool g_font_loaded = false;
bool g_telex_mode = false;
String g_status = "Ready";

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

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
    if (ec.symbol == sym) {
      ec.count += count;
      return;
    }
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
    if (c == ')') {
      return true;
    }
    if (c == '(') {
      pos++;
      std::vector<ElemCount> inner;
      if (!parseFormulaGroup(s, pos, inner, err)) return false;
      if (pos >= s.length() || s[pos] != ')') {
        err = "Missing )";
        return false;
      }
      pos++;
      int mult = parseNumber(s, pos);
      for (const auto& ec : inner) {
        addCount(out, ec.symbol, ec.count * mult);
      }
      continue;
    }
    if (c == '.' || c == ' ' || c == '\t') {
      pos++;
      continue;
    }
    if (isdigit(static_cast<unsigned char>(c))) {
      // Leading coefficient, ignore.
      (void)parseNumber(s, pos);
      continue;
    }
    if (isupper(static_cast<unsigned char>(c))) {
      String sym = "";
      sym += c;
      if (pos + 1 < s.length() && islower(static_cast<unsigned char>(s[pos + 1]))) {
        sym += s[pos + 1];
        pos += 2;
      } else {
        pos += 1;
      }
      int mult = parseNumber(s, pos);
      addCount(out, sym, mult);
      continue;
    }
    err = "Bad char";
    return false;
  }
  return true;
}

bool parseFormula(const String& input, std::vector<ElemCount>& out, String& err) {
  out.clear();
  String s = input;
  s.replace(" ", "");
  int pos = 0;
  if (!parseFormulaGroup(s, pos, out, err)) return false;
  if (pos < s.length() && s[pos] != ')') {
    err = "Extra chars";
    return false;
  }
  return !out.empty();
}

void splitCompounds(const String& input, std::vector<String>& out) {
  out.clear();
  int start = 0;
  for (int i = 0; i <= input.length(); ++i) {
    if (i == input.length() || input[i] == '+') {
      String part = input.substring(start, i);
      part.trim();
      if (!part.isEmpty()) out.push_back(part);
      start = i + 1;
    }
  }
}

bool balanceEquation(const std::vector<String>& left, const std::vector<String>& right,
                     std::vector<int>& coeffs, String& err) {
  const int n = left.size() + right.size();
  if (n <= 0 || n > kMaxCompounds) {
    err = "Too many compounds";
    return false;
  }

  std::vector<std::vector<ElemCount>> comps;
  comps.reserve(n);
  std::vector<String> elements;

  auto addElement = [&](const String& sym) {
    for (const auto& s : elements) {
      if (s == sym) return;
    }
    elements.push_back(sym);
  };

  String perr;
  for (const auto& s : left) {
    std::vector<ElemCount> ec;
    if (!parseFormula(s, ec, perr)) {
      err = "Bad formula: " + s;
      return false;
    }
    for (const auto& e : ec) addElement(e.symbol);
    comps.push_back(ec);
  }
  for (const auto& s : right) {
    std::vector<ElemCount> ec;
    if (!parseFormula(s, ec, perr)) {
      err = "Bad formula: " + s;
      return false;
    }
    for (const auto& e : ec) addElement(e.symbol);
    comps.push_back(ec);
  }

  const int m = elements.size();
  std::vector<std::vector<int>> mat(n, std::vector<int>(m, 0));
  for (int i = 0; i < n; ++i) {
    for (const auto& ec : comps[i]) {
      for (int e = 0; e < m; ++e) {
        if (elements[e] == ec.symbol) {
          mat[i][e] = ec.count * (i < static_cast<int>(left.size()) ? 1 : -1);
          break;
        }
      }
    }
  }

  coeffs.assign(n, 1);
  bool found = false;
  dfsBalance(0, n, m, mat, coeffs, found);

  if (!found) {
    err = "No balance (limit)";
    return false;
  }

  int g = 0;
  for (int c : coeffs) {
    g = gcdInt(g, c);
  }
  if (g > 1) {
    for (int& c : coeffs) c /= g;
  }
  return true;
}

String buildEquation(const std::vector<String>& left, const std::vector<String>& right,
                     const std::vector<int>& coeffs) {
  String out = "";
  int idx = 0;
  for (size_t i = 0; i < left.size(); ++i) {
    if (i > 0) out += " + ";
    int c = coeffs[idx++];
    if (c > 1) {
      out += String(c) + " ";
    }
    out += left[i];
  }
  out += " -> ";
  for (size_t i = 0; i < right.size(); ++i) {
    if (i > 0) out += " + ";
    int c = coeffs[idx++];
    if (c > 1) {
      out += String(c) + " ";
    }
    out += right[i];
  }
  return out;
}

void actionPeriodic() {
  String query = "";
  if (!keyboardInput(query, "Element (H/8)", false, true, g_telex_mode, 16)) return;
  query.trim();
  if (query.isEmpty()) return;

  const ElementInfo* info = nullptr;
  bool allDigits = true;
  for (int i = 0; i < query.length(); ++i) {
    if (!isdigit(static_cast<unsigned char>(query[i]))) {
      allDigits = false;
      break;
    }
  }
  if (allDigits) {
    int z = query.toInt();
    info = findElementByNumber(z);
  } else {
    String sym = "";
    sym += static_cast<char>(toupper(static_cast<unsigned char>(query[0])));
    if (query.length() > 1) {
      sym += static_cast<char>(tolower(static_cast<unsigned char>(query[1])));
    }
    info = findElementBySymbol(sym);
  }

  if (!info) {
    showResult("Element", "Not found");
    return;
  }

  String out = "";
  out += info->symbol;
  out += " - ";
  out += info->name;
  out += "\nZ = ";
  out += String(info->atomic_number);
  out += "\nMass = ";
  out += formatFloat(info->atomic_mass, 4);
  if (strlen(info->oxidation_states) > 0) {
    out += "\nOx = ";
    out += info->oxidation_states;
  }
  showResult("Element", out);
}

void actionMolarMass() {
  String formula = "";
  if (!keyboardInput(formula, "Formula", false, true, g_telex_mode, 80)) return;
  formula.trim();
  if (formula.isEmpty()) return;
  std::vector<ElemCount> counts;
  String err;
  if (!parseFormula(formula, counts, err)) {
    showResult("Molar", "Bad formula");
    return;
  }
  double total = 0.0;
  for (const auto& ec : counts) {
    const ElementInfo* info = findElementBySymbol(ec.symbol);
    if (!info) {
      showResult("Molar", "Unknown element");
      return;
    }
    total += info->atomic_mass * ec.count;
  }
  String out = "M = " + formatFloat(total, 4) + " g/mol";
  showResult("Molar", out);
}

void actionBalance() {
  String eq = "";
  if (!keyboardInput(eq, "Equation", false, true, g_telex_mode, 120)) return;
  eq.trim();
  if (eq.isEmpty()) return;
  int arrow = eq.indexOf("->");
  int arrowLen = 2;
  if (arrow < 0) {
    arrow = eq.indexOf("=>");
    arrowLen = 2;
  }
  if (arrow < 0) {
    arrow = eq.indexOf("=");
    arrowLen = 1;
  }
  if (arrow < 0) {
    showResult("Balance", "Use -> or =");
    return;
  }
  String leftStr = eq.substring(0, arrow);
  String rightStr = eq.substring(arrow + arrowLen);
  std::vector<String> left, right;
  splitCompounds(leftStr, left);
  splitCompounds(rightStr, right);
  if (left.empty() || right.empty()) {
    showResult("Balance", "Missing side");
    return;
  }

  std::vector<int> coeffs;
  String err;
  if (!balanceEquation(left, right, coeffs, err)) {
    showResult("Balance", err);
    return;
  }
  String out = buildEquation(left, right, coeffs);
  showResult("Balance", out);
}

MenuItem kMenu[] = {
    {"Periodic table", actionPeriodic},
    {"Molar mass", actionMolarMass},
    {"Balance equation", actionBalance},
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
  M5.Display.print("idk-chemistry");

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

int gcdInt(int a, int b) {
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  while (b != 0) {
    int t = a % b;
    a = b;
    b = t;
  }
  return a;
}

bool dfsBalance(int idx, int n, int m, const std::vector<std::vector<int>>& mat,
                std::vector<int>& coeffs, bool& found) {
  if (found) return true;
  if (idx == n) {
    for (int e = 0; e < m; ++e) {
      int sum = 0;
      for (int i = 0; i < n; ++i) sum += mat[i][e] * coeffs[i];
      if (sum != 0) return false;
    }
    found = true;
    return true;
  }
  for (int c = 1; c <= kMaxCoeff; ++c) {
    coeffs[idx] = c;
    if (dfsBalance(idx + 1, n, m, mat, coeffs, found)) return true;
  }
  return false;
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  g_font_loaded = loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);

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
