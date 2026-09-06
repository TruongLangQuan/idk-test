#include "cyd_display_ui.h"

#if IDK_ENABLE_GUI

#include <SPI.h>
#include <TFT_eSPI.h>

namespace idk {
namespace {

TFT_eSPI gTft;

constexpr uint16_t kBg = TFT_BLACK;
constexpr uint16_t kFg = TFT_WHITE;
constexpr uint16_t kAccent = TFT_GREENYELLOW;
constexpr uint16_t kWarn = TFT_ORANGE;

void safeCopy(char* dst, size_t dstSize, const char* src) {
  if (dst == nullptr || dstSize == 0) {
    return;
  }
  if (src == nullptr) {
    dst[0] = '\0';
    return;
  }
  strlcpy(dst, src, dstSize);
}

}  // namespace

void CydDisplayUI::begin(const RuntimeConfig& config) {
  intervalMs_ = (intervalMs_ == 0) ? config.uiUpdateMs : intervalMs_;
  memset(cachedValues_, 0, sizeof(cachedValues_));

  gTft.init();
  gTft.setRotation(1);

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

  gTft.fillScreen(kBg);
  gTft.setTextDatum(TL_DATUM);
  gTft.setTextColor(kFg, kBg);
  gTft.setTextFont(1);
  gTft.setTextSize(1);

  drawLayout(config);
}

void CydDisplayUI::update(const TelemetryState& state) {
  const uint32_t now = millis();
  if (lastDrawMs_ != 0 && (now - lastDrawMs_ < intervalMs_)) {
    return;
  }
  lastDrawMs_ = now;

  char value[64];

  snprintf(value, sizeof(value), "%s  wifi:%d pool:%d", state.coin, state.wifiConnected ? 1 : 0,
           state.poolConnected ? 1 : 0);
  drawField(0, "state", value, state.poolConnected ? kAccent : kWarn);

  snprintf(value, sizeof(value), "%llu", static_cast<unsigned long long>(state.totalHash));
  drawField(1, "total hash", value);

  snprintf(value, sizeof(value), "%.6f", state.bestDiff);
  drawField(2, "best diff", value);

  drawField(3, "pool job", state.poolJob);
  drawField(4, "pool target", state.poolTarget);

  snprintf(value, sizeof(value), "%08lx", static_cast<unsigned long>(state.target32));
  drawField(5, "32-bit target", value);

  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(state.blockFound));
  drawField(6, "block found", value);

  snprintf(value, sizeof(value), "%.2f H/s", state.currentHashrate);
  drawField(7, "current hashrate", value);

  snprintf(value, sizeof(value), "A:%lu R:%lu %s", static_cast<unsigned long>(state.acceptedShares),
           static_cast<unsigned long>(state.rejectedShares), state.status);
  drawField(8, "pool", value, kWarn);
}

void CydDisplayUI::drawLayout(const RuntimeConfig& config) {
  gTft.fillScreen(kBg);

  gTft.setTextColor(kAccent, kBg);
  gTft.setCursor(4, 4);
  gTft.printf("%s", config.projectName);

  gTft.setTextColor(kFg, kBg);
  gTft.setCursor(4, 16);
  gTft.printf("miner: %s  worker: %s", config.minerName, config.workerName);

  gTft.drawFastHLine(0, 28, gTft.width(), TFT_DARKGREY);

  for (uint8_t i = 0; i < kFieldCount; ++i) {
    const int y = 32 + (i * 22);
    gTft.drawFastHLine(0, y + 20, gTft.width(), TFT_DARKGREY);
  }
}

void CydDisplayUI::drawField(uint8_t row, const char* label, const char* value, uint16_t color) {
  if (row >= kFieldCount) {
    return;
  }

  const char* safeValue = (value == nullptr) ? "-" : value;
  if (strncmp(cachedValues_[row], safeValue, kValueMax) == 0) {
    return;
  }

  safeCopy(cachedValues_[row], sizeof(cachedValues_[row]), safeValue);

  const int y = 32 + (row * 22);
  gTft.fillRect(0, y, gTft.width(), 20, kBg);

  gTft.setTextColor(TFT_CYAN, kBg);
  gTft.setCursor(4, y + 2);
  gTft.printf("%s", label);

  gTft.setTextColor(color, kBg);
  gTft.setCursor(132, y + 2);
  gTft.printf("%s", safeValue);
}

}  // namespace idk

#endif  // IDK_ENABLE_GUI
