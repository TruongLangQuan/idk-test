#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;
constexpr uint32_t kWifiTimeoutMs = 8000;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct PosFlags {
  bool noun = false;
  bool verb = false;
  bool adj = false;
  bool adv = false;
  bool known = false;
};

struct IrregularVerb {
  const char* base;
  const char* past;
  const char* past_part;
  const char* third;
  const char* gerund;
};

struct IrregularNoun {
  const char* singular;
  const char* plural;
};

struct IrregularAdj {
  const char* base;
  const char* comp;
  const char* sup;
};

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

const IrregularVerb kIrregularVerbs[] = {
    {"be", "was/were", "been", "is", "being"},
    {"go", "went", "gone", "goes", "going"},
    {"have", "had", "had", "has", "having"},
    {"do", "did", "done", "does", "doing"},
    {"say", "said", "said", "says", "saying"},
    {"make", "made", "made", "makes", "making"},
    {"take", "took", "taken", "takes", "taking"},
    {"come", "came", "come", "comes", "coming"},
    {"see", "saw", "seen", "sees", "seeing"},
    {"get", "got", "gotten", "gets", "getting"},
    {"give", "gave", "given", "gives", "giving"},
    {"find", "found", "found", "finds", "finding"},
    {"think", "thought", "thought", "thinks", "thinking"},
    {"tell", "told", "told", "tells", "telling"},
    {"become", "became", "become", "becomes", "becoming"},
    {"show", "showed", "shown", "shows", "showing"},
    {"leave", "left", "left", "leaves", "leaving"},
    {"feel", "felt", "felt", "feels", "feeling"},
    {"put", "put", "put", "puts", "putting"},
    {"bring", "brought", "brought", "brings", "bringing"},
    {"begin", "began", "begun", "begins", "beginning"},
    {"keep", "kept", "kept", "keeps", "keeping"},
    {"hold", "held", "held", "holds", "holding"},
    {"write", "wrote", "written", "writes", "writing"},
    {"stand", "stood", "stood", "stands", "standing"},
    {"hear", "heard", "heard", "hears", "hearing"},
    {"let", "let", "let", "lets", "letting"},
    {"mean", "meant", "meant", "means", "meaning"},
    {"set", "set", "set", "sets", "setting"},
    {"meet", "met", "met", "meets", "meeting"},
    {"run", "ran", "run", "runs", "running"},
    {"pay", "paid", "paid", "pays", "paying"},
    {"sit", "sat", "sat", "sits", "sitting"},
    {"speak", "spoke", "spoken", "speaks", "speaking"},
    {"lie", "lay", "lain", "lies", "lying"},
    {"lay", "laid", "laid", "lays", "laying"},
    {"lose", "lost", "lost", "loses", "losing"},
    {"read", "read", "read", "reads", "reading"},
    {"cut", "cut", "cut", "cuts", "cutting"},
    {"eat", "ate", "eaten", "eats", "eating"},
    {"drink", "drank", "drunk", "drinks", "drinking"},
    {"sleep", "slept", "slept", "sleeps", "sleeping"},
    {"buy", "bought", "bought", "buys", "buying"},
    {"teach", "taught", "taught", "teaches", "teaching"},
    {"catch", "caught", "caught", "catches", "catching"},
};

const IrregularNoun kIrregularNouns[] = {
    {"man", "men"},
    {"woman", "women"},
    {"child", "children"},
    {"person", "people"},
    {"mouse", "mice"},
    {"foot", "feet"},
    {"tooth", "teeth"},
    {"goose", "geese"},
    {"ox", "oxen"},
    {"cactus", "cacti"},
    {"analysis", "analyses"},
};

const IrregularAdj kIrregularAdjs[] = {
    {"good", "better", "best"},
    {"bad", "worse", "worst"},
    {"far", "farther", "farthest"},
};

bool g_telex_mode = false;
String g_status = "Ready";
int g_menu_idx = 0;
int g_menu_scroll = 0;

String toLowerCopy(String s) {
  s.toLowerCase();
  return s;
}

bool isVowel(char c) {
  c = static_cast<char>(tolower(c));
  return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y';
}

const IrregularVerb* findIrregularVerb(const String& word) {
  for (const auto& v : kIrregularVerbs) {
    if (word == v.base) return &v;
  }
  return nullptr;
}

const IrregularNoun* findIrregularNoun(const String& word) {
  for (const auto& n : kIrregularNouns) {
    if (word == n.singular) return &n;
  }
  return nullptr;
}

const IrregularAdj* findIrregularAdj(const String& word) {
  for (const auto& a : kIrregularAdjs) {
    if (word == a.base) return &a;
  }
  return nullptr;
}

String pluralize(const String& w) {
  if (const auto* irr = findIrregularNoun(w)) return irr->plural;
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("s") || lower.endsWith("x") || lower.endsWith("z") || lower.endsWith("ch") ||
      lower.endsWith("sh")) {
    return w + "es";
  }
  if (lower.endsWith("y") && len > 1 && !isVowel(lower[len - 2])) {
    return w.substring(0, len - 1) + "ies";
  }
  if (lower.endsWith("f")) {
    return w.substring(0, len - 1) + "ves";
  }
  if (lower.endsWith("fe")) {
    return w.substring(0, len - 2) + "ves";
  }
  if (lower.endsWith("o") && len > 1 && !isVowel(lower[len - 2])) {
    return w + "es";
  }
  return w + "s";
}

String thirdPerson(const String& w) {
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("s") || lower.endsWith("x") || lower.endsWith("z") || lower.endsWith("ch") ||
      lower.endsWith("sh") || lower.endsWith("o")) {
    return w + "es";
  }
  if (lower.endsWith("y") && len > 1 && !isVowel(lower[len - 2])) {
    return w.substring(0, len - 1) + "ies";
  }
  return w + "s";
}

