#include "idk_nes.h"

#include <SD.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
#include "nofrendo/nofrendo.h"
}

namespace {

TaskHandle_t g_nes_task = nullptr;
uint8_t* g_rom_blob = nullptr;
size_t g_rom_blob_size = 0;
volatile bool g_active = false;
volatile bool g_exit_requested = false;
String g_rom_name;
String g_last_error;

void freeRomBlob() {
  if (g_rom_blob) {
    heap_caps_free(g_rom_blob);
    g_rom_blob = nullptr;
  }
  g_rom_blob_size = 0;
  g_rom_name = "";
}

void nesTask(void*) {
  nofrendo_main(0, nullptr);
  g_active = false;
  g_nes_task = nullptr;
  vTaskDelete(nullptr);
}

}  // namespace

extern "C" uint8_t* idk_nes_rom_ptr(void) {
  return g_rom_blob;
}

extern "C" size_t idk_nes_rom_size_bytes(void) {
  return g_rom_blob_size;
}

extern "C" const char* idk_nes_rom_name(void) {
  return g_rom_name.c_str();
}

extern "C" int idk_nes_should_exit_flag(void) {
  return g_exit_requested ? 1 : 0;
}

extern "C" void idk_nes_set_last_error(const char* msg) {
  g_last_error = msg ? msg : "";
}

bool idk_nes_begin(const String& path, String& error) {
  idk_nes_end();

  File f = SD.open(path, FILE_READ);
  if (!f) {
    error = "Open failed";
    return false;
  }

  const size_t size = f.size();
  const uint32_t maxKb = idk_nes_max_rom_kb();
  if (((size + 1023) / 1024) > maxKb) {
    f.close();
    error = String("ROM too large for NES core: ") + ((size + 1023) / 1024) + " KB > " + maxKb + " KB";
    return false;
  }

  g_rom_blob = static_cast<uint8_t*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!g_rom_blob) {
    g_rom_blob = static_cast<uint8_t*>(heap_caps_malloc(size, MALLOC_CAP_8BIT));
  }
  if (!g_rom_blob) {
    f.close();
    error = "Out of memory";
    return false;
  }

  const size_t readBytes = f.read(g_rom_blob, size);
  f.close();
  if (readBytes != size) {
    freeRomBlob();
    error = "Read failed";
    return false;
  }

  g_rom_blob_size = size;
  g_rom_name = path;
  g_last_error = "";
  g_exit_requested = false;
  g_active = true;

  BaseType_t ok = xTaskCreatePinnedToCore(nesTask, "nesTask", 24576, nullptr, 1, &g_nes_task, 1);
  if (ok != pdPASS) {
    g_active = false;
    freeRomBlob();
    error = "Failed to start NES task";
    return false;
  }

  error = "";
  return true;
}

bool idk_nes_active() {
  return g_active;
}

void idk_nes_end() {
  if (g_active) {
    g_exit_requested = true;
    main_quit();
    const uint32_t start = millis();
    while (g_active && millis() - start < 2000) {
      delay(10);
    }
    if (g_nes_task) {
      vTaskDelete(g_nes_task);
      g_nes_task = nullptr;
      g_active = false;
    }
  }
  g_exit_requested = false;
  freeRomBlob();
}

uint32_t idk_nes_max_rom_kb() {
  const uint32_t freePsram = ESP.getFreePsram() / 1024;
  if (freePsram == 0) return 768;
  if (freePsram > 4864) return 4096;
  if (freePsram > 1536) return freePsram - 768;
  return freePsram > 256 ? freePsram - 256 : freePsram;
}

const char* idk_nes_last_error() {
  return g_last_error.c_str();
}
