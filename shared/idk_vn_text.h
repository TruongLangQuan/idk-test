#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace idk_vn_text {

enum MarkBits : uint16_t {
  MarkNone = 0,
  MarkAcute = 1 << 0,
  MarkGrave = 1 << 1,
  MarkHook = 1 << 2,
  MarkTilde = 1 << 3,
  MarkDot = 1 << 4,
  MarkCirc = 1 << 5,
  MarkBreve = 1 << 6,
  MarkHorn = 1 << 7,
  MarkStroke = 1 << 8,
};

struct GlyphInfo {
  char base;
  uint16_t marks;
};

static inline bool decodeUtf8(const String& text, int& index, uint32_t& cp) {
  if (index >= text.length()) return false;
  uint8_t c0 = static_cast<uint8_t>(text[index++]);
  if (c0 < 0x80) {
    cp = c0;
    return true;
  }
  if ((c0 & 0xE0) == 0xC0 && index < text.length()) {
    cp = ((c0 & 0x1F) << 6) | (static_cast<uint8_t>(text[index++]) & 0x3F);
    return true;
  }
  if ((c0 & 0xF0) == 0xE0 && index + 1 < text.length()) {
    cp = ((c0 & 0x0F) << 12) | ((static_cast<uint8_t>(text[index++]) & 0x3F) << 6) |
         (static_cast<uint8_t>(text[index++]) & 0x3F);
    return true;
  }
  if ((c0 & 0xF8) == 0xF0 && index + 2 < text.length()) {
    cp = ((c0 & 0x07) << 18) | ((static_cast<uint8_t>(text[index++]) & 0x3F) << 12) |
         ((static_cast<uint8_t>(text[index++]) & 0x3F) << 6) |
         (static_cast<uint8_t>(text[index++]) & 0x3F);
    return true;
  }
  cp = '?';
  return true;
}

