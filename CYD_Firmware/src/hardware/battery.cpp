#include "battery.h"
#include "../config.h"

static float current_voltage = 0.0f;
static int current_percent = 0;

void battery_init() {
    pinMode(BATTERY_ADC, INPUT);
    battery_update();
}

void battery_update() {
    int raw = analogRead(BATTERY_ADC);
    current_voltage = (raw / 4095.0f) * 3.3f * 2.0f; // 2:1 divider
    
    // mapping 3.0V -> 0%, 4.2V -> 100%
    if (current_voltage >= 4.2f) current_percent = 100;
    else if (current_voltage <= 3.0f) current_percent = 0;
    else current_percent = (int)((current_voltage - 3.0f) / 1.2f * 100.0f);
}

float battery_get_voltage() {
    return current_voltage;
}

int battery_get_percent() {
    return current_percent;
}

bool battery_is_charging() {
    // Basic detection: if voltage is rapidly rising or > 4.3V (USB plugged in).
    // CYD lacks a dedicated charging pin.
    return current_voltage > 4.3f;
}