String pastTense(const String& w) {
  if (const auto* irr = findIrregularVerb(w)) return irr->past;
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("e")) return w + "d";
  if (lower.endsWith("y") && len > 1 && !isVowel(lower[len - 2])) {
    return w.substring(0, len - 1) + "ied";
  }
  return w + "ed";
}

String pastParticiple(const String& w) {
  if (const auto* irr = findIrregularVerb(w)) return irr->past_part;
  return pastTense(w);
}

String gerund(const String& w) {
  if (const auto* irr = findIrregularVerb(w)) return irr->gerund;
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("ie")) return w.substring(0, len - 2) + "ying";
  if (lower.endsWith("e") && !lower.endsWith("ee") && !lower.endsWith("oe")) {
    return w.substring(0, len - 1) + "ing";
  }
  return w + "ing";
}

String comparative(const String& w) {
  if (const auto* irr = findIrregularAdj(w)) return irr->comp;
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("y") && len > 1 && !isVowel(lower[len - 2])) {
    return w.substring(0, len - 1) + "ier";
  }
  if (len <= 5) return w + "er";
  return String("more ") + w;
}

String superlative(const String& w) {
  if (const auto* irr = findIrregularAdj(w)) return irr->sup;
  int len = w.length();
  if (len == 0) return w;
  String lower = toLowerCopy(w);
  if (lower.endsWith("y") && len > 1 && !isVowel(lower[len - 2])) {
    return w.substring(0, len - 1) + "iest";
  }
  if (len <= 5) return w + "est";
  return String("most ") + w;
}

PosFlags fetchPosTags(const String& word, String& status) {
  PosFlags flags;
  if (WiFi.status() != WL_CONNECTED) {
    status = "Offline";
    return flags;
  }
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = String("https://api.datamuse.com/words?sp=") + word + "&md=sp&max=1";
  if (!http.begin(client, url)) {
    status = "HTTP init failed";
    return flags;
  }
  int code = http.GET();
  if (code <= 0) {
    status = "HTTP error";
    http.end();
    return flags;
  }
  String body = http.getString();
  http.end();
  flags.noun = body.indexOf("\"n\"") >= 0;
  flags.verb = body.indexOf("\"v\"") >= 0;
  flags.adj = body.indexOf("adj") >= 0 || body.indexOf("\"a\"") >= 0;
  flags.adv = body.indexOf("adv") >= 0;
  flags.known = flags.noun || flags.verb || flags.adj || flags.adv;
  status = flags.known ? "Online" : "Online (pos?)";
  return flags;
}

String buildResult(const String& input, const PosFlags& pos) {
  String w = toLowerCopy(input);
  String out = "Word: " + w + "\n";

  bool showVerb = pos.verb || !pos.known;
  bool showNoun = pos.noun || !pos.known;
  bool showAdj = pos.adj || !pos.known;

  if (showVerb) {
    out += "Verb: ";
    out += w;
    out += " / ";
    out += thirdPerson(w);
    out += " / ";
    out += pastTense(w);
    out += " / ";
    out += pastParticiple(w);
    out += " / ";
    out += gerund(w);
    out += "\n";
  }

  if (showNoun) {
    out += "Noun plural: ";
    out += pluralize(w);
    out += "\n";
  }

  if (showAdj) {
    out += "Adj: ";
    out += comparative(w);
    out += " / ";
    out += superlative(w);
    out += "\n";
  }

  return out;
}

void ensureScroll() {
  if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
  if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
}

void drawMenu() {
  ensureScroll();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-wordform");

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 14);
  M5.Display.print(g_status);

  const MenuItem menu[] = {{"Tra wordform", nullptr}, {"WiFi connect", nullptr}, {"About", nullptr}};
  constexpr int kMenuCount = sizeof(menu) / sizeof(menu[0]);

  int y = 28;
  for (int i = 0; i < kMenuLines; ++i) {
    int idx = g_menu_scroll + i;
    if (idx >= kMenuCount) break;
    bool sel = (idx == g_menu_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    M5.Display.print(menu[idx].title);
    y += 16;
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print("A:Select  B:Next  PWR:Prev");
}

void actionLookup() {
  String word = "";
  if (!keyboardInput(word, "Word", false, true, g_telex_mode, 64, false)) return;
  if (word.isEmpty()) return;
  String status = "Offline";
  PosFlags pos = fetchPosTags(toLowerCopy(word), status);
  String out = buildResult(word, pos);
  out += "Mode: ";
  out += status;
  showResult("Wordform", out);
}

void actionWifi() {
  g_status = "WiFi...";
  drawMenu();
  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status, kWifiTimeoutMs);
  drawMenu();
}

void actionAbout() {
  showResult("About", "Offline rules + online POS (Datamuse)");
}

void handleMenuSelect() {
  switch (g_menu_idx) {
    case 0:
      actionLookup();
      break;
    case 1:
      actionWifi();
      break;
    case 2:
      actionAbout();
      break;
    default:
      break;
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  g_status = "WiFi scan...";
  drawMenu();
  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status, kWifiTimeoutMs);
  drawMenu();
}

void loop() {
  M5.update();
  if (M5.BtnB.wasPressed()) {
    g_menu_idx++;
  }
  if (M5.BtnPWR.wasPressed()) {
    g_menu_idx--;
  }
  if (g_menu_idx < 0) g_menu_idx = 2;
  if (g_menu_idx > 2) g_menu_idx = 0;

  if (M5.BtnA.wasPressed()) {
    handleMenuSelect();
    drawMenu();
  }

  delay(10);
}
