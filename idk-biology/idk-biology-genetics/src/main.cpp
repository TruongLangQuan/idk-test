#include <M5Unified.h>
#include <vector>
#include <map>
#include <cctype>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
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

String normalizeGenotype(const String& in) {
  String out = "";
  for (int i = 0; i < in.length(); ++i) {
    char c = in[i];
    if (isalpha(static_cast<unsigned char>(c))) out += c;
  }
  return out;
}

std::vector<String> makeGametes(const String& genotype) {
  std::vector<String> gametes;
  if (genotype.length() % 2 != 0) return gametes;
  gametes.push_back("");
  for (int i = 0; i < genotype.length(); i += 2) {
    char a = genotype[i];
    char b = genotype[i + 1];
    std::vector<char> alleles;
    alleles.push_back(a);
    if (toupper(static_cast<unsigned char>(a)) != toupper(static_cast<unsigned char>(b)) || a != b) {
      if (b != a) alleles.push_back(b);
    }
    std::vector<String> next;
    for (const auto& g : gametes) {
      for (char al : alleles) {
        next.push_back(g + String(al));
      }
    }
    gametes.swap(next);
  }
  return gametes;
}

String combineGenotype(const String& g1, const String& g2) {
  String out = "";
  int genes = min(g1.length(), g2.length());
  for (int i = 0; i < genes; ++i) {
    char a = g1[i];
    char b = g2[i];
    char up = isupper(static_cast<unsigned char>(a)) ? a : (isupper(static_cast<unsigned char>(b)) ? b
                                                                                                     : static_cast<char>(toupper(a)));
    char low = islower(static_cast<unsigned char>(a)) ? a : (islower(static_cast<unsigned char>(b)) ? b
                                                                                                     : static_cast<char>(tolower(a)));
    out += up;
    out += low;
  }
  return out;
}

String phenotypeFor(const String& genotype) {
  String out = "";
  for (int i = 0; i < genotype.length(); i += 2) {
    char a = genotype[i];
    char b = (i + 1 < genotype.length()) ? genotype[i + 1] : genotype[i];
    if (isupper(static_cast<unsigned char>(a)) || isupper(static_cast<unsigned char>(b))) {
      out += static_cast<char>(toupper(a));
      out += '-';
    } else {
      out += static_cast<char>(tolower(a));
      out += static_cast<char>(tolower(a));
    }
  }
  return out;
}

void runCross(const String& p1, const String& p2, const char* title) {
  String g1 = normalizeGenotype(p1);
  String g2 = normalizeGenotype(p2);
  if (g1.length() != g2.length() || g1.length() == 0 || g1.length() % 2 != 0) {
    showResult(title, "Invalid genotype");
    return;
  }
  auto gam1 = makeGametes(g1);
  auto gam2 = makeGametes(g2);
  if (gam1.empty() || gam2.empty()) {
    showResult(title, "Invalid gametes");
    return;
  }

  std::map<String, int> genoCounts;
  std::map<String, int> phenoCounts;
  int total = 0;
  for (const auto& a : gam1) {
    for (const auto& b : gam2) {
      String child = combineGenotype(a, b);
      genoCounts[child]++;
      phenoCounts[phenotypeFor(child)]++;
      total++;
    }
  }

  String out = "Genotypes:\n";
  for (const auto& kv : genoCounts) {
    out += kv.first + ": " + String(kv.second) + "\n";
  }
  out += "\nPhenotypes:\n";
  for (const auto& kv : phenoCounts) {
    out += kv.first + ": " + String(kv.second) + "\n";
  }
  out += "Total = " + String(total);
  showResult(title, out);
}

void actionMonohybrid() {
  String p1 = "";
  String p2 = "";
  if (!keyboardInput(p1, "P1 (Aa)", false, false, g_telex_mode, 10, true)) return;
  if (!keyboardInput(p2, "P2 (Aa)", false, false, g_telex_mode, 10, true)) return;
  runCross(p1, p2, "Monohybrid");
}

void actionDihybrid() {
  String p1 = "";
  String p2 = "";
  if (!keyboardInput(p1, "P1 (AaBb)", false, false, g_telex_mode, 10, true)) return;
  if (!keyboardInput(p2, "P2 (AaBb)", false, false, g_telex_mode, 10, true)) return;
  runCross(p1, p2, "Dihybrid");
}

void actionHardy() {
  double p = 0;
  if (!inputNumber(p, "p (allele)", g_telex_mode)) return;
  if (p < 0 || p > 1) {
    showResult("Hardy", "p in [0,1]");
    return;
  }
  double q = 1.0 - p;
  String out = "p^2 = " + formatFloat(p * p, 4) + "\n";
  out += "2pq = " + formatFloat(2 * p * q, 4) + "\n";
  out += "q^2 = " + formatFloat(q * q, 4);
  showResult("Hardy-Weinberg", out);
}

MenuItem kMenu[] = {
    {"Monohybrid cross", actionMonohybrid},
    {"Dihybrid cross", actionDihybrid},
    {"Hardy-Weinberg", actionHardy},
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
  M5.Display.print("idk-biology-genetics");

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
