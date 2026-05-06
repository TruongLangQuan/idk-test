#ifndef BATTERY_H
#define BATTERY_H

void battery_init();
void battery_update();
float battery_get_voltage();
int battery_get_percent();
bool battery_is_charging();

#endif // BATTERY_H