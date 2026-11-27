#include "Dashboard.h"

#include <stdint.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>


bool Dashboard::start() {
    if (running) return true;

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0))) return false;
    if (!WiFi.softAP(SSID)) return false;

    server.serveStatic("/", LittleFS, "/dashboard/").setDefaultFile("index.html");
    server.addHandler(&ws);
    server.begin();

    running = true;
    Serial.print("Dashboard IP Adress: http://");
    Serial.println(WiFi.softAPIP());

    return true;
}

void Dashboard::stop() {
    if (!running) return;

    ws.closeAll();
    server.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    running = false;
}

void Dashboard::sendState(EmulatorState &state) {
    if (!running) return;

    JsonDocument doc;
    
    char pixels[CHIP8_WIDTH * CHIP8_HEIGHT + 1];
    size_t pixelIndex = 0;
    for (uint8_t y = 0; y < CHIP8_HEIGHT; ++y) {
        for (uint8_t x = 0; x < CHIP8_WIDTH; ++x) {
            pixels[pixelIndex++] = state.pixels[y][x] ? '1' : '0';
        }
    }
    pixels[pixelIndex] = '\0';

    doc["direction"]["center"] = state.direction.center;
    doc["direction"]["up"] = state.direction.up;
    doc["direction"]["down"] = state.direction.down;
    doc["direction"]["left"] = state.direction.left;
    doc["direction"]["right"] = state.direction.right;

    doc["shake"] = state.shake;
    doc["accelerometer_enabled"] = state.accelerometerEnabled;
    doc["screen"] = pixels;

    for (uint8_t i = 0; i < 16; i++) {
        doc["keys"][i] = state.keys[i];
    }

    String json;
    serializeJson(doc, json);
    ws.textAll(json);
}

void Dashboard::loop() {
    if (!running) return;

    ws.cleanupClients();
}