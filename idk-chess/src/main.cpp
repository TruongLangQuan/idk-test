#include <M5Unified.h>
#include <WiFi.h>

#include <algorithm>
#include <array>
#include <cstdint>

#ifndef IDK_CHESS_APP_TITLE
#define IDK_CHESS_APP_TITLE "idk-chess"
#endif

#ifndef IDK_CHESS_AI_LABEL
#define IDK_CHESS_AI_LABEL "Classic"
#endif

#ifndef IDK_CHESS_BASE_DEPTH
#define IDK_CHESS_BASE_DEPTH 4
#endif

#ifndef IDK_CHESS_ENDGAME_DEPTH
#define IDK_CHESS_ENDGAME_DEPTH 5
#endif

#ifndef IDK_CHESS_MAX_DEPTH
#define IDK_CHESS_MAX_DEPTH 6
#endif

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
// Active-LOW with INPUT_PULLUP.
// UP=32(Grove), DOWN=33(Grove), LEFT=25(header), RIGHT=26(header), CENTER=0(header)
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

struct ExtButton {
  int pin;
  bool pressed;
  uint32_t lastRepeatMs;
};

static ExtButton g_ext[] = {
    {kPinUp, false, 0},
    {kPinDown, false, 0},
    {kPinLeft, false, 0},
    {kPinRight, false, 0},
    {kPinCenter, false, 0},
};

static bool g_5way_detected = false;
static constexpr uint32_t kExtRepeatMs = 100;  // faster repeat for chess cursor

bool readExtPressed(int pin) {
  return digitalRead(pin) == LOW;
}

// ─── Chess constants ────────────────────────────────────────────
constexpr int8_t EMPTY = 0;
constexpr int8_t PC_WP = 1;
constexpr int8_t PC_WN = 2;
constexpr int8_t PC_WB = 3;
constexpr int8_t PC_WR = 4;
constexpr int8_t PC_WQ = 5;
constexpr int8_t PC_WK = 6;

constexpr int8_t PC_BP = -1;
constexpr int8_t PC_BN = -2;
constexpr int8_t PC_BB = -3;
constexpr int8_t PC_BR = -4;
constexpr int8_t PC_BQ = -5;
constexpr int8_t PC_BK = -6;

constexpr uint8_t CASTLE_WK = 1 << 0;
constexpr uint8_t CASTLE_WQ = 1 << 1;
constexpr uint8_t CASTLE_BK = 1 << 2;
constexpr uint8_t CASTLE_BQ = 1 << 3;

enum MoveFlags : uint8_t {
  MF_NONE = 0,
  MF_CAPTURE = 1 << 0,
  MF_EP = 1 << 1,
  MF_CASTLE_SHORT = 1 << 2,
  MF_CASTLE_LONG = 1 << 3,
  MF_PROMOTION = 1 << 4,
  MF_PAWN_DOUBLE = 1 << 5,
};

struct Move {
  uint8_t from;
  uint8_t to;
  int8_t promo;
  uint8_t flags;
};

struct MoveList {
  std::array<Move, 256> data{};
  int count = 0;
  void clear() { count = 0; }
  void push(const Move& m) {
    if (count < static_cast<int>(data.size())) {
      data[count++] = m;
    }
  }
};

struct Undo {
  Move m;
  int8_t movedPiece = 0;
  int8_t capturedPiece = 0;
  uint8_t castling = 0;
  int8_t epSquare = -1;
  int halfmove = 0;
};

struct GameState {
  std::array<int8_t, 64> board{};
  bool whiteToMove = true;
  uint8_t castling = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
  int8_t epSquare = -1;
  int halfmove = 0;
  int fullmove = 1;
};

enum class AppState {
  WIFI_SELECT,
  WIFI_CONNECTING,
  CHESS,
};

AppState g_app_state = AppState::WIFI_SELECT;
GameState g_game;
MoveList g_legal;
MoveList g_selected;

// ─── Offline mode and connectivity ──────────────────────────────
bool g_offline_mode = false;  // true = play vs AI locally, false = try WiFi first
bool g_force_offline = false; // true = user explicitly chose offline

std::array<String, 16> g_ssids{};
std::array<int, 16> g_rssi{};
std::array<bool, 16> g_secured{};
int g_wifi_count = 0;
int g_wifi_index = 0;
String g_wifi_status = "Scanning...";
unsigned long g_connect_start = 0;

int g_cursor_r = 7;
int g_cursor_c = 4;
int g_selected_sq = -1;
bool g_player_is_white = true;
int g_player_score = 0;
int g_bot_score = 0;
bool g_game_over = false;
String g_game_over_text;

bool g_promotion_active = false;
Move g_promotion_move{};
std::array<int8_t, 4> g_promotion_choices{};
int g_promotion_count = 0;
int g_promotion_index = 0;

bool g_castle_select_active = false;
std::array<Move, 2> g_castle_moves{};
int g_castle_count = 0;
int g_castle_index = 0;

void drawWifi();
void startGame();

struct KnownWifi {
  const char* ssid;
  const char* pass;
};

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

bool tryKnownWifiAutoConnect() {
  WiFi.mode(WIFI_STA);
  for (const auto& known : kKnownWifis) {
    int found = -1;
    for (int i = 0; i < g_wifi_count; ++i) {
      if (g_ssids[i] == known.ssid) {
        found = i;
        break;
      }
    }
    if (found < 0) continue;

    g_wifi_status = String("Auto connect: ") + known.ssid;
    g_wifi_index = found;
    drawWifi();
    WiFi.disconnect(true, true);
    delay(120);
    WiFi.begin(known.ssid, known.pass);
    uint32_t start = millis();
    while (millis() - start < 9000) {
      if (WiFi.status() == WL_CONNECTED) {
        g_wifi_status = String("Connected: ") + known.ssid;
        drawWifi();
        return true;
      }
      delay(120);
    }
  }
  WiFi.disconnect(true, true);
  return false;
}

uint32_t uiSignature() {
  uint32_t h = 2166136261u;
  auto mix = [&h](uint32_t v) {
    h ^= v;
    h *= 16777619u;
  };
  for (int i = 0; i < 64; ++i) {
    mix(static_cast<uint8_t>(g_game.board[i] + 16));
  }
  mix(static_cast<uint32_t>(g_cursor_r));
  mix(static_cast<uint32_t>(g_cursor_c));
  mix(static_cast<uint32_t>(g_selected_sq + 1));
  mix(static_cast<uint32_t>(g_game.whiteToMove ? 1 : 0));
  mix(static_cast<uint32_t>(g_game_over ? 1 : 0));
  mix(static_cast<uint32_t>(g_player_score));
  mix(static_cast<uint32_t>(g_bot_score));
  mix(static_cast<uint32_t>(g_selected.count));
  mix(static_cast<uint32_t>(g_castle_select_active ? 1 : 0));
  mix(static_cast<uint32_t>(g_castle_count));
  mix(static_cast<uint32_t>(g_castle_index));
  mix(static_cast<uint32_t>(g_promotion_active ? 1 : 0));
  mix(static_cast<uint32_t>(g_promotion_count));
  mix(static_cast<uint32_t>(g_promotion_index));
  return h;
}

int signOf(int8_t p) {
  if (p > 0) return 1;
  if (p < 0) return -1;
  return 0;
}

int pieceAbs(int8_t p) { return p >= 0 ? p : -p; }

bool isWhitePiece(int8_t p) { return p > 0; }
bool isBlackPiece(int8_t p) { return p < 0; }

int toIndex(int r, int c) { return r * 8 + c; }
int rowOf(int sq) { return sq / 8; }
int colOf(int sq) { return sq % 8; }

char pieceChar(int8_t p) {
  switch (p) {
    case PC_WP: return 'P';
    case PC_WN: return 'N';
    case PC_WB: return 'B';
    case PC_WR: return 'R';
    case PC_WQ: return 'Q';
    case PC_WK: return 'K';
    case PC_BP: return 'p';
    case PC_BN: return 'n';
    case PC_BB: return 'b';
    case PC_BR: return 'r';
    case PC_BQ: return 'q';
    case PC_BK: return 'k';
    default: return ' ';
  }
}

