#include "Chip8.h"

#include <stddef.h>
#include <string.h>

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>


#define NNN 0x0FFF
#define NN 0x00FF
#define N 0x000F
#define X 0x0F00
#define Y 0x00F0
#define CMD 0xF000

#define FONT_START_ADDR 0x050
#define PROGRAM_START_ADDR 0x200
#define MEMORY_BYTES 4096


// Test roms
const uint8_t IBM[] = {
    0x00, 0xE0, 0xA2, 0x2A, 0x60, 0x0C, 0x61, 0x08,
    0xD0, 0x1F, 0x70, 0x09, 0xA2, 0x39, 0xD0, 0x1F,
    0xA2, 0x48, 0x70, 0x08, 0xD0, 0x1F, 0x70, 0x04,
    0xA2, 0x57, 0xD0, 0x1F, 0x70, 0x08, 0xA2, 0x66,
    0xD0, 0x1F, 0x70, 0x08, 0xA2, 0x75, 0xD0, 0x1F,
    0x12, 0x28, 0xFF, 0x00, 0xFF, 0x00, 0x3C, 0x00,
    0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0xFF, 0x00,
    0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x38, 0x00, 0x3F,
    0x00, 0x3F, 0x00, 0x38, 0x00, 0xFF, 0x00, 0xFF,
    0x80, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0x80, 0x00,
    0x80, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0x80, 0xF8,
    0x00, 0xFC, 0x00, 0x3E, 0x00, 0x3F, 0x00, 0x3B,
    0x00, 0x39, 0x00, 0xF8, 0x00, 0xF8, 0x03, 0x00,
    0x07, 0x00, 0x0F, 0x00, 0xBF, 0x00, 0xFB, 0x00,
    0xF3, 0x00, 0xE3, 0x00, 0x43, 0xE0, 0x00, 0xE0,
    0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
    0x00, 0xE0, 0x00, 0xE0
};
const uint8_t DRAW[] = {
  0xA2, 0x0C,
  0x60, 0x00,
  0x61, 0x00, 
  0xD0, 0x11,
  0x12, 0x0A,
  0x12, 0x0A,
  0x80 
};
const uint8_t KEY[] = {
    0x00, 0xE0, 
    0xF0, 0x0A, 
    0xF0, 0x29, 
    0xD0, 0x15, 
    0x61, 0x00, 
    0x71, 0x01, 
    0x31, 0x00,
    0x12, 0x0A, 
    0x00, 0xE0,
    0xE0, 0xA1,
    0x12, 0x12, 
    0x12, 0x00 
};

