#include <M5Unified.h>
#include <vector>
#include <cctype>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

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

struct Fraction {
  long long num;
  long long den;
};

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

long long gcdll(long long a, long long b) {
  a = llabs(a);
  b = llabs(b);
  while (b != 0) {
    long long t = a % b;
    a = b;
    b = t;
  }
  return a == 0 ? 1 : a;
}

Fraction makeFrac(long long n, long long d = 1) {
  if (d < 0) { n = -n; d = -d; }
  long long g = gcdll(n, d);
  return {n / g, d / g};
}

Fraction addFrac(const Fraction& a, const Fraction& b) {
  return makeFrac(a.num * b.den + b.num * a.den, a.den * b.den);
}

Fraction subFrac(const Fraction& a, const Fraction& b) {
  return makeFrac(a.num * b.den - b.num * a.den, a.den * b.den);
}

Fraction mulFrac(const Fraction& a, const Fraction& b) {
  return makeFrac(a.num * b.num, a.den * b.den);
}

Fraction divFrac(const Fraction& a, const Fraction& b) {
  return makeFrac(a.num * b.den, a.den * b.num);
}

bool fracIsZero(const Fraction& f) { return f.num == 0; }

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

void splitSpecies(const String& side, std::vector<String>& out) {
  out.clear();
  String token = "";
  for (int i = 0; i <= side.length(); ++i) {
    char c = (i == side.length()) ? '+' : side[i];
    if (c == '+') {
      String t = token; t.trim();
      if (!t.isEmpty()) {
        int pos = 0;
        while (pos < t.length() && isdigit(static_cast<unsigned char>(t[pos]))) pos++;
        String formula = t.substring(pos);
        formula.trim();
        if (!formula.isEmpty()) out.push_back(formula);
      }
      token = "";
    } else {
      token += c;
    }
  }
}

bool splitEquation(const String& eq, String& left, String& right) {
  int arrow = eq.indexOf("->");
  if (arrow >= 0) {
    left = eq.substring(0, arrow);
    right = eq.substring(arrow + 2);
    return true;
  }
  int eqpos = eq.indexOf('=');
  if (eqpos >= 0) {
    left = eq.substring(0, eqpos);
    right = eq.substring(eqpos + 1);
    return true;
  }
  return false;
}

bool balanceEquation(const String& eq, String& out) {
  String left, right;
  if (!splitEquation(eq, left, right)) return false;
  std::vector<String> reactants;
  std::vector<String> products;
  splitSpecies(left, reactants);
  splitSpecies(right, products);
  if (reactants.empty() || products.empty()) return false;

  std::vector<String> species = reactants;
  species.insert(species.end(), products.begin(), products.end());
  int n = static_cast<int>(species.size());
  if (n < 2) return false;

  std::vector<String> elements;
  std::vector<std::vector<int>> counts;
  for (const auto& sp : species) {
    std::vector<ElemCount> list;
    String err;
    if (!parseFormula(sp, list, err)) return false;
    counts.push_back(std::vector<int>());
    for (const auto& ec : list) {
      int idx = -1;
      for (int i = 0; i < static_cast<int>(elements.size()); ++i) {
        if (elements[i] == ec.symbol) { idx = i; break; }
      }
      if (idx < 0) {
        idx = static_cast<int>(elements.size());
        elements.push_back(ec.symbol);
      }
    }
  }

  int m = static_cast<int>(elements.size());
  std::vector<std::vector<int>> mat(m, std::vector<int>(n, 0));
  for (int s = 0; s < n; ++s) {
    std::vector<ElemCount> list;
    String err;
    if (!parseFormula(species[s], list, err)) return false;
    for (const auto& ec : list) {
      int idx = 0;
      for (int i = 0; i < m; ++i) {
        if (elements[i] == ec.symbol) { idx = i; break; }
      }
      int sign = (s < static_cast<int>(reactants.size())) ? 1 : -1;
      mat[idx][s] += sign * ec.count;
    }
  }

  int cols = n - 1;
  std::vector<std::vector<Fraction>> a(m, std::vector<Fraction>(cols + 1, makeFrac(0)));
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < cols; ++j) {
      a[i][j] = makeFrac(mat[i][j]);
    }
    a[i][cols] = makeFrac(-mat[i][n - 1]);
  }

  int row = 0;
  for (int col = 0; col < cols && row < m; ++col) {
    int pivot = -1;
    for (int r = row; r < m; ++r) {
      if (!fracIsZero(a[r][col])) { pivot = r; break; }
    }
    if (pivot < 0) continue;
    if (pivot != row) std::swap(a[pivot], a[row]);

    Fraction piv = a[row][col];
    for (int c = col; c <= cols; ++c) a[row][c] = divFrac(a[row][c], piv);

    for (int r = 0; r < m; ++r) {
      if (r == row) continue;
      if (fracIsZero(a[r][col])) continue;
      Fraction factor = a[r][col];
      for (int c = col; c <= cols; ++c) {
        a[r][c] = subFrac(a[r][c], mulFrac(factor, a[row][c]));
      }
    }
    row++;
  }

  std::vector<Fraction> sol(cols, makeFrac(0));
  for (int r = 0; r < m; ++r) {
    int lead = -1;
    for (int c = 0; c < cols; ++c) {
      if (!fracIsZero(a[r][c])) { lead = c; break; }
    }
    if (lead >= 0) sol[lead] = a[r][cols];
  }

  std::vector<Fraction> coeffs(n);
  for (int i = 0; i < cols; ++i) coeffs[i] = sol[i];
  coeffs[n - 1] = makeFrac(1);

  long long lcm = 1;
  for (const auto& f : coeffs) {
    lcm = lcm / gcdll(lcm, f.den) * f.den;
  }
  std::vector<long long> ints(n);
  for (int i = 0; i < n; ++i) {
    ints[i] = coeffs[i].num * (lcm / coeffs[i].den);
  }

  long long g = 0;
  for (long long v : ints) g = gcdll(g, v);
  if (g == 0) return false;
  for (long long& v : ints) v /= g;

  bool all_neg = true;
  for (long long v : ints) if (v > 0) all_neg = false;
  if (all_neg) {
    for (long long& v : ints) v = -v;
  }

  for (long long v : ints) if (v <= 0) return false;

  String result = "";
  for (int i = 0; i < static_cast<int>(reactants.size()); ++i) {
    if (i > 0) result += " + ";
    if (ints[i] != 1) result += String(ints[i]);
    result += reactants[i];
  }
  result += " -> ";
  for (int i = 0; i < static_cast<int>(products.size()); ++i) {
    int idx = i + reactants.size();
    if (i > 0) result += " + ";
    if (ints[idx] != 1) result += String(ints[idx]);
    result += products[i];
  }
  out = result;
  return true;
}

void actionBalance() {
  String eq = "";
  if (!keyboardInput(eq, "Eq: H2+O2->H2O", false, false, g_telex_mode, 120, true)) return;
  eq.trim();
  if (eq.isEmpty()) return;
  String result;
  if (!balanceEquation(eq, result)) {
    showResult("Balance", "Cannot balance");
    return;
  }
  showResult("Balance", result);
}

MenuItem kMenu[] = {
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
  M5.Display.print("idk-chemistry-reaction");

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

  (void)loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
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
