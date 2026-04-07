#include <M5Unified.h>
#include <cctype>
#include <vector>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct CodonMap {
  const char* codon;
  const char* aa;
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

const CodonMap kCodons[] = {
    {"UUU", "Phe"}, {"UUC", "Phe"}, {"UUA", "Leu"}, {"UUG", "Leu"},
    {"UCU", "Ser"}, {"UCC", "Ser"}, {"UCA", "Ser"}, {"UCG", "Ser"},
    {"UAU", "Tyr"}, {"UAC", "Tyr"}, {"UAA", "Stop"}, {"UAG", "Stop"},
    {"UGU", "Cys"}, {"UGC", "Cys"}, {"UGA", "Stop"}, {"UGG", "Trp"},
    {"CUU", "Leu"}, {"CUC", "Leu"}, {"CUA", "Leu"}, {"CUG", "Leu"},
    {"CCU", "Pro"}, {"CCC", "Pro"}, {"CCA", "Pro"}, {"CCG", "Pro"},
    {"CAU", "His"}, {"CAC", "His"}, {"CAA", "Gln"}, {"CAG", "Gln"},
    {"CGU", "Arg"}, {"CGC", "Arg"}, {"CGA", "Arg"}, {"CGG", "Arg"},
    {"AUU", "Ile"}, {"AUC", "Ile"}, {"AUA", "Ile"}, {"AUG", "Met"},
    {"ACU", "Thr"}, {"ACC", "Thr"}, {"ACA", "Thr"}, {"ACG", "Thr"},
    {"AAU", "Asn"}, {"AAC", "Asn"}, {"AAA", "Lys"}, {"AAG", "Lys"},
    {"AGU", "Ser"}, {"AGC", "Ser"}, {"AGA", "Arg"}, {"AGG", "Arg"},
    {"GUU", "Val"}, {"GUC", "Val"}, {"GUA", "Val"}, {"GUG", "Val"},
    {"GCU", "Ala"}, {"GCC", "Ala"}, {"GCA", "Ala"}, {"GCG", "Ala"},
    {"GAU", "Asp"}, {"GAC", "Asp"}, {"GAA", "Glu"}, {"GAG", "Glu"},
    {"GGU", "Gly"}, {"GGC", "Gly"}, {"GGA", "Gly"}, {"GGG", "Gly"},
};

String normalizeSeq(const String& input) {
  String out = "";
  for (int i = 0; i < input.length(); ++i) {
    char c = static_cast<char>(toupper(static_cast<unsigned char>(input[i])));
    if (c == 'A' || c == 'T' || c == 'U' || c == 'C' || c == 'G') out += c;
  }
  return out;
}

String dnaComplement(const String& dna) {
  String out = "";
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'A') out += 'T';
    else if (c == 'T') out += 'A';
    else if (c == 'C') out += 'G';
    else if (c == 'G') out += 'C';
  }
  return out;
}

String dnaToRna(const String& dna) {
  String out = "";
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'T') out += 'U';
    else out += c;
  }
  return out;
}

const char* codonToAA(const String& codon) {
  for (const auto& item : kCodons) {
    if (codon == item.codon) return item.aa;
  }
  return "?";
}

void actionTranscribe() {
  String dna = "";
  if (!keyboardInput(dna, "DNA", false, false, g_telex_mode, 200, true)) return;
  dna = normalizeSeq(dna);
  if (dna.isEmpty()) return;
  String rna = dnaToRna(dna);
  showResult("Transcribe", rna);
}

void actionComplement() {
  String dna = "";
  if (!keyboardInput(dna, "DNA", false, false, g_telex_mode, 200, true)) return;
  dna = normalizeSeq(dna);
  if (dna.isEmpty()) return;
  String comp = dnaComplement(dna);
  showResult("Complement", comp);
}

void actionTranslate() {
  String seq = "";
  if (!keyboardInput(seq, "RNA/DNA", false, false, g_telex_mode, 200, true)) return;
  seq = normalizeSeq(seq);
  if (seq.isEmpty()) return;
  if (seq.indexOf('T') >= 0) seq = dnaToRna(seq);

  String out = "";
  for (int i = 0; i + 2 < seq.length(); i += 3) {
    String codon = seq.substring(i, i + 3);
    const char* aa = codonToAA(codon);
    if (String(aa) == "Stop") break;
    if (out.length() > 0) out += "-";
    out += aa;
  }
  if (out.isEmpty()) out = "No protein";
  showResult("Translate", out);
}

void actionGC() {
  String seq = "";
  if (!keyboardInput(seq, "DNA", false, false, g_telex_mode, 200, true)) return;
  seq = normalizeSeq(seq);
  if (seq.isEmpty()) return;
  int gc = 0;
  for (int i = 0; i < seq.length(); ++i) {
    char c = seq[i];
    if (c == 'G' || c == 'C') gc++;
  }
  double pct = static_cast<double>(gc) * 100.0 / seq.length();
  showResult("GC", "GC% = " + formatFloat(pct, 2));
}

MenuItem kMenu[] = {
    {"Transcribe DNA->RNA", actionTranscribe},
    {"Complement DNA", actionComplement},
    {"Translate RNA", actionTranslate},
    {"GC content", actionGC},
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
  M5.Display.print("idk-biology-molecular");

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
