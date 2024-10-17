#include <Arduino.h>
#include "HeartRateMonitor.h"

HeartRateMonitor hrm;

void setup() {
    hrm.Init();
}

void loop() {
    hrm.Update();
}
