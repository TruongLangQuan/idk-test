#include <Arduino.h>
#include <soc/soc.h>
#include <soc/spi_reg.h>

void setup() {
  Serial.begin(115200);
  Serial.printf("SPI_PORT: %d\n", FSPI);
  Serial.printf("REG_SPI_BASE(FSPI): %x\n", REG_SPI_BASE(FSPI));
}
void loop() {}
