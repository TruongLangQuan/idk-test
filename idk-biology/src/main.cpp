#include <M5Unified.h>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

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

String sanitizeSequence(const String& input, bool allowU) {
  String out = "";
  out.reserve(input.length());
  for (int i = 0; i < input.length(); ++i) {
    char c = static_cast<char>(toupper(static_cast<unsigned char>(input[i])));
    if (c == 'A' || c == 'C' || c == 'G' || c == 'T' || (allowU && c == 'U')) {
      if (!allowU && c == 'U') continue;
      out += c;
    }
  }
  return out;
}

String dnaComplement(const String& dna) {
  String out = "";
  out.reserve(dna.length());
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'A') out += 'T';
    else if (c == 'T') out += 'A';
    else if (c == 'C') out += 'G';
    else if (c == 'G') out += 'C';
    else if (c == 'U') out += 'A';
  }
  return out;
}

String transcriptionCoding(const String& dna) {
  String out = "";
  out.reserve(dna.length());
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'T') out += 'U';
    else out += c;
  }
  return out;
}

String transcriptionTemplate(const String& dna) {
  String out = "";
  out.reserve(dna.length());
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'A') out += 'U';
    else if (c == 'T') out += 'A';
    else if (c == 'C') out += 'G';
    else if (c == 'G') out += 'C';
    else if (c == 'U') out += 'A';
  }
  return out;
}

struct Codon {
  const char* codon;
  const char* aa;
};

const Codon kCodons[] = {
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

const char* codonToAA(const String& codon) {
  for (const auto& entry : kCodons) {
    if (codon == entry.codon) return entry.aa;
  }
  return nullptr;
}

String translateMRNA(const String& rna, int& startIndex, bool& hitStop) {
  startIndex = -1;
  hitStop = false;
  if (rna.length() < 3) return "";
  for (int i = 0; i + 2 < rna.length(); ++i) {
    if (rna.substring(i, i + 3) == "AUG") {
      startIndex = i;
      break;
    }
  }
  int pos = (startIndex >= 0) ? startIndex : 0;
  String out = "";
  for (int i = pos; i + 2 < rna.length(); i += 3) {
    String codon = rna.substring(i, i + 3);
    const char* aa = codonToAA(codon);
    if (!aa) break;
    if (strcmp(aa, "Stop") == 0) {
      hitStop = true;
      if (!out.isEmpty()) out += "-";
      out += "Stop";
      break;
    }
    if (!out.isEmpty()) out += "-";
    out += aa;
  }
  return out;
}

bool parseMonoGenotype(const String& input, char& a1, char& a2, char& gene) {
  String s = "";
  for (int i = 0; i < input.length(); ++i) {
    if (isalpha(static_cast<unsigned char>(input[i]))) s += input[i];
  }
  if (s.length() != 2) return false;
  char g = static_cast<char>(toupper(static_cast<unsigned char>(s[0])));
  if (toupper(static_cast<unsigned char>(s[1])) != g) return false;
  a1 = s[0];
  a2 = s[1];
  gene = g;
  return true;
}

String normalizePair(char gene, char a, char b) {
  char upper = static_cast<char>(toupper(static_cast<unsigned char>(gene)));
  char lower = static_cast<char>(tolower(static_cast<unsigned char>(gene)));
  bool dom = isupper(static_cast<unsigned char>(a)) || isupper(static_cast<unsigned char>(b));
  if (dom) {
    String out = "";
    out += upper;
    out += lower;
    return out;
  }
  String out = "";
  out += lower;
  out += lower;
  return out;
}

void actionTranscriptionCoding() {
  String dna = "";
  if (!keyboardInput(dna, "DNA coding", false, false, g_telex_mode, 200)) return;
  dna = sanitizeSequence(dna, false);
  if (dna.isEmpty()) {
    showResult("Coding", "No DNA input");
    return;
  }
  String rna = transcriptionCoding(dna);
  String out = "mRNA:\n" + rna;
  showResult("Coding", out);
}

void actionTranscriptionTemplate() {
  String dna = "";
  if (!keyboardInput(dna, "DNA template", false, false, g_telex_mode, 200)) return;
  dna = sanitizeSequence(dna, false);
  if (dna.isEmpty()) {
    showResult("Template", "No DNA input");
    return;
  }
  String rna = transcriptionTemplate(dna);
  String out = "mRNA:\n" + rna;
  showResult("Template", out);
}

void actionTranslation() {
  String rna = "";
  if (!keyboardInput(rna, "mRNA", false, false, g_telex_mode, 200)) return;
  rna = sanitizeSequence(rna, true);
  rna.replace("T", "U");
  if (rna.length() < 3) {
    showResult("Translate", "mRNA too short");
    return;
  }
  int startIndex = -1;
  bool hitStop = false;
  String aa = translateMRNA(rna, startIndex, hitStop);
  String out = "AA:\n" + aa + "\n";
  if (startIndex >= 0) {
    out += "Start: " + String(startIndex);
  } else {
    out += "Start: none";
  }
  if (hitStop) out += "\nStop: yes";
  showResult("Translate", out);
}

void actionComplement() {
  String dna = "";
  if (!keyboardInput(dna, "DNA", false, false, g_telex_mode, 200)) return;
  dna = sanitizeSequence(dna, true);
  if (dna.isEmpty()) {
    showResult("Complement", "No input");
    return;
  }
  String comp = dnaComplement(dna);
  showResult("Complement", comp);
}

void actionGCContent() {
  String dna = "";
  if (!keyboardInput(dna, "DNA/RNA", false, false, g_telex_mode, 200)) return;
  dna = sanitizeSequence(dna, true);
  if (dna.isEmpty()) {
    showResult("GC%", "No input");
    return;
  }
  int gc = 0;
  for (int i = 0; i < dna.length(); ++i) {
    char c = dna[i];
    if (c == 'G' || c == 'C') gc++;
  }
  double pct = (100.0 * gc) / dna.length();
  String out = "Length: " + String(dna.length()) + "\nGC%: " + formatFloat(pct, 2);
  showResult("GC%", out);
}

void actionMendelMono() {
  String p1 = "";
  String p2 = "";
  if (!keyboardInput(p1, "Parent 1", false, false, g_telex_mode, 8)) return;
  if (!keyboardInput(p2, "Parent 2", false, false, g_telex_mode, 8)) return;
  char a1, a2, b1, b2, gene1, gene2;
  if (!parseMonoGenotype(p1, a1, a2, gene1) || !parseMonoGenotype(p2, b1, b2, gene2)) {
    showResult("Mendel", "Use AA, Aa, aa");
    return;
  }
  if (gene1 != gene2) {
    showResult("Mendel", "Genes differ");
    return;
  }
  int countDom = 0;
  int countRec = 0;
  int countAA = 0, countAa = 0, countaa = 0;
  char p1a[2] = {a1, a2};
  char p2a[2] = {b1, b2};
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      bool dom = isupper(static_cast<unsigned char>(p1a[i])) ||
                 isupper(static_cast<unsigned char>(p2a[j]));
      if (dom) countDom++; else countRec++;
      String geno = normalizePair(gene1, p1a[i], p2a[j]);
      if (geno[0] == static_cast<char>(toupper(gene1)) && geno[1] == static_cast<char>(toupper(gene1))) {
        countAA++;
      } else if (geno[0] == static_cast<char>(toupper(gene1))) {
        countAa++;
      } else {
        countaa++;
      }
    }
  }
  char upper = static_cast<char>(toupper(gene1));
  char lower = static_cast<char>(tolower(gene1));
  String out = "Genotype:\n";
  out += String(upper) + String(upper) + ": " + String(countAA) + " ";
  out += String(upper) + String(lower) + ": " + String(countAa) + " ";
  out += String(lower) + String(lower) + ": " + String(countaa) + "\n";
  out += "Phenotype:\nDom: " + String(countDom) + " Rec: " + String(countRec);
  showResult("Mendel", out);
}