static inline GlyphInfo mapGlyph(uint32_t cp) {
  switch (cp) {
    case 0xC0: return {'A', MarkGrave};
    case 0xC1: return {'A', MarkAcute};
    case 0xC2: return {'A', MarkCirc};
    case 0xC3: return {'A', MarkTilde};
    case 0xC8: return {'E', MarkGrave};
    case 0xC9: return {'E', MarkAcute};
    case 0xCA: return {'E', MarkCirc};
    case 0xCC: return {'I', MarkGrave};
    case 0xCD: return {'I', MarkAcute};
    case 0xD2: return {'O', MarkGrave};
    case 0xD3: return {'O', MarkAcute};
    case 0xD4: return {'O', MarkCirc};
    case 0xD5: return {'O', MarkTilde};
    case 0xD9: return {'U', MarkGrave};
    case 0xDA: return {'U', MarkAcute};
    case 0xDD: return {'Y', MarkAcute};
    case 0xE0: return {'a', MarkGrave};
    case 0xE1: return {'a', MarkAcute};
    case 0xE2: return {'a', MarkCirc};
    case 0xE3: return {'a', MarkTilde};
    case 0xE8: return {'e', MarkGrave};
    case 0xE9: return {'e', MarkAcute};
    case 0xEA: return {'e', MarkCirc};
    case 0xEC: return {'i', MarkGrave};
    case 0xED: return {'i', MarkAcute};
    case 0xF2: return {'o', MarkGrave};
    case 0xF3: return {'o', MarkAcute};
    case 0xF4: return {'o', MarkCirc};
    case 0xF5: return {'o', MarkTilde};
    case 0xF9: return {'u', MarkGrave};
    case 0xFA: return {'u', MarkAcute};
    case 0xFD: return {'y', MarkAcute};
    case 0x102: return {'A', MarkBreve};
    case 0x103: return {'a', MarkBreve};
    case 0x110: return {'D', MarkStroke};
    case 0x111: return {'d', MarkStroke};
    case 0x128: return {'I', MarkTilde};
    case 0x129: return {'i', MarkTilde};
    case 0x168: return {'U', MarkTilde};
    case 0x169: return {'u', MarkTilde};
    case 0x1A0: return {'O', MarkHorn};
    case 0x1A1: return {'o', MarkHorn};
    case 0x1AF: return {'U', MarkHorn};
    case 0x1B0: return {'u', MarkHorn};
    case 0x1EA0: return {'A', MarkDot};
    case 0x1EA1: return {'a', MarkDot};
    case 0x1EA2: return {'A', MarkHook};
    case 0x1EA3: return {'a', MarkHook};
    case 0x1EA4: return {'A', MarkCirc | MarkAcute};
    case 0x1EA5: return {'a', MarkCirc | MarkAcute};
    case 0x1EA6: return {'A', MarkCirc | MarkGrave};
    case 0x1EA7: return {'a', MarkCirc | MarkGrave};
    case 0x1EA8: return {'A', MarkCirc | MarkHook};
    case 0x1EA9: return {'a', MarkCirc | MarkHook};
    case 0x1EAA: return {'A', MarkCirc | MarkTilde};
    case 0x1EAB: return {'a', MarkCirc | MarkTilde};
    case 0x1EAC: return {'A', MarkDot | MarkCirc};
    case 0x1EAD: return {'a', MarkDot | MarkCirc};
    case 0x1EAE: return {'A', MarkBreve | MarkAcute};
    case 0x1EAF: return {'a', MarkBreve | MarkAcute};
    case 0x1EB0: return {'A', MarkBreve | MarkGrave};
    case 0x1EB1: return {'a', MarkBreve | MarkGrave};
    case 0x1EB2: return {'A', MarkBreve | MarkHook};
    case 0x1EB3: return {'a', MarkBreve | MarkHook};
    case 0x1EB4: return {'A', MarkBreve | MarkTilde};
    case 0x1EB5: return {'a', MarkBreve | MarkTilde};
    case 0x1EB6: return {'A', MarkDot | MarkBreve};
    case 0x1EB7: return {'a', MarkDot | MarkBreve};
    case 0x1EB8: return {'E', MarkDot};
    case 0x1EB9: return {'e', MarkDot};
    case 0x1EBA: return {'E', MarkHook};
    case 0x1EBB: return {'e', MarkHook};
    case 0x1EBC: return {'E', MarkTilde};
    case 0x1EBD: return {'e', MarkTilde};
    case 0x1EBE: return {'E', MarkCirc | MarkAcute};
    case 0x1EBF: return {'e', MarkCirc | MarkAcute};
    case 0x1EC0: return {'E', MarkCirc | MarkGrave};
    case 0x1EC1: return {'e', MarkCirc | MarkGrave};
    case 0x1EC2: return {'E', MarkCirc | MarkHook};
    case 0x1EC3: return {'e', MarkCirc | MarkHook};
    case 0x1EC4: return {'E', MarkCirc | MarkTilde};
    case 0x1EC5: return {'e', MarkCirc | MarkTilde};
    case 0x1EC6: return {'E', MarkDot | MarkCirc};
    case 0x1EC7: return {'e', MarkDot | MarkCirc};
    case 0x1EC8: return {'I', MarkHook};
    case 0x1EC9: return {'i', MarkHook};
    case 0x1ECA: return {'I', MarkDot};
    case 0x1ECB: return {'i', MarkDot};
    case 0x1ECC: return {'O', MarkDot};
    case 0x1ECD: return {'o', MarkDot};
    case 0x1ECE: return {'O', MarkHook};
    case 0x1ECF: return {'o', MarkHook};
    case 0x1ED0: return {'O', MarkCirc | MarkAcute};
    case 0x1ED1: return {'o', MarkCirc | MarkAcute};
    case 0x1ED2: return {'O', MarkCirc | MarkGrave};
    case 0x1ED3: return {'o', MarkCirc | MarkGrave};
    case 0x1ED4: return {'O', MarkCirc | MarkHook};
    case 0x1ED5: return {'o', MarkCirc | MarkHook};
    case 0x1ED6: return {'O', MarkCirc | MarkTilde};
    case 0x1ED7: return {'o', MarkCirc | MarkTilde};
    case 0x1ED8: return {'O', MarkDot | MarkCirc};
    case 0x1ED9: return {'o', MarkDot | MarkCirc};
    case 0x1EDA: return {'O', MarkHorn | MarkAcute};
    case 0x1EDB: return {'o', MarkHorn | MarkAcute};
    case 0x1EDC: return {'O', MarkHorn | MarkGrave};
    case 0x1EDD: return {'o', MarkHorn | MarkGrave};
    case 0x1EDE: return {'O', MarkHorn | MarkHook};
    case 0x1EDF: return {'o', MarkHorn | MarkHook};
    case 0x1EE0: return {'O', MarkHorn | MarkTilde};
    case 0x1EE1: return {'o', MarkHorn | MarkTilde};
    case 0x1EE2: return {'O', MarkHorn | MarkDot};
    case 0x1EE3: return {'o', MarkHorn | MarkDot};
    case 0x1EE4: return {'U', MarkDot};
    case 0x1EE5: return {'u', MarkDot};
    case 0x1EE6: return {'U', MarkHook};
    case 0x1EE7: return {'u', MarkHook};
    case 0x1EE8: return {'U', MarkHorn | MarkAcute};
    case 0x1EE9: return {'u', MarkHorn | MarkAcute};
    case 0x1EEA: return {'U', MarkHorn | MarkGrave};
    case 0x1EEB: return {'u', MarkHorn | MarkGrave};
    case 0x1EEC: return {'U', MarkHorn | MarkHook};
    case 0x1EED: return {'u', MarkHorn | MarkHook};
    case 0x1EEE: return {'U', MarkHorn | MarkTilde};
    case 0x1EEF: return {'u', MarkHorn | MarkTilde};
    case 0x1EF0: return {'U', MarkHorn | MarkDot};
    case 0x1EF1: return {'u', MarkHorn | MarkDot};
    case 0x1EF2: return {'Y', MarkGrave};
    case 0x1EF3: return {'y', MarkGrave};
    case 0x1EF4: return {'Y', MarkDot};
    case 0x1EF5: return {'y', MarkDot};
    case 0x1EF6: return {'Y', MarkHook};
    case 0x1EF7: return {'y', MarkHook};
    case 0x1EF8: return {'Y', MarkTilde};
    case 0x1EF9: return {'y', MarkTilde};
    default:
      if (cp < 128) return {static_cast<char>(cp), MarkNone};
      return {'?', MarkNone};
  }
}