bool inBounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

int pieceSquareBonus(int8_t p, int sq) {
  static constexpr int pawnTable[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      8, 10, 10, -4, -4, 10, 10, 8,
      6, 8, 8, 12, 12, 8, 8, 6,
      4, 6, 10, 16, 16, 10, 6, 4,
      3, 5, 8, 14, 14, 8, 5, 3,
      2, 4, 6, 8, 8, 6, 4, 2,
      4, 6, 6, -8, -8, 6, 6, 4,
      0, 0, 0, 0, 0, 0, 0, 0,
  };
  static constexpr int knightTable[64] = {
      -20, -12, -8, -8, -8, -8, -12, -20,
      -12, -2, 2, 4, 4, 2, -2, -12,
      -8, 4, 8, 10, 10, 8, 4, -8,
      -8, 2, 10, 12, 12, 10, 2, -8,
      -8, 2, 10, 12, 12, 10, 2, -8,
      -8, 4, 8, 10, 10, 8, 4, -8,
      -12, -2, 2, 2, 2, 2, -2, -12,
      -20, -12, -8, -8, -8, -8, -12, -20,
  };
  static constexpr int bishopTable[64] = {
      -8, -4, -4, -4, -4, -4, -4, -8,
      -4, 4, 2, 2, 2, 2, 4, -4,
      -4, 6, 6, 8, 8, 6, 6, -4,
      -4, 2, 8, 10, 10, 8, 2, -4,
      -4, 2, 8, 10, 10, 8, 2, -4,
      -4, 6, 6, 8, 8, 6, 6, -4,
      -4, 4, 2, 2, 2, 2, 4, -4,
      -8, -4, -4, -4, -4, -4, -4, -8,
  };
  static constexpr int rookTable[64] = {
      2, 4, 6, 8, 8, 6, 4, 2,
      4, 8, 10, 12, 12, 10, 8, 4,
      -2, 0, 2, 4, 4, 2, 0, -2,
      -2, 0, 2, 4, 4, 2, 0, -2,
      -2, 0, 2, 4, 4, 2, 0, -2,
      -2, 0, 2, 4, 4, 2, 0, -2,
      -2, 0, 2, 4, 4, 2, 0, -2,
      0, 2, 4, 6, 6, 4, 2, 0,
  };
  static constexpr int queenTable[64] = {
      -10, -6, -4, -2, -2, -4, -6, -10,
      -6, 0, 2, 2, 2, 2, 0, -6,
      -4, 2, 4, 4, 4, 4, 2, -4,
      -2, 2, 4, 6, 6, 4, 2, -2,
      -2, 2, 4, 6, 6, 4, 2, -2,
      -4, 2, 4, 4, 4, 4, 2, -4,
      -6, 0, 2, 2, 2, 2, 0, -6,
      -10, -6, -4, -2, -2, -4, -6, -10,
  };
  static constexpr int kingTable[64] = {
      -16, -20, -20, -24, -24, -20, -20, -16,
      -12, -16, -16, -20, -20, -16, -16, -12,
      -8, -12, -12, -16, -16, -12, -12, -8,
      -4, -8, -8, -12, -12, -8, -8, -4,
      0, -4, -4, -8, -8, -4, -4, 0,
      4, 0, 0, -4, -4, 0, 0, 4,
      10, 12, 6, 0, 0, 6, 12, 10,
      12, 18, 8, 2, 2, 8, 18, 12,
  };

  int idx = sq;
  if (p < 0) {
    idx = (7 - rowOf(sq)) * 8 + colOf(sq);
  }
  switch (pieceAbs(p)) {
    case 1: return pawnTable[idx];
    case 2: return knightTable[idx];
    case 3: return bishopTable[idx];
    case 4: return rookTable[idx];
    case 5: return queenTable[idx];
    case 6: return kingTable[idx];
    default: return 0;
  }
}

void initBoard() {
  g_game.board = {
      PC_BR, PC_BN, PC_BB, PC_BQ, PC_BK, PC_BB, PC_BN, PC_BR,
      PC_BP, PC_BP, PC_BP, PC_BP, PC_BP, PC_BP, PC_BP, PC_BP,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      PC_WP, PC_WP, PC_WP, PC_WP, PC_WP, PC_WP, PC_WP, PC_WP,
      PC_WR, PC_WN, PC_WB, PC_WQ, PC_WK, PC_WB, PC_WN, PC_WR,
  };
  g_game.whiteToMove = true;
  g_game.castling = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
  g_game.epSquare = -1;
  g_game.halfmove = 0;
  g_game.fullmove = 1;

  g_cursor_r = 7;
  g_cursor_c = 4;
  g_selected_sq = -1;
  g_selected.clear();
  g_game_over = false;
  g_game_over_text = "";
  g_promotion_active = false;
  g_castle_select_active = false;
  g_historyCount = 0;
}

bool isSquareAttacked(const GameState& st, int sq, bool byWhite) {
  const int sr = rowOf(sq);
  const int sc = colOf(sq);

  const int pawnDir = byWhite ? -1 : 1;
  const int pr = sr - pawnDir;
  if (inBounds(pr, sc - 1)) {
    int8_t p = st.board[toIndex(pr, sc - 1)];
    if (byWhite && p == PC_WP) return true;
    if (!byWhite && p == PC_BP) return true;
  }
  if (inBounds(pr, sc + 1)) {
    int8_t p = st.board[toIndex(pr, sc + 1)];
    if (byWhite && p == PC_WP) return true;
    if (!byWhite && p == PC_BP) return true;
  }

  constexpr int kN[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
  for (auto& d : kN) {
    int r = sr + d[0], c = sc + d[1];
    if (!inBounds(r, c)) continue;
    int8_t p = st.board[toIndex(r, c)];
    if (byWhite && p == PC_WN) return true;
    if (!byWhite && p == PC_BN) return true;
  }

  constexpr int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
  for (auto& d : rookDirs) {
    int r = sr + d[0], c = sc + d[1];
    while (inBounds(r, c)) {
      int8_t p = st.board[toIndex(r, c)];
      if (p != 0) {
        if (byWhite && (p == PC_WR || p == PC_WQ)) return true;
        if (!byWhite && (p == PC_BR || p == PC_BQ)) return true;
        break;
      }
      r += d[0]; c += d[1];
    }
  }

  constexpr int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
  for (auto& d : bishopDirs) {
    int r = sr + d[0], c = sc + d[1];
    while (inBounds(r, c)) {
      int8_t p = st.board[toIndex(r, c)];
      if (p != 0) {
        if (byWhite && (p == PC_WB || p == PC_WQ)) return true;
        if (!byWhite && (p == PC_BB || p == PC_BQ)) return true;
        break;
      }
      r += d[0]; c += d[1];
    }
  }

  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (!dr && !dc) continue;
      int r = sr + dr, c = sc + dc;
      if (!inBounds(r, c)) continue;
      int8_t p = st.board[toIndex(r, c)];
      if (byWhite && p == PC_WK) return true;
      if (!byWhite && p == PC_BK) return true;
    }
  }

  return false;
}

int kingSquare(const GameState& st, bool whiteKing) {
  int8_t k = whiteKing ? PC_WK : PC_BK;
  for (int i = 0; i < 64; ++i) {
    if (st.board[i] == k) return i;
  }
  return -1;
}

bool inCheck(const GameState& st, bool whiteKing) {
  int ks = kingSquare(st, whiteKing);
  if (ks < 0) return false;
  return isSquareAttacked(st, ks, !whiteKing);
}

void addPromotionMoves(MoveList& out, int from, int to, uint8_t baseFlags, bool white) {
  const int8_t promo[4] = {PC_WQ, PC_WR, PC_WB, PC_WN};
  for (int i = 0; i < 4; ++i) {
    int8_t pp = promo[i] * (white ? 1 : -1);
    out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), pp, static_cast<uint8_t>(baseFlags | MF_PROMOTION)});
  }
}

