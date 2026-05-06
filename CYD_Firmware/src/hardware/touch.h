#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>

void touch_init();
void touch_update();

// Callbacks
void set_on_swipe_left(void (*cb)());
void set_on_swipe_right(void (*cb)());
void set_on_tap_sd(void (*cb)());
void set_on_double_tap(void (*cb)());
void set_on_long_press(void (*cb)());

#endif // TOUCH_H