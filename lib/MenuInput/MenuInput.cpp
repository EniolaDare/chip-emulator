#include "MenuInput.h"

MenuInput::MenuInput(uint8_t encoderA, uint8_t encoderB, uint8_t select, uint8_t back) : encoderA(encoderA), encoderB(encoderB), select(select), back(back) {}

void MenuInput::init() {
    pinMode(encoderA, INPUT);
    pinMode(encoderB, INPUT);
    pinMode(select, INPUT_PULLUP);
    pinMode(back, INPUT_PULLUP);

    lastEncoderState = (digitalRead(encoderA) << 1) | digitalRead(encoderB);
    encoderDelta = 0;

    selectLastReading = selectStableState = digitalRead(select);
    backLastReading = backStableState = digitalRead(back);

    selectLastChange = backLastChange = millis();
    lastScan = millis();
}

MenuEvent MenuInput::getInputEvent() {
    unsigned long now = millis();
    if ((now - lastScan) < SCAN_INTERVAL) return NONE;
    lastScan = now;

    int8_t encoderState = (digitalRead(encoderA) << 1) | digitalRead(encoderB);
    static int8_t encoderTransitions[16] = {
        0, 1, -1, 0,
        -1, 0, 0, 1,
        1, 0, 0, -1,
        0, -1, 1, 0
    };
    uint8_t transitionIndex = (lastEncoderState << 2) | encoderState;
    encoderDelta += encoderTransitions[transitionIndex];
    lastEncoderState = encoderState;

    if (encoderDelta >= 4) {
        encoderDelta = 0;
        return NEXT;
    }
    if (encoderDelta <= -4) {
        encoderDelta = 0;
        return PREVIOUS;
    }

    bool selectReading = digitalRead(select);
    if (selectReading != selectLastReading) {
        selectLastReading = selectReading;
        selectLastChange = now;
    } else if (selectReading != selectStableState && now - selectLastChange >= DEBOUNCE_TIME) {
        selectStableState = selectReading;
        if (selectStableState == LOW) return SELECT;
    }

    bool backReading = digitalRead(back);
    if (backReading != backLastReading) {
        backLastReading = backReading;
        backLastChange = now;
    } else if (backReading != backStableState && now - backLastChange >= DEBOUNCE_TIME) {
        backStableState = backReading;
        if (backStableState == LOW) return BACK;
    }

    return NONE;
}