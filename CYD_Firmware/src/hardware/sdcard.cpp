#include "sdcard.h"
#include "../config.h"
#include <SPI.h>
#include <SD.h>

static bool is_mounted = false;
bool sd_init() {
    // SPI bus is already initialized in ui_init
    if (!SD.begin(SD_CS, SPI)) {
        DEBUG_PRINTLN("SD Mount Failed");
        is_mounted = false;
        return false;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        DEBUG_PRINTLN("No SD card attached");
        is_mounted = false;
        return false;
    }
    is_mounted = true;
    
    // Create necessary directories
    if (!SD.exists("/media")) {
        SD.mkdir("/media");
    }
    if (!SD.exists("/config")) {
        SD.mkdir("/config");
    }
    
    return true;
}

bool sd_mounted() {
    return is_mounted;
}

void sd_list_dir(const char* path, std::vector<String>& files) {
    if (!is_mounted) return;
    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) return;
    
    File file = dir.openNextFile();
    int count = 0;
    while (file && count < 32) {
        if (!file.isDirectory()) {
            files.push_back(String(file.name()));
            count++;
        }
        file = dir.openNextFile();
    }
}

bool sd_read_file(const char* path, uint8_t* buf, size_t len) {
    if (!is_mounted) return false;
    File file = SD.open(path, FILE_READ);
    if (!file) return false;
    size_t bytesRead = file.read(buf, len);
    file.close();
    return bytesRead > 0;
}

bool sd_write_file(const char* path, const uint8_t* buf, size_t len) {
    if (!is_mounted) return false;
    File file = SD.open(path, FILE_WRITE);
    if (!file) return false;
    size_t bytesWritten = file.write(buf, len);
    file.close();
    return bytesWritten == len;
}