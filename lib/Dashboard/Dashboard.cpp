#include "Dashboard.h"

#include <stdint.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>


bool Dashboard::start() {
    if (running) return true;

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
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

void Dashboard::sendState(EmulatorState state) {
    if (!running) return;

    JsonDocument doc;

    doc["direction"]["center"] = state.direction.center;
    doc["direction"]["up"] = state.direction.up;
    doc["direction"]["down"] = state.direction.down;
    doc["direction"]["left"] = state.direction.left;
    doc["direction"]["right"] = state.direction.right;

    state.shake ? doc["shake"] = true : doc["shake"] = false;

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