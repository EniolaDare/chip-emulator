#include "Chip8Storage.h"

#include <stddef.h>

#include <Arduino.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>


bool listRomPaths(fs::FS &filesystem, const char *directory, std::vector<String> &romPaths) {
    File romDirectory = filesystem.open(directory, FILE_READ);
    if (!romDirectory || !romDirectory.isDirectory()) {
        if (romDirectory) romDirectory.close();
        return false;
    }

    File entry = romDirectory.openNextFile();
    while (entry) {
    if (!entry.isDirectory()) {
        String filename = entry.name();
        const int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) filename = filename.substring(lastSlash + 1);

            String path = directory;
            if (!path.endsWith("/")) path += "/";
        path += filename;

            if (path.length() >= 4 &&
                path.substring(path.length() - 4).equalsIgnoreCase(".ch8")) {
                romPaths.push_back(path);
            }
        }
        entry.close();
        entry = romDirectory.openNextFile();
    }

    romDirectory.close();
    return true;
}

Chip8Storage::Chip8Storage(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi)
    : CS(cs), SCK(sck), MISO(miso), MOSI(mosi) {}

bool Chip8Storage::init() {
    if (sdReady && flashReady) return true;

    if (!sdReady) {
        SPI.begin(SCK, MISO, MOSI, CS);
        sdReady = SD.begin(CS, SPI, 4000000);
        if (!sdReady) {
            Serial.println("SD initialization failed. Check FAT32 format, wiring, and CS pin.");
        }
    }

    if (!flashReady) {
        flashReady = LittleFS.begin(false);
    }
    return sdReady || flashReady;
}

bool Chip8Storage::listRoms(const char *directory, std::vector<String> &romPaths, RomSource source) {
    romPaths.clear();
    if (!init()) return false;

    if (source == SD_CARD) {
        if (!sdReady) return false;
        if (!listRomPaths(SD, directory, romPaths)) {
            Serial.print("SD directory not found or unreadable: ");
            Serial.println(directory);
            return false;
        }
        return true;
    }

    return flashReady && listRomPaths(LittleFS, directory, romPaths);
}
bool Chip8Storage::isReady(RomSource source) {
    init();
    return source == SD_CARD ? sdReady : flashReady;
}
RomLoadResult Chip8Storage::readRom(const char *romPath, size_t destinationCapacity, uint8_t *romBuffer, RomSource romSource) {
    if (!init()) return STORAGE_NOT_READY;

    bool useFlashFilesystem = romSource == FLASH_FILESYSTEM;
    fs::FS *filesystem = useFlashFilesystem ? (fs::FS *)&LittleFS : (fs::FS *)&SD;

    if (useFlashFilesystem && !flashReady) return STORAGE_NOT_READY;
    if (!useFlashFilesystem && !sdReady) return STORAGE_NOT_READY;

    File rom = filesystem->open(romPath, FILE_READ);
    if (!rom) return FILE_NOT_FOUND;

    size_t romSize = rom.size();
    if (rom.isDirectory() || romSize == 0) {
        rom.close();
        return FILE_INVALID;
    }

    if (romSize > destinationCapacity) {
        rom.close();
        return FILE_TOO_LARGE;
    }

    size_t bytesRead = rom.read(romBuffer, romSize);
    rom.close();

    return bytesRead == romSize ? SUCCESS : READ_FAIL;
}