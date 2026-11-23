#include "Chip8Accel.h"

#include <Wire.h>

Chip8Accel::Chip8Accel(uint8_t sda, uint8_t scl) : sda(sda), scl(scl) {}

bool Chip8Accel::init() {
    Wire.begin(sda, scl);
    ready = mma.begin(MMA8451_DEFAULT_ADDRESS, &Wire);
    if (!ready) return false;

    mma.setRange(MMA8451_RANGE_2_G);
    mma.setDataRate(MMA8451_DATARATE_100_HZ);
    lastShake = lastSample = millis();
    return true;
}

bool Chip8Accel::update() {
    if (!ready) return false;

    unsigned long now = millis();
    if (now - lastSample < SAMPLE_INTERVAL) return true;
    lastSample = now;

    sensors_event_t sensorEvent;
    if (!mma.getEvent(&sensorEvent)) {
        clearDirection();
        shakeSamples = 0;
        shakePending = false;

        return false;
    }

    x = sensorEvent.acceleration.x;
    y = sensorEvent.acceleration.y;
    z = sensorEvent.acceleration.z;

    xDirection = updateAxis(x, xDirection);
    yDirection = updateAxis(y, yDirection);
    
    float magnitude = sqrtf(x*x + y*y + z*z);
    float motion = fabsf(magnitude - GRAVITY);

    if (motion < shakeReleaseThreshold) {
        shakeReady = true;
        shakeSamples = 0;

    } else if (motion >= shakeThreshold) {
        if (shakeSamples < SHAKE_SAMPLES_REQUIRED) ++shakeSamples;

        if (shakeReady && (shakeSamples >= SHAKE_SAMPLES_REQUIRED) && (now - lastShake >= SHAKE_COOLDOWN)) {
            shakePending = true;
            shakeReady = false;
            lastShake = now;
            shakeSamples = 0;
        }

    } else shakeSamples = 0;

    return true;
}

int8_t Chip8Accel::updateAxis(float value, int8_t currentDirection) {
    if (currentDirection > 0) {
        if (value <= -tiltEnterThreshold) return -1;
        return value < tiltReleaseThreshold ? 0 : 1;
    }

    if (currentDirection < 0) {
        if (value >= tiltEnterThreshold) return 1;
        return value > -tiltReleaseThreshold ? 0 : -1;
    }

    if (value >= tiltEnterThreshold) return 1;
    if (value <= -tiltEnterThreshold) return -1;

    return 0;
}

Direction Chip8Accel::getDirection() {
    Direction direction;

    direction.left = xDirection > 0;
    direction.right = xDirection < 0;
    direction.up = yDirection < 0;
    direction.down = yDirection > 0;

    direction.center = !direction.left && !direction.right && !direction.up && !direction.down;

    return direction;
}

bool Chip8Accel::wasShaken() {
    bool detected = shakePending;
    shakePending = false;
    return detected;
}

bool Chip8Accel::setTiltSensitivity(Sensitivity sensitivity) {
    switch (sensitivity) {
        case LESS_SENSITIVE:
            tiltEnterThreshold = 2.0f;
            tiltReleaseThreshold = 1.4f;
            break;

        case NORMAL:
            tiltEnterThreshold = 1.2f;
            tiltReleaseThreshold = 0.8f;
            break;

        case MORE_SENSITIVE:
            tiltEnterThreshold = 0.6f;
            tiltReleaseThreshold = 0.4f;
            break;

        default:
            return false;
    }

    clearDirection();
    return true;
}
bool Chip8Accel::setShakeSensitivity(Sensitivity sensitivity) {
    switch (sensitivity) {
        case LESS_SENSITIVE:
            shakeThreshold = 2.8f;
            shakeReleaseThreshold = 1.1f;
            break;

        case NORMAL:
            shakeThreshold = 1.8f;
            shakeReleaseThreshold = 0.7f;
            break;

        case MORE_SENSITIVE:
            shakeThreshold = 0.9f;
            shakeReleaseThreshold = 0.35f;
            break;

        default:
            return false;
    }

    shakeSamples = 0;
    shakePending = false;
    shakeReady = true;

    return true;
}
void Chip8Accel::clearDirection() {
    xDirection = 0;
    yDirection = 0;
}
