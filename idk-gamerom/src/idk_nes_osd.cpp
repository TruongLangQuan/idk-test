#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "idk_nes.h"

extern "C" {
#include "nofrendo/bitmap.h"
#include "nofrendo/event.h"
#include "nofrendo/gui.h"
#include "nofrendo/log.h"
#include "nofrendo/nofconfig.h"
#include "nofrendo/nofrendo.h"
#include "nofrendo/noftypes.h"
#include "nofrendo/osd.h"
#include "nofrendo/vid_drv.h"
#include "nofrendo/nes/nes.h"
#include "nofrendo/nes/nesinput.h"

uint8_t* idk_nes_rom_ptr(void);
size_t idk_nes_rom_size_bytes(void);
const char* idk_nes_rom_name(void);
int idk_nes_should_exit_flag(void);
void idk_nes_set_last_error(const char* msg);
}

namespace {

constexpr int kPinUp = 32;
constexpr int kPinDown = 33;
constexpr int kPinLeft = 25;
constexpr int kPinRight = 26;
constexpr int kPinCenter = 0;
constexpr int kOutW = 240;
constexpr int kOutH = 135;
constexpr int kSrcW = 256;
constexpr int kSrcH = NES_VISIBLE_HEIGHT;

TimerHandle_t g_timer = nullptr;
void (*g_timer_func)(void) = nullptr;
uint16_t g_palette[256];
uint16_t g_line[kOutW];
uint16_t g_xmap[kOutW];
uint16_t g_ymap[kOutH];
uint8_t* g_dummy_fb = nullptr;
uint16_t g_prev_mask = 0;
bool g_quit_latched = false;

void timerThunk(TimerHandle_t) {
  if (g_timer_func) g_timer_func();
}

void ensureMaps() {
  static bool ready = false;
  if (ready) return;
  for (int x = 0; x < kOutW; ++x) {
    g_xmap[x] = (x * kSrcW) / kOutW;
  }
  for (int y = 0; y < kOutH; ++y) {
    g_ymap[y] = (y * kSrcH) / kOutH;
  }
  ready = true;
}

uint16_t readMask() {
  uint16_t mask = 0;
  if (M5.BtnA.isPressed()) mask |= 0x01;       // A
  if (M5.BtnB.isPressed()) mask |= 0x02;       // B
  if (digitalRead(kPinCenter) == LOW) mask |= 0x04;  // Select
  if (M5.BtnPWR.isPressed()) mask |= 0x08;     // Start
  if (digitalRead(kPinUp) == LOW) mask |= 0x10;
  if (digitalRead(kPinDown) == LOW) mask |= 0x20;
  if (digitalRead(kPinLeft) == LOW) mask |= 0x40;
  if (digitalRead(kPinRight) == LOW) mask |= 0x80;
  return mask;
}

void dispatchEvent(int eventId, bool pressed) {
  event_t handler = event_get(eventId);
  if (handler) handler(pressed ? INP_STATE_MAKE : INP_STATE_BREAK);
}

void renderFrame(bitmap_t* bmp) {
  if (!bmp) return;
  M5.Display.startWrite();
  for (int y = 0; y < kOutH; ++y) {
    const uint8_t* src = bmp->line[g_ymap[y]];
    for (int x = 0; x < kOutW; ++x) {
      g_line[x] = g_palette[src[g_xmap[x]]];
    }
    M5.Display.pushImage(0, y, kOutW, 1, g_line);
  }
  M5.Display.endWrite();
}

int logPrint(const char* s) {
  Serial.print(s);
  return 0;
}

int driverInit(int, int) { return 0; }
void driverShutdown(void) {}
int driverSetMode(int, int) { return 0; }
void driverSetPalette(rgb_t* pal) {
  for (int i = 0; i < 256; ++i) {
    g_palette[i] = M5.Display.color565(pal[i].r, pal[i].g, pal[i].b);
  }
}
void driverClear(uint8 color) {
  const uint16_t rgb = g_palette[color];
  M5.Display.fillScreen(rgb);
}
bitmap_t* driverLockWrite(void) {
  if (!g_dummy_fb) {
    g_dummy_fb = static_cast<uint8_t*>(malloc(kSrcW * kSrcH));
  }
  return bmp_createhw(g_dummy_fb, kSrcW, kSrcH, kSrcW);
}
void driverFreeWrite(int, rect_t*) {}
void driverCustomBlit(bitmap_t* bmp, int, rect_t*) {
  renderFrame(bmp);
}

viddriver_t g_driver = {
    "M5StickCPlus2",
    driverInit,
    driverShutdown,
    driverSetMode,
    driverSetPalette,
    driverClear,
    driverLockWrite,
    driverFreeWrite,
    driverCustomBlit,
    false,
};

}  // namespace

