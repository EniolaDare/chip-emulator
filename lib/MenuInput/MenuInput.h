#ifndef MENU_INPUT
#define MENU_INPUT

#include <stdint.h>
#include <Arduino.h>

enum MenuEvent {
    NONE,
    NEXT,
    PREVIOUS,
    SELECT,
    BACK
};


class MenuInput {
    public:
        MenuInput(uint8_t encoderA, uint8_t encoderB, uint8_t select, uint8_t back);
        void init();
        MenuEvent getInputEvent();

    private:
        uint8_t encoderA;
        uint8_t encoderB;
        uint8_t select;
        uint8_t back;

        int8_t lastEncoderState = 0;
        int8_t encoderDelta = 0;

        bool selectLastReading = HIGH;
        bool backLastReading = HIGH;

        bool selectStableState = HIGH;
        bool backStableState = HIGH;
        
        unsigned long selectLastChange = 0;
        unsigned long backLastChange = 0;

        const uint8_t SCAN_INTERVAL = 5;
        const uint8_t DEBOUNCE_TIME = 15;
        unsigned long lastScan = 0;
};

#endif