bool parseDiGenotype(const String& input, char& g1, char& g2, char alleles1[2], char alleles2[2]) {
  String s = "";
  for (int i = 0; i < input.length(); ++i) {
    if (isalpha(static_cast<unsigned char>(input[i]))) s += input[i];
  }
  if (s.length() != 4) return false;
  g1 = static_cast<char>(toupper(static_cast<unsigned char>(s[0])));
  g2 = static_cast<char>(toupper(static_cast<unsigned char>(s[2])));
  if (toupper(static_cast<unsigned char>(s[1])) != g1) return false;
  if (toupper(static_cast<unsigned char>(s[3])) != g2) return false;
  alleles1[0] = s[0];
  alleles1[1] = s[1];
  alleles2[0] = s[2];
  alleles2[1] = s[3];
  return true;
}

void actionMendelDihybrid() {
  String p1 = "";
  String p2 = "";
  if (!keyboardInput(p1, "Parent 1", false, false, g_telex_mode, 12)) return;
  if (!keyboardInput(p2, "Parent 2", false, false, g_telex_mode, 12)) return;
  char g1a, g2a, g1b, g2b;
  char a1[2], a2[2], b1[2], b2[2];
  if (!parseDiGenotype(p1, g1a, g2a, a1, a2) || !parseDiGenotype(p2, g1b, g2b, b1, b2)) {
    showResult("Dihybrid", "Use AaBb");
    return;
  }
  if (g1a != g1b || g2a != g2b) {
    showResult("Dihybrid", "Genes differ");
    return;
  }
  int domDom = 0, domRec = 0, recDom = 0, recRec = 0;
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        for (int m = 0; m < 2; ++m) {
          bool dom1 = isupper(static_cast<unsigned char>(a1[i])) ||
                      isupper(static_cast<unsigned char>(b1[k]));
          bool dom2 = isupper(static_cast<unsigned char>(a2[j])) ||
                      isupper(static_cast<unsigned char>(b2[m]));
          if (dom1 && dom2) domDom++;
          else if (dom1 && !dom2) domRec++;
          else if (!dom1 && dom2) recDom++;
          else recRec++;
        }
      }
    }
  }
  String out = "Phenotype (16):\n";
  out += "Dom/Dom: " + String(domDom) + "\n";
  out += "Dom/Rec: " + String(domRec) + "\n";
  out += "Rec/Dom: " + String(recDom) + "\n";
  out += "Rec/Rec: " + String(recRec);
  showResult("Dihybrid", out);
}

MenuItem kMenu[] = {
    {"Transcribe (coding)", actionTranscriptionCoding},
    {"Transcribe (template)", actionTranscriptionTemplate},
    {"Translate (mRNA)", actionTranslation},
    {"DNA complement", actionComplement},
    {"GC content", actionGCContent},
    {"Mendel mono", actionMendelMono},
    {"Mendel dihyb", actionMendelDihybrid},
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
  M5.Display.print("idk-biology");

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