void generatePseudo(const GameState& st, MoveList& out) {
  out.clear();
  bool white = st.whiteToMove;

  for (int from = 0; from < 64; ++from) {
    int8_t p = st.board[from];
    if (p == 0) continue;
    if (white && p < 0) continue;
    if (!white && p > 0) continue;

    int r = rowOf(from);
    int c = colOf(from);
    int type = pieceAbs(p);

    if (type == 1) {
      int dir = white ? -1 : 1;
      int startRow = white ? 6 : 1;
      int promoRow = white ? 0 : 7;

      int fr = r + dir;
      if (inBounds(fr, c) && st.board[toIndex(fr, c)] == 0) {
        int to = toIndex(fr, c);
        if (fr == promoRow) addPromotionMoves(out, from, to, MF_NONE, white);
        else out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_NONE});

        int fr2 = r + dir * 2;
        if (r == startRow && inBounds(fr2, c) && st.board[toIndex(fr2, c)] == 0) {
          out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(toIndex(fr2, c)), 0, MF_PAWN_DOUBLE});
        }
      }

      for (int dc = -1; dc <= 1; dc += 2) {
        int cr = r + dir;
        int cc = c + dc;
        if (!inBounds(cr, cc)) continue;
        int to = toIndex(cr, cc);
        int8_t tp = st.board[to];
        if (tp != 0 && signOf(tp) != signOf(p)) {
          if (cr == promoRow) addPromotionMoves(out, from, to, MF_CAPTURE, white);
          else out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_CAPTURE});
        }
        if (st.epSquare == to) {
          out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, static_cast<uint8_t>(MF_CAPTURE | MF_EP)});
        }
      }
      continue;
    }

    if (type == 2) {
      constexpr int kN[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
      for (auto& d : kN) {
        int nr = r + d[0], nc = c + d[1];
        if (!inBounds(nr, nc)) continue;
        int to = toIndex(nr, nc);
        int8_t tp = st.board[to];
        if (tp == 0) out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_NONE});
        else if (signOf(tp) != signOf(p)) out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_CAPTURE});
      }
      continue;
    }

    auto slide = [&](const int dirs[][2], int nDirs) {
      for (int i = 0; i < nDirs; ++i) {
        int dr = dirs[i][0], dc = dirs[i][1];
        int nr = r + dr, nc = c + dc;
        while (inBounds(nr, nc)) {
          int to = toIndex(nr, nc);
          int8_t tp = st.board[to];
          if (tp == 0) {
            out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_NONE});
          } else {
            if (signOf(tp) != signOf(p)) out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_CAPTURE});
            break;
          }
          nr += dr; nc += dc;
        }
      }
    };

    if (type == 3) {
      constexpr int dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
      slide(dirs, 4);
      continue;
    }
    if (type == 4) {
      constexpr int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
      slide(dirs, 4);
      continue;
    }
    if (type == 5) {
      constexpr int dirs[8][2] = {{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
      slide(dirs, 8);
      continue;
    }

    if (type == 6) {
      for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
          if (!dr && !dc) continue;
          int nr = r + dr, nc = c + dc;
          if (!inBounds(nr, nc)) continue;
          int to = toIndex(nr, nc);
          int8_t tp = st.board[to];
          if (tp == 0) out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_NONE});
          else if (signOf(tp) != signOf(p)) out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), 0, MF_CAPTURE});
        }
      }

      if (white) {
        if ((st.castling & CASTLE_WK) && st.board[toIndex(7, 5)] == 0 && st.board[toIndex(7, 6)] == 0) {
          if (!isSquareAttacked(st, toIndex(7, 4), false) && !isSquareAttacked(st, toIndex(7, 5), false) && !isSquareAttacked(st, toIndex(7, 6), false)) {
            out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(toIndex(7, 6)), 0, MF_CASTLE_SHORT});
          }
        }
        if ((st.castling & CASTLE_WQ) && st.board[toIndex(7, 1)] == 0 && st.board[toIndex(7, 2)] == 0 && st.board[toIndex(7, 3)] == 0) {
          if (!isSquareAttacked(st, toIndex(7, 4), false) && !isSquareAttacked(st, toIndex(7, 3), false) && !isSquareAttacked(st, toIndex(7, 2), false)) {
            out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(toIndex(7, 2)), 0, MF_CASTLE_LONG});
          }
        }
      } else {
        if ((st.castling & CASTLE_BK) && st.board[toIndex(0, 5)] == 0 && st.board[toIndex(0, 6)] == 0) {
          if (!isSquareAttacked(st, toIndex(0, 4), true) && !isSquareAttacked(st, toIndex(0, 5), true) && !isSquareAttacked(st, toIndex(0, 6), true)) {
            out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(toIndex(0, 6)), 0, MF_CASTLE_SHORT});
          }
        }
        if ((st.castling & CASTLE_BQ) && st.board[toIndex(0, 1)] == 0 && st.board[toIndex(0, 2)] == 0 && st.board[toIndex(0, 3)] == 0) {
          if (!isSquareAttacked(st, toIndex(0, 4), true) && !isSquareAttacked(st, toIndex(0, 3), true) && !isSquareAttacked(st, toIndex(0, 2), true)) {
            out.push(Move{static_cast<uint8_t>(from), static_cast<uint8_t>(toIndex(0, 2)), 0, MF_CASTLE_LONG});
          }
        }
      }
    }
  }
}

Undo makeMove(GameState& st, const Move& m) {
  Undo u;
  u.m = m;
  u.castling = st.castling;
  u.epSquare = st.epSquare;
  u.halfmove = st.halfmove;
  u.movedPiece = st.board[m.from];
  u.capturedPiece = st.board[m.to];

  int8_t piece = st.board[m.from];
  st.board[m.from] = 0;

  if (m.flags & MF_EP) {
    int capSq = st.whiteToMove ? (m.to + 8) : (m.to - 8);
    u.capturedPiece = st.board[capSq];
    st.board[capSq] = 0;
  }

  if (m.flags & MF_CASTLE_SHORT) {
    if (piece > 0) {
      st.board[toIndex(7, 5)] = PC_WR;
      st.board[toIndex(7, 7)] = 0;
    } else {
      st.board[toIndex(0, 5)] = PC_BR;
      st.board[toIndex(0, 7)] = 0;
    }
  } else if (m.flags & MF_CASTLE_LONG) {
    if (piece > 0) {
      st.board[toIndex(7, 3)] = PC_WR;
      st.board[toIndex(7, 0)] = 0;
    } else {
      st.board[toIndex(0, 3)] = PC_BR;
      st.board[toIndex(0, 0)] = 0;
    }
  }

  int8_t put = piece;
  if (m.flags & MF_PROMOTION) put = m.promo;
  st.board[m.to] = put;

  st.epSquare = -1;
  if (m.flags & MF_PAWN_DOUBLE) {
    st.epSquare = st.whiteToMove ? (m.to + 8) : (m.to - 8);
  }

  if (piece == PC_WK) st.castling &= static_cast<uint8_t>(~(CASTLE_WK | CASTLE_WQ));
  if (piece == PC_BK) st.castling &= static_cast<uint8_t>(~(CASTLE_BK | CASTLE_BQ));
  if (piece == PC_WR && m.from == toIndex(7, 7)) st.castling &= static_cast<uint8_t>(~CASTLE_WK);
  if (piece == PC_WR && m.from == toIndex(7, 0)) st.castling &= static_cast<uint8_t>(~CASTLE_WQ);
  if (piece == PC_BR && m.from == toIndex(0, 7)) st.castling &= static_cast<uint8_t>(~CASTLE_BK);
  if (piece == PC_BR && m.from == toIndex(0, 0)) st.castling &= static_cast<uint8_t>(~CASTLE_BQ);

  if (u.capturedPiece == PC_WR && m.to == toIndex(7, 7)) st.castling &= static_cast<uint8_t>(~CASTLE_WK);
  if (u.capturedPiece == PC_WR && m.to == toIndex(7, 0)) st.castling &= static_cast<uint8_t>(~CASTLE_WQ);
  if (u.capturedPiece == PC_BR && m.to == toIndex(0, 7)) st.castling &= static_cast<uint8_t>(~CASTLE_BK);
  if (u.capturedPiece == PC_BR && m.to == toIndex(0, 0)) st.castling &= static_cast<uint8_t>(~CASTLE_BQ);

  if (pieceAbs(piece) == 1 || u.capturedPiece != 0) st.halfmove = 0;
  else st.halfmove++;

  st.whiteToMove = !st.whiteToMove;
  if (st.whiteToMove) st.fullmove++;

  return u;
}

