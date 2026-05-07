#include <Arduino.h>

#include "Forecaster.h"
Forecaster cond;

#include "sets.h"

#include <DHT.h>

DHT dht11(DHT11_PIN, DHT11);

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

#include <GTimer.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

static AsyncWebServer server(API_PORT);

GTimer<millis> SensorTimer(SENSOR_READ_INTERVAL, true);
GTimer<millis> ForecasterTimer(FORECASTER_INTERVAL_MS, true);

static void printI2CDevices() {
    Serial.println("Scanning I2C bus...");

    bool found = false;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C device found at 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
            found = true;
        }
    }

    if (!found) {
        Serial.println("No I2C devices found");
    }
}

static bool beginBmp280() {
    if (bmp.begin(BMP280_PRIMARY_ADDRESS)) {
        Serial.print("BMP280 sensor found at 0x");
        Serial.println(BMP280_PRIMARY_ADDRESS, HEX);
        return true;
    }

    if (bmp.begin(BMP280_SECONDARY_ADDRESS)) {
        Serial.print("BMP280 sensor found at 0x");
        Serial.println(BMP280_SECONDARY_ADDRESS, HEX);
        return true;
    }

    printI2CDevices();
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    sett_begin();

    server.begin();

    Wire.begin(BMP280_SDA_PIN, BMP280_SCL_PIN);

    cond.setH(LOCATION_ALTITUDE);

    Serial.println(db[kk::wifi_ssid]);

    Serial.print("SETUP | LED is now ");
    Serial.println(db[kk::switch_state] ? "ON" : "OFF");

    if (!MDNS.begin(MDNS_ADDRESS)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
        delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    dht11.begin();

    if (!beginBmp280()) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        while (1);
    }

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,  
                    Adafruit_BMP280::STANDBY_MS_500);

    delay(2000);

    // Should make refactor in furure and put to separate function for cleaner code
    tempC = bmp.readTemperature();
    humidity = dht11.readHumidity();
    pressure = bmp.readPressure() / 100.0F;
    if (!isnan(tempC) && !isnan(pressure)) {
        Serial.println("initial forecast sensor read successful");
        cond.addP(pressure * 100.0F, tempC);
    } else {
        Serial.println("initial sensor read failed, skipping forecast");
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "API is online. Use " SENSOR_STATION_API_PATH);
    });

    server.on(SENSOR_STATION_API_PATH, HTTP_GET, [](AsyncWebServerRequest *request){
        if (isnan(tempC) || isnan(humidity) || isnan(pressure)) {
            request->send(500, "text/plain", "Failed to read from sensor");
            return;
        }

        String json = "{";
        json += "\"temp\": " + String(tempC) + ",";
        json += "\"humidity\": " + String(humidity) + ",";
        json += "\"pressure\": " + String(pressure) + ",";
        json += "\"cast\": " + String(cond.getCast());
        json += "}";

        request->send(200, "application/json", json);
    });
}

void loop() {
    if (SensorTimer.tick()) {
        tempC = bmp.readTemperature();
        humidity = dht11.readHumidity();
        pressure = bmp.readPressure() / 100.0F;
    }
    if (ForecasterTimer.tick()) {
        cond.addP(pressure * 100.0F, tempC);
        Serial.print("forecast: ");
        Serial.println(cond.getCast());
    }

    sett_loop();
}
