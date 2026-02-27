#include <Arduino.h>

#include "app/app_controller.h"
#include "project_profile_factory.h"

idk::AppController app(makeProjectProfile());

void setup() { app.begin(); }

void loop() { app.loop(); }