void unmakeMove(GameState& st, const Undo& u) {
  st.whiteToMove = !st.whiteToMove;
  if (st.whiteToMove) st.fullmove--;

  st.castling = u.castling;
  st.epSquare = u.epSquare;
  st.halfmove = u.halfmove;

  int8_t piece = u.movedPiece;

  if (u.m.flags & MF_CASTLE_SHORT) {
    if (piece > 0) {
      st.board[toIndex(7, 7)] = PC_WR;
      st.board[toIndex(7, 5)] = 0;
    } else {
      st.board[toIndex(0, 7)] = PC_BR;
      st.board[toIndex(0, 5)] = 0;
    }
  } else if (u.m.flags & MF_CASTLE_LONG) {
    if (piece > 0) {
      st.board[toIndex(7, 0)] = PC_WR;
      st.board[toIndex(7, 3)] = 0;
    } else {
      st.board[toIndex(0, 0)] = PC_BR;
      st.board[toIndex(0, 3)] = 0;
    }
  }

  st.board[u.m.from] = piece;

  if (u.m.flags & MF_EP) {
    st.board[u.m.to] = 0;
    int capSq = st.whiteToMove ? (u.m.to + 8) : (u.m.to - 8);
    st.board[capSq] = u.capturedPiece;
  } else {
    st.board[u.m.to] = u.capturedPiece;
  }
}

void generateLegal(const GameState& st, MoveList& out) {
  MoveList pseudo;
  generatePseudo(st, pseudo);
  out.clear();

  for (int i = 0; i < pseudo.count; ++i) {
    Move m = pseudo.data[i];
    GameState cp = st;
    makeMove(cp, m);
    if (!inCheck(cp, !cp.whiteToMove)) {
      out.push(m);
    }
  }
}

int evalBoard(const GameState& st) {
  static constexpr int val[7] = {0, 100, 320, 330, 500, 900, 20000};
  int score = 0;
  int whitePieces = 0, blackPieces = 0;

  // Material + piece-square bonuses + endgame adjustments
  for (int sq = 0; sq < 64; ++sq) {
    int8_t p = st.board[sq];
    if (p > 0) {
      score += val[p] + pieceSquareBonus(p, sq);
      whitePieces++;
    } else if (p < 0) {
      score -= val[-p] + pieceSquareBonus(p, sq);
      blackPieces++;
    }
  }

  // King safety: in endgame, white king should be more active (closer to center)
  // In middlegame, white king should be safer (near corner)
  int totalPieces = whitePieces + blackPieces;
  for (int sq = 0; sq < 64; ++sq) {
    int8_t p = st.board[sq];
    if (p == PC_WK) {
      if (totalPieces < 10) {  // endgame
        int dist = abs((sq % 8) - 3) + abs((sq / 8) - 3);
        score += (14 - dist) * 5;  // encourage moving toward center
      }
    } else if (p == PC_BK) {
      if (totalPieces < 10) {
        int dist = abs((sq % 8) - 4) + abs((sq / 8) - 4);
        score -= (14 - dist) * 5;
      }
    }
  }

  // Mobility: more moves = better position (weight increased)
  MoveList whiteMoves;
  MoveList blackMoves;
  GameState whitePos = st;
  whitePos.whiteToMove = true;
  generateLegal(whitePos, whiteMoves);
  GameState blackPos = st;
  blackPos.whiteToMove = false;
  generateLegal(blackPos, blackMoves);
  score += (whiteMoves.count - blackMoves.count) * 5;  // increased from 3 to 5

  // Pawn structure bonus: connected pawns
  for (int sq = 0; sq < 64; ++sq) {
    if (st.board[sq] == PC_WP) {
      int c = sq % 8;
      if (c > 0 && st.board[sq - 1] == PC_WP) score += 10;  // pawn next to pawn
      if (c < 7 && st.board[sq + 1] == PC_WP) score += 10;
    } else if (st.board[sq] == PC_BP) {
      int c = sq % 8;
      if (c > 0 && st.board[sq - 1] == PC_BP) score -= 10;
      if (c < 7 && st.board[sq + 1] == PC_BP) score -= 10;
    }
  }

  return score;
}

int moveOrderingScore(const GameState& st, const Move& m) {
  static constexpr int val[7] = {0, 100, 320, 330, 500, 900, 20000};
  int score = 0;
  int8_t mover = st.board[m.from];
  int8_t target = st.board[m.to];
  if (m.flags & MF_EP) {
    target = st.whiteToMove ? PC_BP : PC_WP;
  }
  if (target != 0) {
    score += 12000 + val[pieceAbs(target)] * 16 - val[pieceAbs(mover)];
  }
  if (m.flags & MF_PROMOTION) {
    score += 10000 + val[pieceAbs(m.promo)];
  }
  if (m.flags & (MF_CASTLE_SHORT | MF_CASTLE_LONG)) {
    score += 1500;
  }
  GameState probe = st;
  makeMove(probe, m);
  if (inCheck(probe, probe.whiteToMove)) {
    score += 2200;
  }
  score += pieceSquareBonus(mover, m.to) - pieceSquareBonus(mover, m.from);
  return score;
}

void sortMoves(const GameState& st, MoveList& list) {
  std::sort(list.data.begin(), list.data.begin() + list.count,
            [&](const Move& a, const Move& b) {
              return moveOrderingScore(st, a) > moveOrderingScore(st, b);
            });
}

// ─── Zobrist Hashing ────────────────────────────────────────────
static uint64_t g_zobrist_pieces[13][64];   // piece_index(0-12) x square
static uint64_t g_zobrist_side;             // side to move
static uint64_t g_zobrist_castling[16];     // castling rights
static uint64_t g_zobrist_ep[65];           // en passant file (+1 for none)
static bool g_zobrist_init = false;

static uint64_t xorshift64(uint64_t& s) {
  s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s;
}

void initZobrist() {
  if (g_zobrist_init) return;
  uint64_t seed = 0x123456789ABCDEF0ULL;
  for (int p = 0; p < 13; ++p)
    for (int s = 0; s < 64; ++s)
      g_zobrist_pieces[p][s] = xorshift64(seed);
  g_zobrist_side = xorshift64(seed);
  for (int i = 0; i < 16; ++i) g_zobrist_castling[i] = xorshift64(seed);
  for (int i = 0; i < 65; ++i) g_zobrist_ep[i] = xorshift64(seed);
  g_zobrist_init = true;
}

int pieceToIndex(int8_t p) {
  // Maps piece -6..-1, 0, 1..6 to 0..12
  return p + 6;
}

uint64_t computeHash(const GameState& st) {
  uint64_t h = 0;
  for (int sq = 0; sq < 64; ++sq) {
    if (st.board[sq] != 0) h ^= g_zobrist_pieces[pieceToIndex(st.board[sq])][sq];
  }
  if (!st.whiteToMove) h ^= g_zobrist_side;
  h ^= g_zobrist_castling[st.castling & 0xF];
  h ^= g_zobrist_ep[st.epSquare < 0 ? 64 : st.epSquare];
  return h;
}

// ─── Transposition Table ────────────────────────────────────────
enum TTFlag : uint8_t { TT_EXACT = 0, TT_ALPHA = 1, TT_BETA = 2 };

struct TTEntry {
  uint64_t key = 0;
  int16_t score = 0;
  uint8_t depth = 0;
  TTFlag flag = TT_EXACT;
  Move bestMove{};
};

static constexpr int kTTSize = 4096;  // small for ESP32 memory
static TTEntry g_tt[kTTSize];

void clearTT() {
  memset(g_tt, 0, sizeof(g_tt));
}

TTEntry* probeTT(uint64_t key) {
  TTEntry& e = g_tt[key % kTTSize];
  if (e.key == key) return &e;
  return nullptr;
}

