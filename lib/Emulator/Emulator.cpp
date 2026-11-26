#include "Emulator.h"

#include <string.h>

#include <Arduino.h>

Emulator::Emulator(Chip8 &chip8, Chip8Display &display, Chip8Keypad &keys, Chip8Storage &storage, Chip8Accel &accel)
    : chip8(chip8),
      display(display),
      keys(keys),
      storage(storage),
      accel(accel) {}

bool Emulator::init(){
    chip8Init(chip8);
    keys.init();
    display.init();
    accelReady = accel.init();

    if (!accelReady) {
        Serial.println("Accelerometer unavailable; continuing without motion input.");
    }

    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    buzzerState = false;

    return true;
}

bool Emulator::start(const char *romPath, RomSource source){
    chip8Init(chip8);
    display.clear();

    if (!storage.init()) return false;

    RomLoadResult loadResult = storage.readRom(romPath, chip8GetCapacity(chip8), chip8GetProgramStart(chip8), source);
    if (loadResult != SUCCESS) {
        Serial.print("ROM load failed: ");
        Serial.println((int)loadResult);
        display.clear();
        return false;
    }

    uint32_t now = micros();

    lastCpuTime = now;
    lastTimerTime = now;
    lastDisplayTime = now;

    memset(physicalKeys, 0, sizeof(physicalKeys));
    memset(accelerometerKeys, 0, sizeof(accelerometerKeys));

    shakeKeyActive = false;
    buzzerBeep();

    return true;
}

void Emulator::stop() {
    noTone(BUZZER_PIN);
    buzzerState = false;

    memset(physicalKeys, 0, sizeof(physicalKeys));
    memset(accelerometerKeys, 0, sizeof(accelerometerKeys));
    
    shakeKeyActive = false;

    chip8Init(chip8);
    display.clear();
}

void Emulator::loop(){
    uint32_t now = micros();

    keys.update(physicalKeys);
    updateAccelerometer();
    updateCombinedKeys();

    uint8_t executed = 0;
    while ( (executed <= MAX_BACKLOG) && ( (now - lastCpuTime) >= CPU_PERIOD_US)){
        chip8Cycle(chip8);
        lastCpuTime += CPU_PERIOD_US;
        executed++;
    }

    if ( ( (now - lastDisplayTime) >= DISPLAY_PERIOD_US) && chip8.draw){
        display.drawChip8(chip8.display);
        chip8.draw = false;
        lastDisplayTime = now;
    }

    if ( (now - lastTimerTime) >= TIMER_PERIOD_US){
        chip8UpdateTimers(chip8);
        lastTimerTime += TIMER_PERIOD_US;
    }

    updateBuzzer();
}

void Emulator::updateCombinedKeys() {
    bool shakePressed = shakeKeyActive && ((millis() - lastShakeTime) < accelKeyMapping.shakePressDurationMs);

    if (!shakePressed) shakeKeyActive = false;

    for (uint8_t key = 0; key < 16; ++key) {
        chip8.keypad[key] = physicalKeys[key] || accelerometerKeys[key];
    }

    if (shakePressed && accelKeyMapping.shake >= 0) {
        chip8.keypad[(uint8_t)accelKeyMapping.shake] = 1;
    }
}

void Emulator::updateAccelerometerKeys() {
    memset(accelerometerKeys, 0, sizeof(accelerometerKeys));

    if (!accelEnabled || !accelReady) {
        accel.wasShaken();
        shakeKeyActive = false;
        return;
    }

    if (!accel.update()) {
        accel.wasShaken();
        shakeKeyActive = false;
        return;
    }

    Direction direction = accel.getDirection();
    if (direction.center && accelKeyMapping.center >= 0) {
        accelerometerKeys[(uint8_t)accelKeyMapping.center] = 1;
    }
    if (direction.up && accelKeyMapping.up >= 0) {
        accelerometerKeys[(uint8_t)accelKeyMapping.up] = 1;
    }
    if (direction.down && accelKeyMapping.down >= 0) {
        accelerometerKeys[(uint8_t)accelKeyMapping.down] = 1;
    }
    if (direction.left && accelKeyMapping.left >= 0) {
        accelerometerKeys[(uint8_t)accelKeyMapping.left] = 1;
    }
    if (direction.right && accelKeyMapping.right >= 0) {
        accelerometerKeys[(uint8_t)accelKeyMapping.right] = 1;
    }

    if (accelKeyMapping.shake >= 0 && accel.wasShaken()) {
        shakeKeyActive = true;
        lastShakeTime = millis();
    } else {
        accel.wasShaken();
    }
}

bool Emulator::isValidMappedKey(int8_t key) {
    return key == -1 || (key >= 0 && key <= 0xF);
}

void Emulator::setAccelerometerEnabled(bool enabled) {
    accelEnabled = enabled;
    
    if (!enabled) {
        memset(accelerometerKeys, 0, sizeof(accelerometerKeys));
        shakeKeyActive = false;
        accel.wasShaken();
    }
}

bool Emulator::isAccelerometerEnabled() {
    return accelEnabled;
}

void Emulator::updateAccelerometer() {
    updateAccelerometerKeys();
}

bool Emulator::setAccelKeyMapping(AccelKeyMapping &mapping) {
    if (!isValidMappedKey(mapping.center) ||
        !isValidMappedKey(mapping.up) ||
        !isValidMappedKey(mapping.down) ||
        !isValidMappedKey(mapping.left) ||
        !isValidMappedKey(mapping.right) ||
        !isValidMappedKey(mapping.shake) ||
        (mapping.shake >= 0 && mapping.shakePressDurationMs == 0)) {
        return false;
    }

    accelKeyMapping = mapping;
    shakeKeyActive = false;
    return true;
}

bool Emulator::setTiltSensitivity(Sensitivity sensitivity) {
    return accel.setTiltSensitivity(sensitivity);
}

bool Emulator::setShakeSensitivity(Sensitivity sensitivity) {
    return accel.setShakeSensitivity(sensitivity);
}

void Emulator::updateBuzzer(){
    bool soundOn = chip8.soundTimer > 0;
    if (buzzerState == soundOn) return;
    
    buzzerState = soundOn;

    if (buzzerState) tone(BUZZER_PIN, BUZZER_FREQUENCY);
    else noTone(BUZZER_PIN);
}

void Emulator::buzzerBeep(uint16_t duration){
    tone(BUZZER_PIN, BUZZER_FREQUENCY);
    delay(duration);
    noTone(BUZZER_PIN);
}

State Emulator::getState() {
    State state;
    memcpy(state.keys, chip8.keypad, sizeof(state.keys));
    state.direction = accel.getDirection();
    state.shake = shakeKeyActive && ((millis() - lastShakeTime) < accelKeyMapping.shakePressDurationMs);
    return state;
}