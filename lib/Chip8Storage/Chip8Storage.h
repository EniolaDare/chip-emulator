#ifndef CHIP8_STORAGE_H
#define CHIP8_STORAGE_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <Arduino.h>

enum RomLoadResult {
    SUCCESS,
    READ_FAIL,
    STORAGE_NOT_READY,
    FILE_NOT_FOUND,
    FILE_TOO_LARGE,
    FILE_INVALID,
};

enum RomSource {
    SD_CARD,
    FLASH_FILESYSTEM
};


class Chip8Storage {
    public:
        Chip8Storage(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi);
        bool init();
        RomLoadResult readRom(const char *romPath, size_t destinationCapacity, uint8_t *romBuffer, RomSource romSource = SD_CARD);
        bool listRoms(const char *directory, std::vector<String> &romPaths, RomSource source);
        bool isReady(RomSource source);

    private:
        bool sdReady = false;
        bool flashReady = false;
        uint8_t CS;
        uint8_t SCK;
        uint8_t MISO;
        uint8_t MOSI;
};

#endif