void storeTT(uint64_t key, int score, int depth, TTFlag flag, const Move& bestMove) {
  TTEntry& e = g_tt[key % kTTSize];
  if (e.key == key && e.depth > depth) return;  // don't overwrite deeper entries
  e.key = key;
  e.score = static_cast<int16_t>(score);
  e.depth = static_cast<uint8_t>(depth);
  e.flag = flag;
  e.bestMove = bestMove;
}

// ─── Killer Moves ───────────────────────────────────────────────
static Move g_killers[IDK_CHESS_MAX_DEPTH + 2][2];

void storeKiller(int ply, const Move& m) {
  if (ply >= IDK_CHESS_MAX_DEPTH + 2) return;
  if (!moveEquals(m, g_killers[ply][0])) {
    g_killers[ply][1] = g_killers[ply][0];
    g_killers[ply][0] = m;
  }
}

bool isKiller(int ply, const Move& m) {
  if (ply >= IDK_CHESS_MAX_DEPTH + 2) return false;
  return moveEquals(m, g_killers[ply][0]) || moveEquals(m, g_killers[ply][1]);
}

// ─── Enhanced Move Ordering with killers ────────────────────────
int moveOrderingScoreEx(const GameState& st, const Move& m, int ply, const Move* ttMove) {
  int score = moveOrderingScore(st, m);
  if (ttMove && moveEquals(m, *ttMove)) score += 50000;  // TT move first
  if (isKiller(ply, m)) score += 8000;  // killer bonus
  return score;
}

void sortMovesEx(const GameState& st, MoveList& list, int ply, const Move* ttMove) {
  std::sort(list.data.begin(), list.data.begin() + list.count,
            [&](const Move& a, const Move& b) {
              return moveOrderingScoreEx(st, a, ply, ttMove) > moveOrderingScoreEx(st, b, ply, ttMove);
            });
}

