#include <M5Unified.h>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct Question {
  const char* q;
  const char* a;
  const char* b;
  const char* c;
  int correct;
};

const Question kQuestions[] = {
    {"Avogadro number", "6.02e23", "3.14", "9.8", 0},
    {"pH of 1e-7", "7", "1", "14", 0},
    {"Mol = ?", "m/MM", "MM/m", "m*MM", 0},
    {"Ideal gas", "PV=nRT", "F=ma", "E=mc2", 0},
    {"Oxidation of O2", "-2", "0", "+2", 1},
};
constexpr int kQuestionCount = sizeof(kQuestions) / sizeof(kQuestions[0]);

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

void drawQuestion(const Question& q, int index) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("Quiz %d/%d", index + 1, kQuestionCount);
  drawWrapped(String(q.q), 2, 16, 236, 2);

  M5.Display.setCursor(2, 54);
  M5.Display.printf("A: %s", q.a);
  M5.Display.setCursor(2, 70);
  M5.Display.printf("B: %s", q.b);
  M5.Display.setCursor(2, 86);
  M5.Display.printf("C: %s", q.c);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print("A/B/PWR chọn - hold A thoát");
}

void runQuiz() {
  int score = 0;
  for (int i = 0; i < kQuestionCount; ++i) {
    const Question& q = kQuestions[i];
    drawQuestion(q, i);
    while (true) {
      M5.update();
      if (M5.BtnA.pressedFor(700)) return;
      int choice = -1;
      if (M5.BtnA.wasPressed()) choice = 0;
      if (M5.BtnB.wasPressed()) choice = 1;
      if (M5.BtnPWR.wasPressed()) choice = 2;
      if (choice >= 0) {
        bool ok = (choice == q.correct);
        if (ok) score++;
        showResult("Result", ok ? "Correct" : "Wrong");
        break;
      }
      delay(10);
    }
  }
  showResult("Score", String(score) + "/" + String(kQuestionCount));
}

MenuItem kMenu[] = {
    {"Start quiz", runQuiz},
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
  M5.Display.print("idk-exam-practice (chemistry)");

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
