#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <vector>

bool sd_init();
bool sd_mounted();
void sd_list_dir(const char* path, std::vector<String>& files);
bool sd_read_file(const char* path, uint8_t* buf, size_t len);
bool sd_write_file(const char* path, const uint8_t* buf, size_t len);

#endif // SDCARD_H