// ─── Quiescence Search ──────────────────────────────────────────
int quiescence(GameState& st, int alpha, int beta) {
  int raw = evalBoard(st);
  int stand_pat = st.whiteToMove ? raw : -raw;
  if (stand_pat >= beta) return beta;
  if (stand_pat > alpha) alpha = stand_pat;

  MoveList legal;
  generateLegal(st, legal);

  for (int i = 0; i < legal.count; ++i) {
    const Move& m = legal.data[i];
    if (!(m.flags & (MF_CAPTURE | MF_EP | MF_PROMOTION))) continue;  // captures/promos only

    Undo u = makeMove(st, m);
    int score = -quiescence(st, -beta, -alpha);
    unmakeMove(st, u);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

// ─── Negamax with TT + Quiescence + Killers ─────────────────────
int negamax(GameState& st, int depth, int alpha, int beta, int ply) {
  uint64_t hash = computeHash(st);

  // Probe transposition table
  TTEntry* tte = probeTT(hash);
  Move* ttMove = nullptr;
  if (tte && tte->depth >= depth) {
    if (tte->flag == TT_EXACT) return tte->score;
    if (tte->flag == TT_ALPHA && tte->score <= alpha) return alpha;
    if (tte->flag == TT_BETA && tte->score >= beta) return beta;
  }
  if (tte) ttMove = &tte->bestMove;

  MoveList legal;
  generateLegal(st, legal);

  if (legal.count == 0) {
    if (inCheck(st, st.whiteToMove)) return -100000 + ply;
    return 0;  // stalemate
  }

  if (depth <= 0) return quiescence(st, alpha, beta);

  sortMovesEx(st, legal, ply, ttMove);

  int best = -200000;
  Move bestMove = legal.data[0];
  TTFlag flag = TT_ALPHA;

  for (int i = 0; i < legal.count; ++i) {
    Undo u = makeMove(st, legal.data[i]);
    int score = -negamax(st, depth - 1, -beta, -alpha, ply + 1);
    unmakeMove(st, u);

    if (score > best) {
      best = score;
      bestMove = legal.data[i];
    }
    if (best > alpha) {
      alpha = best;
      flag = TT_EXACT;
    }
    if (alpha >= beta) {
      if (!(legal.data[i].flags & (MF_CAPTURE | MF_EP))) {
        storeKiller(ply, legal.data[i]);  // non-capture cutoff = killer
      }
      flag = TT_BETA;
      break;
    }
  }

  storeTT(hash, best, depth, flag, bestMove);
  return best;
}

// ─── Simple Opening Book ────────────────────────────────────────
struct BookEntry {
  const char* moves;  // space-separated algebraic (e.g. "e2e4 e7e5 g1f3")
};

static const BookEntry kOpeningBook[] = {
  {"e2e4 e7e5 g1f3 b8c6 f1b5"},  // Ruy Lopez
  {"e2e4 e7e5 g1f3 b8c6 f1c4"},  // Italian
  {"d2d4 d7d5 c2c4"},              // Queen's Gambit
  {"e2e4 c7c5 g1f3 d7d6 d2d4"},  // Sicilian
  {"e2e4 e7e6 d2d4 d7d5"},        // French
  {"d2d4 g8f6 c2c4 g7g6"},        // King's Indian
  {"e2e4 e7e5 f1c4 g8f6"},        // Two Knights
  {"g1f3 d7d5 c2c4"},              // English-Reti
};
constexpr int kBookCount = sizeof(kOpeningBook) / sizeof(kOpeningBook[0]);

struct BookMove { uint8_t from; uint8_t to; };

bool parseAlgebraic(const char* s, BookMove& bm) {
  if (strlen(s) < 4) return false;
  int fc = s[0] - 'a', fr = 8 - (s[1] - '0');
  int tc = s[2] - 'a', tr = 8 - (s[3] - '0');
  if (fc < 0 || fc > 7 || fr < 0 || fr > 7) return false;
  if (tc < 0 || tc > 7 || tr < 0 || tr > 7) return false;
  bm.from = fr * 8 + fc;
  bm.to = tr * 8 + tc;
  return true;
}

static std::array<BookMove, 32> g_history;
static int g_historyCount = 0;

void recordMove(const Move& m) {
  if (g_historyCount < 32) {
    g_history[g_historyCount++] = {m.from, m.to};
  }
}

bool tryBookMove(const GameState& st, Move& out) {
  if (g_historyCount > 10) return false;  // out of book range

  for (int b = 0; b < kBookCount; ++b) {
    String line = kOpeningBook[b].moves;
    int pos = 0;
    bool match = true;
    int moveIdx = 0;

    // Parse all moves in the book line
    std::array<BookMove, 16> bookMoves;
    int bookLen = 0;
    int start = 0;
    for (int i = 0; i <= (int)line.length(); ++i) {
      if (i == (int)line.length() || line[i] == ' ') {
        String token = line.substring(start, i);
        BookMove bm;
        if (parseAlgebraic(token.c_str(), bm) && bookLen < 16) {
          bookMoves[bookLen++] = bm;
        }
        start = i + 1;
      }
    }

    // Check if history matches this book line
    if (g_historyCount >= bookLen) continue;
    match = true;
    for (int i = 0; i < g_historyCount && i < bookLen; ++i) {
      if (g_history[i].from != bookMoves[i].from || g_history[i].to != bookMoves[i].to) {
        match = false;
        break;
      }
    }

    if (match && g_historyCount < bookLen) {
      BookMove& next = bookMoves[g_historyCount];
      // Find this in legal moves
      MoveList legal;
      generateLegal(st, legal);
      for (int i = 0; i < legal.count; ++i) {
        if (legal.data[i].from == next.from && legal.data[i].to == next.to) {
          out = legal.data[i];
          return true;
        }
      }
    }
  }
  return false;
}

Move findBestMove(const GameState& st, int depth) {
  // Try opening book first
  Move bookMove;
  if (tryBookMove(st, bookMove)) return bookMove;

  initZobrist();
  memset(g_killers, 0, sizeof(g_killers));

  MoveList legal;
  generateLegal(st, legal);
  if (legal.count == 0) return Move{0, 0, 0, MF_NONE};

  // Iterative deepening
  Move best = legal.data[0];
  int bestScore = -200000;

  for (int d = 1; d <= depth; ++d) {
    sortMovesEx(st, legal, 0, nullptr);
    int currentBest = -200000;
    Move currentBestMove = legal.data[0];

    for (int i = 0; i < legal.count; ++i) {
      GameState cp = st;
      makeMove(cp, legal.data[i]);
      int score = -negamax(cp, d - 1, -200000, 200000, 1);
      if (score > currentBest) {
        currentBest = score;
        currentBestMove = legal.data[i];
      }
    }

    best = currentBestMove;
    bestScore = currentBest;
  }
  return best;
}

bool moveEquals(const Move& a, const Move& b) {
  return a.from == b.from && a.to == b.to && a.flags == b.flags && a.promo == b.promo;
}

bool executeMove(const Move& m) {
  bool found = false;
  for (int i = 0; i < g_legal.count; ++i) {
    if (moveEquals(g_legal.data[i], m)) {
      found = true;
      break;
    }
  }
  if (!found) return false;

  recordMove(m);
  makeMove(g_game, m);
  g_selected_sq = -1;
  g_selected.clear();
  generateLegal(g_game, g_legal);

  if (g_legal.count == 0) {
    g_game_over = true;
    if (inCheck(g_game, g_game.whiteToMove)) {
      if (g_game.whiteToMove) {
        g_game_over_text = "Bot wins";
        g_bot_score++;
      } else {
        g_game_over_text = "You win";
        g_player_score++;
      }
    } else {
      g_game_over_text = "Draw";
    }
  }
  return true;
}

void maybeStartPromotionMenu(int from, int to) {
  g_promotion_count = 0;
  for (int i = 0; i < g_legal.count; ++i) {
    const Move& m = g_legal.data[i];
    if (m.from == from && m.to == to && (m.flags & MF_PROMOTION)) {
      if (g_promotion_count < 4) {
        g_promotion_choices[g_promotion_count++] = m.promo;
      }
    }
  }
  if (g_promotion_count > 0) {
    g_promotion_active = true;
    g_promotion_move = Move{static_cast<uint8_t>(from), static_cast<uint8_t>(to), g_promotion_choices[0], static_cast<uint8_t>(MF_PROMOTION)};
    g_promotion_index = 0;
  }
}

void startCastleSelection() {
  g_castle_count = 0;
  if (g_selected_sq < 0) return;
  int8_t p = g_game.board[g_selected_sq];
  if (!(p == PC_WK || p == PC_BK)) return;

  for (int i = 0; i < g_selected.count; ++i) {
    const Move& m = g_selected.data[i];
    if (m.flags & MF_CASTLE_SHORT) {
      g_castle_moves[g_castle_count++] = m;
    }
    if (m.flags & MF_CASTLE_LONG) {
      g_castle_moves[g_castle_count++] = m;
    }
  }

  if (g_castle_count > 0) {
    g_castle_select_active = true;
    g_castle_index = 0;
  }
}

void buildSelectedMoves() {
  g_selected.clear();
  if (g_selected_sq < 0) return;
  for (int i = 0; i < g_legal.count; ++i) {
    if (g_legal.data[i].from == g_selected_sq) g_selected.push(g_legal.data[i]);
  }
}

bool isValidDest(int sq) {
  for (int i = 0; i < g_selected.count; ++i) {
    if (g_selected.data[i].to == sq) return true;
  }
  return false;
}

bool hasPromotionFromTo(int from, int to) {
  for (int i = 0; i < g_selected.count; ++i) {
    const Move& m = g_selected.data[i];
    if (m.from == from && m.to == to && (m.flags & MF_PROMOTION)) return true;
  }
  return false;
}

bool hasCastleOptions() {
  for (int i = 0; i < g_selected.count; ++i) {
    if (g_selected.data[i].flags & (MF_CASTLE_SHORT | MF_CASTLE_LONG)) return true;
  }
  return false;
}

bool inputPassword(String& out) {
  const char* row1 = "1234567890";
  const char* row2 = "abcdefghijklmnopqrstuvwxyz";
  const char* row3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* row4 = "!@#$%^&*()-_=+[]{};:,.?/\\";
  const char* top[] = {"OK", "DEL", "SPACE", "CANCEL"};
  constexpr int topCount = 4;

  int row = 0;
  int col = 0;
  String value = out;

  auto rowLen = [&](int r) -> int {
    if (r == 0) return topCount;
    if (r == 1) return static_cast<int>(strlen(row1));
    if (r == 2) return static_cast<int>(strlen(row2));
    if (r == 3) return static_cast<int>(strlen(row3));
    return static_cast<int>(strlen(row4));
  };

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.println("WiFi Password");
    M5.Display.setCursor(2, 14);
    M5.Display.print("Len: ");
    M5.Display.print(value.length());

    M5.Display.drawRect(2, 24, 236, 12, TFT_DARKGREY);
    M5.Display.setCursor(4, 26);
    for (size_t i = 0; i < value.length() && i < 42; ++i) M5.Display.print('*');

    int y = 42;
    for (int i = 0; i < topCount; ++i) {
      M5.Display.setTextColor((row == 0 && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(4 + i * 58, y);
      M5.Display.print(top[i]);
    }

    auto drawRow = [&](int rr, const char* text, int yy) {
      int n = strlen(text);
      for (int i = 0; i < n && i < 28; ++i) {
        M5.Display.setTextColor((row == rr && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(4 + (i % 14) * 16, yy + (i / 14) * 10);
        M5.Display.print(text[i]);
      }
    };
    drawRow(1, row1, 56);
    drawRow(2, row2, 68);
    drawRow(3, row3, 88);
    drawRow(4, row4, 108);
  };

  draw();
  while (true) {
    M5.update();
    if (M5.BtnPWR.wasPressed()) {
      row = (row + 1) % 5;
      int mx = rowLen(row);
      if (col >= mx) col = mx - 1;
      if (col < 0) col = 0;
      draw();
    }
    if (M5.BtnB.wasPressed()) {
      int mx = rowLen(row);
      col = (col + 1) % mx;
      draw();
    }
    if (M5.BtnA.wasPressed()) {
      if (row == 0) {
        if (col == 0) { out = value; return true; }
        if (col == 1) { if (!value.isEmpty()) value.remove(value.length() - 1); }
        if (col == 2) { if (value.length() < 63) value += ' '; }
        if (col == 3) { return false; }
      } else {
        char ch = 0;
        if (row == 1) ch = row1[col];
        if (row == 2) ch = row2[col];
        if (row == 3) ch = row3[col];
        if (row == 4) ch = row4[col];
        if (ch && value.length() < 63) value += ch;
      }
      draw();
    }
    delay(8);
  }
}

// ─── 5-way tactile switch input handler ────────────────────────
void handleExtButtons() {
  if (!g_5way_detected) return;
  const uint32_t now = millis();

  auto handleDir = [&](ExtButton& btn, int dx, int dy) {
    const bool down = readExtPressed(btn.pin);
    if (!down) {
      btn.pressed = false;
      return;
    }
    bool trigger = false;
    if (!btn.pressed) {
      btn.pressed = true;
      btn.lastRepeatMs = now;
      trigger = true;
    } else if (now - btn.lastRepeatMs > kExtRepeatMs) {
      btn.lastRepeatMs = now;
      trigger = true;
    }
    if (trigger) {
      // dx controls column (LEFT=-1, RIGHT=+1)
      // dy controls row (UP=-1, DOWN=+1)
      if (dx != 0) g_cursor_c = (g_cursor_c + dx + 8) % 8;
      if (dy != 0) g_cursor_r = (g_cursor_r + dy + 8) % 8;
    }
  };

  handleDir(g_ext[0], 0, -1);  // UP
  handleDir(g_ext[1], 0, 1);   // DOWN
  handleDir(g_ext[2], -1, 0);  // LEFT
  handleDir(g_ext[3], 1, 0);   // RIGHT

  // CENTER — select square
  ExtButton& center = g_ext[4];
  const bool cDown = (now > 400) && readExtPressed(center.pin);
  if (!cDown) {
    center.pressed = false;
    return;
  }
  if (!center.pressed) {
    center.pressed = true;
    // Simulate M5.BtnA press
    int sq = toIndex(g_cursor_r, g_cursor_c);
    if (g_selected_sq < 0) {
      int8_t p = g_game.board[sq];
      if (p != EMPTY && (p > 0) == g_game.whiteToMove) {
        g_selected_sq = sq;
        buildSelectedMoves();
        if (hasCastleOptions()) startCastleSelection();
        else if (hasPromotionFromTo(sq, sq)) maybeStartPromotionMenu(sq, sq);
      }
    } else {
      if (isValidDest(sq)) {
        if (hasPromotionFromTo(g_selected_sq, sq)) {
          maybeStartPromotionMenu(g_selected_sq, sq);
        } else {
          Move m{static_cast<uint8_t>(g_selected_sq), static_cast<uint8_t>(sq), 0, MF_NONE};
          bool found = false;
          for (int i = 0; i < g_legal.count; ++i) {
            if (g_legal.data[i].from == m.from && g_legal.data[i].to == m.to && g_legal.data[i].promo == 0) {
              m = g_legal.data[i];
              found = true;
              break;
            }
          }
          if (found) {
            executeMove(m);
            g_selected_sq = -1;
            g_selected.clear();
          }
        }
      } else {
        g_selected_sq = -1;
        g_selected.clear();
      }
    }
  }
}

void drawWifi() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("%s WiFi\n", IDK_CHESS_APP_TITLE);
  M5.Display.println("Next/Prev:scroll  M5:connect");
  M5.Display.println("M5+Prev:offline");
  M5.Display.println(g_wifi_status);

  if (g_wifi_count <= 0) {
    M5.Display.println("No AP found");
    return;
  }

  int start = std::max(0, g_wifi_index - 2);
  int end = std::min(g_wifi_count, start + 4);
  for (int i = start; i < end; ++i) {
    if (i == g_wifi_index) M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    else M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.printf("%c %s (%d)\n", i == g_wifi_index ? '>' : ' ', g_ssids[i].c_str(), g_rssi[i]);
  }
}

void scanWifi() {
  g_wifi_status = "Scanning...";
  drawWifi();
  g_wifi_count = WiFi.scanNetworks();
  if (g_wifi_count < 0) g_wifi_count = 0;
  if (g_wifi_count > static_cast<int>(g_ssids.size())) g_wifi_count = g_ssids.size();
  for (int i = 0; i < g_wifi_count; ++i) {
    g_ssids[i] = WiFi.SSID(i);
    g_rssi[i] = WiFi.RSSI(i);
    g_secured[i] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  g_wifi_index = 0;
  if (g_wifi_count > 0 && tryKnownWifiAutoConnect()) {
    startGame();
    return;
  }
  if (g_wifi_count == 0) {
    // Auto-fallback to offline mode when no WiFi networks found
    g_offline_mode = true;
    g_wifi_status = "No WiFi - Going offline";
    drawWifi();
    delay(800);
    startGame();
    return;
  }
  g_wifi_status = "Select WiFi";
  drawWifi();
}

void startGame() {
  initBoard();
  generateLegal(g_game, g_legal);
  g_app_state = AppState::CHESS;
}

void drawBoard() {
  M5.Display.fillScreen(TFT_BLACK);

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("You:%d Bot:%d", g_player_score, g_bot_score);
  M5.Display.setCursor(2, 12);
  M5.Display.printf("AI:%s", IDK_CHESS_AI_LABEL);
  M5.Display.setCursor(136, 2);
  M5.Display.print(g_game.whiteToMove ? "Turn:You" : "Turn:Bot");

  constexpr int cell = 14;
  constexpr int boardX = 64;
  constexpr int boardY = 20;

  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      int sq = toIndex(r, c);
      bool light = ((r + c) & 1) == 0;
      uint16_t col = light ? M5.Display.color565(240, 217, 181) : M5.Display.color565(181, 136, 99);
      if (sq == g_selected_sq) col = M5.Display.color565(127, 201, 127);
      else if (isValidDest(sq)) col = M5.Display.color565(255, 215, 0);
      if (r == g_cursor_r && c == g_cursor_c) col = M5.Display.color565(100, 100, 255);

      int x = boardX + c * cell;
      int y = boardY + r * cell;
      M5.Display.fillRect(x, y, cell, cell, col);

      char ch = pieceChar(g_game.board[sq]);
      if (ch != ' ') {
        uint16_t pc = (g_game.board[sq] > 0) ? TFT_WHITE : TFT_BLACK;
        M5.Display.setTextColor(pc, col);
        M5.Display.setCursor(x + 4, y + 3);
        M5.Display.print(ch);
      }
    }
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 128);
  M5.Display.print("M5:sel Next:R/HoldL Prev:D/HoldU");

  if (g_castle_select_active) {
    M5.Display.fillRect(14, 44, 212, 46, TFT_NAVY);
    M5.Display.drawRect(14, 44, 212, 46, TFT_WHITE);
    M5.Display.setCursor(20, 50);
    M5.Display.print("Castle?");
    for (int i = 0; i < g_castle_count; ++i) {
      bool sh = (g_castle_moves[i].flags & MF_CASTLE_SHORT) != 0;
      M5.Display.setTextColor(i == g_castle_index ? TFT_YELLOW : TFT_WHITE, TFT_NAVY);
      M5.Display.setCursor(20 + i * 90, 66);
      M5.Display.print(sh ? "Short" : "Long");
    }
  }

  if (g_promotion_active) {
    M5.Display.fillRect(10, 38, 220, 54, TFT_DARKGREEN);
    M5.Display.drawRect(10, 38, 220, 54, TFT_WHITE);
    M5.Display.setCursor(18, 44);
    M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    M5.Display.print("Promotion");
    for (int i = 0; i < g_promotion_count; ++i) {
      M5.Display.setTextColor(i == g_promotion_index ? TFT_YELLOW : TFT_WHITE, TFT_DARKGREEN);
      M5.Display.setCursor(20 + i * 50, 64);
      M5.Display.print(pieceChar(g_promotion_choices[i]));
    }
  }

  if (g_game_over) {
    M5.Display.fillRect(24, 48, 190, 34, TFT_MAROON);
    M5.Display.drawRect(24, 48, 190, 34, TFT_WHITE);
    M5.Display.setCursor(34, 58);
    M5.Display.setTextColor(TFT_WHITE, TFT_MAROON);
    M5.Display.print(g_game_over_text);
    M5.Display.setCursor(34, 70);
    M5.Display.print("M5 to restart");
  }
}

void handleWifiInput() {
  // ─── 5-way support for WiFi selection ───────────────────────
  if (g_5way_detected) {
    if (readExtPressed(kPinDown) && g_wifi_count > 0) {
      static uint32_t lastDown = 0;
      uint32_t now = millis();
      if (now - lastDown > 200) {
        g_wifi_index = (g_wifi_index + 1) % g_wifi_count;
        drawWifi();
        lastDown = now;
      }
    }
    if (readExtPressed(kPinUp) && g_wifi_count > 0) {
      static uint32_t lastUp = 0;
      uint32_t now = millis();
      if (now - lastUp > 200) {
        g_wifi_index = (g_wifi_index + g_wifi_count - 1) % g_wifi_count;
        drawWifi();
        lastUp = now;
      }
    }
    if (readExtPressed(kPinCenter)) {
      static uint32_t lastCenter = 0;
      uint32_t now = millis();
      if (now - lastCenter > 400) {
        // CENTER button leads to WiFi connect (same as BtnA on single press)
        if (g_wifi_count == 0) {
          g_offline_mode = true;
          g_wifi_status = "No WiFi - Offline";
          startGame();
          lastCenter = now;
          return;
        }
        String pass = "";
        if (g_secured[g_wifi_index]) {
          bool ok = inputPassword(pass);
          if (!ok) {
            g_wifi_status = "Cancelled";
            drawWifi();
            lastCenter = now;
            return;
          }
        }
        WiFi.mode(WIFI_STA);
        if (g_secured[g_wifi_index]) WiFi.begin(g_ssids[g_wifi_index].c_str(), pass.c_str());
        else WiFi.begin(g_ssids[g_wifi_index].c_str());
        g_connect_start = millis();
        g_wifi_status = "Connecting: " + g_ssids[g_wifi_index];
        g_app_state = AppState::WIFI_CONNECTING;
        drawWifi();
        lastCenter = now;
        return;
      }
    }
  }

  // ─── M5 button support (original) ────────────────────────────
  if (M5.BtnB.wasPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index + 1) % g_wifi_count;
    drawWifi();
  }
  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index + g_wifi_count - 1) % g_wifi_count;
    drawWifi();
  }

  bool selectPressed = M5.BtnA.wasPressed();
  bool skipPressed = M5.BtnA.isPressed() && M5.BtnPWR.wasPressed();

  if (skipPressed) {
    g_offline_mode = true;
    g_wifi_status = "Offline mode";
    startGame();
    return;
  }

  if (selectPressed) {
    if (g_wifi_count == 0) {
      g_offline_mode = true;
      startGame();
      return;
    }
    String pass = "";
    if (g_secured[g_wifi_index]) {
      bool ok = inputPassword(pass);
      if (!ok) {
        g_wifi_status = "Cancelled";
        drawWifi();
        return;
      }
    }
    WiFi.mode(WIFI_STA);
    if (g_secured[g_wifi_index]) WiFi.begin(g_ssids[g_wifi_index].c_str(), pass.c_str());
    else WiFi.begin(g_ssids[g_wifi_index].c_str());
    g_connect_start = millis();
    g_wifi_status = "Connecting: " + g_ssids[g_wifi_index];
    g_app_state = AppState::WIFI_CONNECTING;
    drawWifi();
  }
}