extern "C" char* osd_getromdata() {
  return reinterpret_cast<char*>(idk_nes_rom_ptr());
}

extern "C" int osd_main(int, char**) {
  const char* name = idk_nes_rom_name();
  return main_loop((name && *name) ? name : "rom.nes", system_nes);
}

extern "C" void osd_getvideoinfo(vidinfo_t* info) {
  info->default_width = kSrcW;
  info->default_height = kSrcH;
  info->driver = &g_driver;
}

extern "C" void osd_getsoundinfo(sndinfo_t* info) {
  info->sample_rate = 0;
  info->bps = 0;
}

extern "C" int osd_init(void) {
  log_chain_logfunc(logPrint);
  ensureMaps();
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);
  M5.Display.fillScreen(TFT_BLACK);
  g_prev_mask = 0;
  g_quit_latched = false;
  return 0;
}

extern "C" void osd_shutdown(void) {
  if (g_timer) {
    xTimerStop(g_timer, 0);
    xTimerDelete(g_timer, 0);
    g_timer = nullptr;
  }
  if (g_dummy_fb) {
    free(g_dummy_fb);
    g_dummy_fb = nullptr;
  }
}

extern "C" int osd_installtimer(int frequency, void* func, int, void*, int) {
  if (g_timer) {
    xTimerStop(g_timer, 0);
    xTimerDelete(g_timer, 0);
    g_timer = nullptr;
  }
  g_timer_func = reinterpret_cast<void (*)()>(func);
  TickType_t period = pdMS_TO_TICKS((frequency > 0) ? (1000 / frequency) : 16);
  if (period < 1) period = 1;
  g_timer = xTimerCreate("nesTimer", period, pdTRUE, nullptr, timerThunk);
  if (!g_timer) return -1;
  return xTimerStart(g_timer, 0) == pdPASS ? 0 : -1;
}

extern "C" void osd_fullname(char* fullname, const char* shortname) {
  strncpy(fullname, shortname, PATH_MAX);
  fullname[PATH_MAX - 1] = '\0';
}

extern "C" char* osd_newextension(char* string, char*) {
  return string;
}

extern "C" int osd_makesnapname(char*, int) {
  return -1;
}

extern "C" void osd_setsound(void (*)(void*, int)) {}

extern "C" void osd_getinput(void) {
  M5.update();

  const bool quitCombo = M5.BtnPWR.isPressed() && digitalRead(kPinCenter) == LOW;
  if (idk_nes_should_exit_flag() || quitCombo) {
    if (!g_quit_latched) {
      dispatchEvent(event_quit, true);
      g_quit_latched = true;
    }
  } else {
    g_quit_latched = false;
  }

  const uint16_t mask = readMask();
  const uint16_t changed = mask ^ g_prev_mask;
  static const int events[8] = {
      event_joypad1_a,
      event_joypad1_b,
      event_joypad1_select,
      event_joypad1_start,
      event_joypad1_up,
      event_joypad1_down,
      event_joypad1_left,
      event_joypad1_right,
  };

  for (int i = 0; i < 8; ++i) {
    const uint16_t bit = (1u << i);
    if (changed & bit) {
      dispatchEvent(events[i], (mask & bit) != 0);
    }
  }
  g_prev_mask = mask;
}

extern "C" void osd_getmouse(int* x, int* y, int* button) {
  if (x) *x = 0;
  if (y) *y = 0;
  if (button) *button = 0;
}
