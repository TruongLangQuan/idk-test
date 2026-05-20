#include "idk_snes.h"

#include <SD.h>
#include <esp_heap_caps.h>

using ArduinoString = String;

extern "C" {
#include "snes9x/snes9x.h"
}

namespace {

static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

bool g_active = false;
bool g_display_ready = false;
uint32_t g_rom_capacity = 0;

void* snes_alloc(size_t size, bool zero) {
  uint32_t caps = MALLOC_CAP_8BIT;
  if (ESP.getPsramSize() > 0) caps |= MALLOC_CAP_SPIRAM;
  void* ptr = heap_caps_malloc(size, caps);
  if (!ptr) ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT);
  if (ptr && zero) memset(ptr, 0, size);
  return ptr;
}

void setDefaultSettings() {
  memset(&Settings, 0, sizeof(Settings));
  Settings.CyclesPercentage = 100;
  Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
  Settings.FrameTimePAL = 20000;
  Settings.FrameTimeNTSC = 16667;
  Settings.ControllerOption = SNES_JOYPAD;
  Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
  Settings.SoundPlaybackRate = 16000;
  Settings.SoundInputRate = 16000;
  Settings.DisableSoundEcho = true;
  Settings.InterpolatedSound = false;
  Settings.Mute = true;
  Settings.APUEnabled = false;
  Settings.NextAPUEnabled = false;
}

bool readRomIntoSnesMemory(const ArduinoString& path, ArduinoString& error) {
  File f = SD.open(path, FILE_READ);
  if (!f) {
    error = "Cannot open ROM";
    return false;
  }

  const size_t romSize = f.size();
  if (romSize < 1024) {
    f.close();
    error = "ROM too small";
    return false;
  }
  if (romSize > g_rom_capacity) {
    f.close();
    error = "Need " + ArduinoString((romSize + 1023) / 1024) + "K RAM";
    return false;
  }

  size_t got = f.read(Memory.ROM, romSize);
  f.close();
  if (got != romSize) {
    error = "ROM read failed";
    return false;
  }

  Memory.ROM_AllocSize = romSize;
  return true;
}

void drawSnesFrame(M5Canvas& canvas) {
  if (!GFX.Screen) return;

  uint16_t line[240];
  const uint16_t* src = reinterpret_cast<const uint16_t*>(GFX.Screen);
  const int srcH = IPPU.RenderedScreenHeight > 0 ? min<int>(IPPU.RenderedScreenHeight, SNES_HEIGHT_EXTENDED) : SNES_HEIGHT;
  const int srcW = SNES_WIDTH;

  for (int y = 0; y < 135; ++y) {
    int sy = (y * srcH) / 135;
    const uint16_t* row = src + sy * srcW;
    for (int x = 0; x < 240; ++x) {
      int sx = (x * srcW) / 240;
      line[x] = row[sx];
    }
    canvas.pushImage(0, y, 240, 1, line);
  }
}

}  // namespace

extern "C" void* idk_snes_malloc(size_t size) {
  return snes_alloc(size, false);
}

extern "C" void* idk_snes_calloc(size_t count, size_t size) {
  if (size && count > SIZE_MAX / size) return nullptr;
  return snes_alloc(count * size, true);
}

extern "C" void idk_snes_free(void* ptr) {
  if (ptr) heap_caps_free(ptr);
}

extern "C" bool S9xInitDisplay(void) {
  GFX.Pitch = SNES_WIDTH * 2;
  GFX.ZPitch = SNES_WIDTH;
  GFX.Screen = static_cast<uint8_t*>(idk_snes_calloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED, 2));
  GFX.SubScreen = static_cast<uint8_t*>(idk_snes_calloc(GFX.Pitch, SNES_HEIGHT_EXTENDED));
  GFX.ZBuffer = static_cast<uint8_t*>(idk_snes_calloc(GFX.ZPitch, SNES_HEIGHT_EXTENDED));
  GFX.SubZBuffer = static_cast<uint8_t*>(idk_snes_calloc(GFX.ZPitch, SNES_HEIGHT_EXTENDED));
  g_display_ready = GFX.Screen && GFX.SubScreen && GFX.ZBuffer && GFX.SubZBuffer;
  return g_display_ready;
}