void handleConnectingState() {
  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) {
    g_wifi_status = "Connected";
    delay(300);
    startGame();
    return;
  }

  if (millis() - g_connect_start > 10000) {
    WiFi.disconnect(true, true);
    g_wifi_status = "Connect fail";
    g_app_state = AppState::WIFI_SELECT;
    drawWifi();
  }
}

int chooseSearchDepth(const GameState& st) {
  int nonKingPieces = 0;
  for (int sq = 0; sq < 64; ++sq) {
    int8_t p = st.board[sq];
    if (p != 0 && pieceAbs(p) != 6) {
      nonKingPieces++;
    }
  }

  int depth = IDK_CHESS_BASE_DEPTH;
  if (st.halfmove > 20 || nonKingPieces <= 10) {
    depth = IDK_CHESS_ENDGAME_DEPTH;
  }
  if (nonKingPieces <= 6) {
    depth = std::min(IDK_CHESS_MAX_DEPTH, depth + 1);
  }

  MoveList legal;
  generateLegal(st, legal);
  if (legal.count <= 10) {
    depth = std::min(IDK_CHESS_MAX_DEPTH, depth + 1);
  }
  return depth;
}

void botStepIfNeeded() {
  if (g_game_over) return;
  if (g_game.whiteToMove == g_player_is_white) return;

  int depth = chooseSearchDepth(g_game);
  Move best = findBestMove(g_game, depth);
  recordMove(best);
  executeMove(best);
}

