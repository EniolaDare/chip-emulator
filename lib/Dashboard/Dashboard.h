#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "Emulator.h"
#include <ESPAsyncWebServer.h>


class Dashboard {
    public:
        bool start();
        void stop();
        void loop();
        void sendState(EmulatorState &state);

    private:
        bool running = false;

        const char* SSID = "Chip8Emulator";

        AsyncWebServer server{80};
        AsyncWebSocket ws{"/ws"};
};

#endif