extern "C" void S9xDeinitDisplay(void) {
  idk_snes_free(GFX.Screen);
  idk_snes_free(GFX.SubScreen);
  idk_snes_free(GFX.ZBuffer);
  idk_snes_free(GFX.SubZBuffer);
  GFX.Screen = nullptr;
  GFX.SubScreen = nullptr;
  GFX.ZBuffer = nullptr;
  GFX.SubZBuffer = nullptr;
  g_display_ready = false;
}

extern "C" uint32_t S9xReadJoypad(int32_t port) {
  if (port != 0) return 0;
  uint32_t keys = 0;
  if (digitalRead(kPinUp) == LOW) keys |= SNES_UP_MASK;
  if (digitalRead(kPinDown) == LOW) keys |= SNES_DOWN_MASK;
  if (digitalRead(kPinLeft) == LOW) keys |= SNES_LEFT_MASK;
  if (digitalRead(kPinRight) == LOW) keys |= SNES_RIGHT_MASK;
  if (digitalRead(kPinCenter) == LOW) keys |= SNES_SELECT_MASK;
  if (M5.BtnPWR.isPressed()) keys |= SNES_START_MASK;
  if (M5.BtnA.isPressed()) keys |= SNES_B_MASK;
  if (M5.BtnB.isPressed()) keys |= SNES_A_MASK;
  if (M5.BtnA.isPressed() && digitalRead(kPinCenter) == LOW) keys |= SNES_Y_MASK;
  if (M5.BtnB.isPressed() && digitalRead(kPinCenter) == LOW) keys |= SNES_X_MASK;
  return keys;
}

extern "C" bool S9xReadMousePosition(int32_t, int32_t*, int32_t*, uint32_t*) {
  return false;
}

extern "C" bool S9xReadSuperScopePosition(int32_t*, int32_t*, uint32_t*) {
  return false;
}

extern "C" bool JustifierOffscreen(void) {
  return true;
}

extern "C" void JustifierButtons(uint32_t*) {
}

extern "C" void S9xToggleSoundChannel(int32_t) {
}

bool idk_snes_begin(const ArduinoString& path, ArduinoString& error) {
  idk_snes_end();
  setDefaultSettings();

  if (!S9xInitDisplay()) {
    error = "SNES display RAM failed";
    idk_snes_end();
    return false;
  }
  if (!S9xInitMemory()) {
    error = "SNES memory failed";
    idk_snes_end();
    return false;
  }

  g_rom_capacity = Memory.ROM_AllocSize;
  if (!readRomIntoSnesMemory(path, error)) {
    idk_snes_end();
    return false;
  }

  if (!LoadROM(nullptr)) {
    error = "SNES ROM init failed";
    idk_snes_end();
    return false;
  }
  if (!S9xInitAPU()) {
    error = "SNES APU failed";
    idk_snes_end();
    return false;
  }
  if (!S9xInitSound(0, 0)) {
    error = "SNES sound init failed";
    idk_snes_end();
    return false;
  }
  if (!S9xInitGFX()) {
    error = "SNES GFX failed";
    idk_snes_end();
    return false;
  }

  S9xSetPlaybackRate(Settings.SoundPlaybackRate);
  S9xReset();
  IPPU.RenderThisFrame = true;
  g_active = true;
  return true;
}

void idk_snes_run_frame(M5Canvas& canvas) {
  if (!g_active) return;
  IPPU.RenderThisFrame = true;
  S9xMainLoop();
  drawSnesFrame(canvas);
}

void idk_snes_end() {
  if (g_active) g_active = false;
  S9xDeinitGFX();
  S9xDeinitAPU();
  S9xDeinitMemory();
  if (g_display_ready) S9xDeinitDisplay();
  g_rom_capacity = 0;
}

bool idk_snes_active() {
  return g_active;
}

uint32_t idk_snes_max_rom_kb() {
  const uint32_t freePsram = ESP.getFreePsram() / 1024;
  if (freePsram < 800) return 0;
  return freePsram - 800; // Subtract ~800KB overhead for SNES9x memory maps & framebuffers
}