void moveCursorShort() {
  if (M5.BtnB.wasPressed()) {
    g_cursor_c = (g_cursor_c + 1) % 8;
  }
  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
    g_cursor_r = (g_cursor_r + 1) % 8;
  }
}

void moveCursorHold() {
  static bool nextWasPressed = false;
  static uint32_t nextPressTime = 0;
  static uint32_t lastMoveLeft = 0;
  static bool prevWasPressed = false;
  static uint32_t prevPressTime = 0;
  static uint32_t lastMoveUp = 0;

  if (M5.BtnB.wasPressed()) {
    nextWasPressed = true;
    nextPressTime = millis();
  }
  if (M5.BtnB.isPressed() && nextWasPressed) {
    if (millis() - nextPressTime > 200 && millis() - lastMoveLeft > 100) {
      g_cursor_c = (g_cursor_c + 7) % 8;
      lastMoveLeft = millis();
    }
  } else {
    nextWasPressed = false;
  }

  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
    prevWasPressed = true;
    prevPressTime = millis();
  }
  if (M5.BtnPWR.isPressed() && !M5.BtnA.isPressed() && prevWasPressed) {
    if (millis() - prevPressTime > 200 && millis() - lastMoveUp > 100) {
      g_cursor_r = (g_cursor_r + 7) % 8;
      lastMoveUp = millis();
    }
  } else {
    prevWasPressed = false;
  }
}

void handleChessInput() {
  // ─── 5-way tactile switch support (priority) ───────────────────
  handleExtButtons();

  // ─── M5 button support (fallback) ────────────────────────────
  bool selectPressed = M5.BtnA.wasPressed();
  bool castleCombo = M5.BtnA.isPressed() && M5.BtnPWR.wasPressed();

  if (g_game_over) {
    if (selectPressed) {
      initBoard();
      generateLegal(g_game, g_legal);
    }
    return;
  }

  if (g_game.whiteToMove != g_player_is_white) return;

  if (g_promotion_active) {
    if (M5.BtnB.wasPressed()) g_promotion_index = (g_promotion_index + 1) % g_promotion_count;
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) g_promotion_index = (g_promotion_index + g_promotion_count - 1) % g_promotion_count;
    if (selectPressed) {
      Move pm = g_promotion_move;
      pm.promo = g_promotion_choices[g_promotion_index];
      pm.flags = static_cast<uint8_t>(pm.flags | MF_PROMOTION);
      executeMove(pm);
      g_promotion_active = false;
    }
    return;
  }

  if (g_castle_select_active) {
    if (M5.BtnB.wasPressed()) g_castle_index = (g_castle_index + 1) % g_castle_count;
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) g_castle_index = (g_castle_index + g_castle_count - 1) % g_castle_count;
    if (selectPressed) {
      executeMove(g_castle_moves[g_castle_index]);
      g_castle_select_active = false;
    }
    return;
  }

  moveCursorShort();
  moveCursorHold();

  if (castleCombo && g_selected_sq >= 0 && hasCastleOptions()) {
    startCastleSelection();
    return;
  }

  if (!selectPressed) return;

  int sq = toIndex(g_cursor_r, g_cursor_c);
  if (g_selected_sq < 0) {
    int8_t p = g_game.board[sq];
    if (p == 0) return;
    if (g_game.whiteToMove && p < 0) return;
    if (!g_game.whiteToMove && p > 0) return;
    g_selected_sq = sq;
    buildSelectedMoves();
    return;
  }

  if (sq == g_selected_sq) {
    g_selected_sq = -1;
    g_selected.clear();
    return;
  }

  if (!isValidDest(sq)) {
    int8_t p = g_game.board[sq];
    if (p != 0 && ((g_game.whiteToMove && p > 0) || (!g_game.whiteToMove && p < 0))) {
      g_selected_sq = sq;
      buildSelectedMoves();
    }
    return;
  }

  if (hasPromotionFromTo(g_selected_sq, sq)) {
    maybeStartPromotionMenu(g_selected_sq, sq);
    return;
  }

  for (int i = 0; i < g_selected.count; ++i) {
    const Move& m = g_selected.data[i];
    if (m.to == sq && !(m.flags & MF_PROMOTION)) {
      executeMove(m);
      return;
    }
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setTextSize(1);

  // Configure 5-way tactile switch pins
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);
  delay(10);  // let pull-ups settle
  g_5way_detected = true;  // always enable — no harm if absent

  if (!g_force_offline) {
    scanWifi();
  } else {
    g_offline_mode = true;
    startGame();
    g_app_state = AppState::CHESS;
  }
}

void loop() {
  M5.update();
  static uint32_t lastSig = 0;
  static bool hasSig = false;

  if (g_app_state == AppState::WIFI_SELECT) {
    handleWifiInput();
    delay(10);
    return;
  }

  if (g_app_state == AppState::WIFI_CONNECTING) {
    handleConnectingState();
    delay(10);
    return;
  }

  handleChessInput();
  botStepIfNeeded();
  const uint32_t sig = uiSignature();
  if (!hasSig || sig != lastSig) {
    drawBoard();
    lastSig = sig;
    hasSig = true;
  }
  delay(12);
}
