#include <Arduino.h>

#include "sets.h"

#include <DHT.h>

DHT dht11(DHT11_PIN, DHT11);

#include <GTimer.h>

GTimer<millis> dhtTimer(DHT_READ_INTERVAL, true);

void setup() {
    Serial.begin(115200);
    Serial.println();

    sett_begin();

    Serial.println(db[kk::wifi_ssid]);

    Serial.print("SETUP | LED is now ");
    Serial.println(db[kk::switch_state] ? "ON" : "OFF");
}

void loop() {
    if (dhtTimer.tick()) {
        tempC = dht11.readTemperature();
        humidity = dht11.readHumidity();
    }

    sett_loop();
}