template <typename DisplayT>
static inline void drawMarks(DisplayT& d, int x, int y, int size, uint16_t fg, const GlyphInfo& g) {
  const int cw = 6 * size;
  const int ch = 8 * size;
  const int top = y - 2 * size;
  if (g.marks & MarkCirc) {
    d.drawLine(x + size, top + size, x + cw / 2, top - size, fg);
    d.drawLine(x + cw / 2, top - size, x + cw - size, top + size, fg);
  }
  if (g.marks & MarkBreve) {
    d.drawPixel(x + size, top, fg);
    d.drawPixel(x + cw / 2, top - size, fg);
    d.drawPixel(x + cw - size, top, fg);
  }
  if (g.marks & MarkHorn) d.drawLine(x + cw - 1, y + size, x + cw + size, y, fg);
  if (g.marks & MarkAcute) d.drawLine(x + cw / 2, top, x + cw - size, top - 2 * size, fg);
  if (g.marks & MarkGrave) d.drawLine(x + cw / 2, top - 2 * size, x + cw - size, top, fg);
  if (g.marks & MarkHook) {
    d.drawPixel(x + cw / 2, top - 2 * size, fg);
    d.drawPixel(x + cw / 2 + size, top - size, fg);
    d.drawPixel(x + cw / 2, top, fg);
  }
  if (g.marks & MarkTilde) {
    d.drawPixel(x + size, top - size, fg);
    d.drawPixel(x + 2 * size, top - 2 * size, fg);
    d.drawPixel(x + 3 * size, top - size, fg);
    d.drawPixel(x + 4 * size, top - 2 * size, fg);
  }
  if (g.marks & MarkDot) d.fillCircle(x + cw / 2, y + ch + size, size > 1 ? size : 1, fg);
  if (g.marks & MarkStroke) d.drawFastHLine(x + size, y + ch / 2, cw - 2 * size, fg);
}

template <typename DisplayT>
static inline void drawGlyph(DisplayT& d, int x, int y, const GlyphInfo& g, uint16_t fg, uint16_t bg,
                             int size = 1) {
  d.drawChar(x, y, g.base, fg, bg, size);
  drawMarks(d, x, y, size, fg, g);
}

template <typename DisplayT>
static inline void drawWrapped(DisplayT& d, const String& text, int x, int y, int w, int h, uint16_t fg,
                               uint16_t bg, int size = 1) {
  const int charW = 6 * size;
  const int lineH = 10 * size;
  int cx = x;
  int cy = y;
  int i = 0;
  while (i < text.length()) {
    uint32_t cp = 0;
    if (!decodeUtf8(text, i, cp)) break;
    if (cp == '\r') continue;
    if (cp == '\n') {
      cx = x;
      cy += lineH;
      if (cy + lineH > y + h) break;
      continue;
    }
    if (cp == ' ') {
      cx += charW;
      if (cx + charW > x + w) {
        cx = x;
        cy += lineH;
      }
      if (cy + lineH > y + h) break;
      continue;
    }
    if (cx + charW > x + w) {
      cx = x;
      cy += lineH;
    }
    if (cy + lineH > y + h) break;
    drawGlyph(d, cx, cy, mapGlyph(cp), fg, bg, size);
    cx += charW;
  }
}

}  // namespace idk_vn_text
