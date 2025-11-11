#include <Arduino.h>

#include "sets.h"

void setup() {
    Serial.begin(115200);
    Serial.println();

    sett_begin();

    Serial.println(db[kk::wifi_ssid]);

    Serial.print("SETUP | LED is now ");
    Serial.println(db[kk::switch_state] ? "ON" : "OFF");
}

void loop() {
    sett_loop();
}