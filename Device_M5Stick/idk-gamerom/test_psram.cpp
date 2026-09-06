#include <Arduino.h>
void setup() {
  Serial.begin(115200);
  Serial.printf("PSRAM: %d\n", ESP.getPsramSize());
}
void loop() {}
