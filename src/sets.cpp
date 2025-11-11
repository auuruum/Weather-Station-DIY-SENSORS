#include "sets.h"

#include <LittleFS.h>
#include <WiFiConnector.h>

GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett(PROJECT_NAME, &db);

// ========== build ==========
static void build(sets::Builder& b) {
    if (b.Switch(kk::switch_state, "Switch State")) {
        digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);
        
        Serial.print("LED is now ");
        Serial.println(db[kk::switch_state] ? "ON" : "OFF");
        
        db.update();
    }


    if (b.beginMenu("WiFi")) {
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Pass", "");
        if (b.Switch(kk::close_ap, "Close AP")) {
            WiFiConnector.closeAP(db[kk::close_ap]);
        }
        if (b.Button("Connect")) {
            WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
        }
        b.endMenu();
    }
}

// ========== update ==========
static void update(sets::Updater& u) {
   digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);
}

// ========== begin ==========
void sett_begin() {
    // fs
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // database
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::switch_state, false);
    db.init(kk::close_ap, true);

    // wifi
    WiFiConnector.onConnect([]() {
        Serial.print("Connected: ");
        Serial.println(WiFi.localIP());
    });
    WiFiConnector.onError([]() {
        Serial.print("Error. Start AP: ");
        Serial.println(WiFi.softAPIP());
    });

    WiFiConnector.setName(PROJECT_NAME);
    WiFiConnector.closeAP(db[kk::close_ap]);
    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

    // settings
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);

    // configure LED pin and set initial state from saved setting
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);
}

// ========== loop ==========
void sett_loop() {
    WiFiConnector.tick();
    sett.tick();
}