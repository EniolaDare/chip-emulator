#ifndef CHIP8_ACCEL_H
#define CHIP8_ACCEL_H

#include <Arduino.h>
#include <Adafruit_MMA8451.h>

struct Direction {
    bool center = true;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

enum Sensitivity {
    LESS_SENSITIVE,
    NORMAL,
    MORE_SENSITIVE
};

class Chip8Accel {
    public:
        Chip8Accel(uint8_t sda, uint8_t scl);
        bool init();
        bool update();
        Direction getDirection();
        bool wasShaken();
        bool setTiltSensitivity(Sensitivity sensitivity);
        bool setShakeSensitivity(Sensitivity sensitivity);

    private:
        int8_t updateAxis(float value, int8_t currentDirection);
        void clearDirection();

        Adafruit_MMA8451 mma;
        uint8_t sda;
        uint8_t scl;
        bool ready = false;

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        int8_t xDirection = 0;
        int8_t yDirection = 0;

        bool shakeReady = true;
        bool shakePending = false;

        uint8_t shakeSamples = 0;
        uint32_t lastShake = 0;
        uint32_t lastSample = 0;
        
        float tiltEnterThreshold = 2.5f;
        float tiltReleaseThreshold = 1.8f;
        float shakeThreshold = 3.0f;
        float shakeReleaseThreshold = 1.2f;

        const uint32_t SAMPLE_INTERVAL = 20;
        const uint32_t SHAKE_COOLDOWN = 500;
        const float GRAVITY = 9.80665;

        const uint8_t SHAKE_SAMPLES_REQUIRED = 2;
};

#endif