const uint8_t FONTSET[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8Reset(Chip8 &c) {
    memset(&c, 0, sizeof(Chip8));
    c.PC = PROGRAM_START_ADDR;
}
void chip8LoadFont(Chip8 &c) {
    memcpy(&c.memory[FONT_START_ADDR], FONTSET, sizeof(FONTSET));
}
void chip8Init(Chip8 &c) {
    chip8Reset(c);
    chip8LoadFont(c);
}
void chip8UpdateTimers(Chip8 &c) {
    if (c.delayTimer > 0) c.delayTimer--;
    if (c.soundTimer > 0) c.soundTimer--;
}
uint8_t chip8GetRandom() {
    return (uint8_t)random(256);
}
void chip8Cycle(Chip8 &c) {
    c.opcode = (c.memory[c.PC] << 8) | c.memory[c.PC + 1];
    c.PC += 2;
    chip8Execute(c);
}
void chip8LoadTest(Chip8 &c) {
    memcpy(&c.memory[PROGRAM_START_ADDR], IBM, sizeof(IBM));
}

bool chip8LoadTestRom(Chip8 &c, const char *path) {
    static bool filesystemMounted = false;
    if (!filesystemMounted) {
        filesystemMounted = LittleFS.begin(false);
        if (!filesystemMounted) return false;
    }

    File rom = LittleFS.open(path, FILE_READ);
    if (!rom) return false;

    size_t size = rom.size();
    if (size == 0 || size > chip8GetCapacity(c) || rom.isDirectory()) {
        rom.close();
        return false;
    }

    chip8Init(c);
    size_t bytes = rom.read(chip8GetProgramStart(c), size);
    rom.close();

    if (bytes != size) {
        chip8Init(c);
        return false;
    }

    return true;
}
void chip8Draw(Chip8 &c, uint8_t vx, uint8_t vy, uint8_t n) {
    c.V[0xF] = 0; // reset collision flag
    
    for (uint8_t i = 0; i < n; i++) {
        uint8_t byte = c.memory[c.I + i];

        for (uint8_t j = 0; j < 8; j++) {
            if (!(byte & (0x80 >> j))) continue;

            uint8_t displayX = (vx + j) % 64;
            uint8_t displayY = (vy + i) % 32;

            c.display[displayY][displayX] ^= 1; // toggle pixel

            if (!c.display[displayY][displayX]) c.V[0xF] = 1;
        }
    }

    c.draw = true;
}
void chip8Execute(Chip8 &c) {
    uint16_t nnn = c.opcode & NNN;
    uint8_t nn = c.opcode & NN;
    uint8_t n = c.opcode & N;

    uint8_t x = (c.opcode & X) >> 8;
    uint8_t y = (c.opcode & Y) >> 4;

    uint8_t vx = c.V[x];
    uint8_t vy = c.V[y];

    switch (c.opcode & CMD){
        case 0x0000:
            if (nn == 0x00E0){
                memset(&c.display, 0, sizeof(c.display));
                c.draw = true;
            }
            if (nn == 0x00EE) c.PC = c.stack[c.SP--];

            break;

        case 0x1000:
            c.PC = nnn;
            break;

        case 0x2000:
            c.stack[++c.SP] = c.PC;
            c.PC = nnn;
            break;
        
        case 0x3000:
            if (vx == nn) c.PC += 2;
            break;

        case 0x4000:
            if (vx != nn) c.PC += 2;
            break;

        case 0x5000:
            if (vx == vy) c.PC += 2;
            break;

        case 0x6000:
            c.V[x] = nn;
            break;

        case 0x7000:
            c.V[x] += nn;
            break;

        case 0x8000:
            switch (n){
                case 0:
                    c.V[x] = vy;
                    break;
                case 1:
                    c.V[x] = vy | vx;
                    break;
                case 2:
                    c.V[x] = vy & vx;
                    break;
                case 3:
                    c.V[x] = vy ^ vx;
                    break;
                case 4:
                    c.V[x] += vy;
                    c.V[0xF] = (vx + vy) > 0xFF;
                    break;
                case 5:
                    c.V[x] = vx - vy;
                    c.V[0xF] = (vx >= vy);
                    break;
                case 6:
                    c.V[x] >>= 1;
                    c.V[0xF] = (vx & 1);
                    break;
                case 7:
                    c.V[x] = vy - vx;
                    c.V[0xF] = (vy >= vx);
                    break;
                case 0xE:
                    c.V[x] <<= 1;
                    c.V[0xF] = ((1 << 7) & vx) ? 1 : 0;
                    break;
            }
            break;

        case 0x9000:
            if (vx != vy) c.PC += 2;
            break;

        case 0xA000:
            c.I = nnn;
            break;

        case 0xB000:
            c.PC = nnn + c.V[0x0];
            break;

        case 0xC000:
            c.V[x] =  chip8GetRandom() & nn;
            break;

        case 0xD000:
            chip8Draw(c, vx, vy, n);
            break;

        case 0xE000:
            if (n == 0x1){
                if (!c.keypad[vx]) c.PC += 2;
            }
            if (n == 0xE){
                if (c.keypad[vx]) c.PC += 2;
            }
            break;

        case 0xF000:
            switch (nn){
                case 0x07: 
                    c.V[x] = c.delayTimer;
                    break;
                case 0x0A: {
                    bool pressed = false;

                    for (uint8_t i = 0; i < 16; i++){
                        if (!c.keypad[i]) continue;

                        c.V[x] = i;
                        pressed = true;
                    }

                    if (!pressed) c.PC -= 2;
                    break;
                }
                case 0x15:
                    c.delayTimer = vx;
                    break;
                case 0x18:
                    c.soundTimer = vx;
                    break;
                case 0x1E:
                    c.I += vx;
                    break;
                case 0x29:
                    c.I = FONT_START_ADDR + (vx * 5);
                    break;
                case 0x33:
                    c.memory[c.I] = c.V[x] / 100;
                    c.memory[c.I + 1] = (c.V[x] / 10) % 10;
                    c.memory[c.I + 2] = c.V[x] % 10;
                    break;
                case 0x55:
                    for (uint8_t i = 0; i <= x; i++) c.memory[c.I + i] = c.V[i];
                    break;
                case 0x65:
                    for (uint8_t i = 0; i <= x; i++) c.V[i] = c.memory[c.I + i];
                    break;
            }
            break;
        
        default:
            break;
    }
}

size_t chip8GetCapacity(Chip8 &c){
    return (MEMORY_BYTES - PROGRAM_START_ADDR);
}
uint8_t* chip8GetProgramStart(Chip8 &c){
    return &c.memory[PROGRAM_START_ADDR];
}