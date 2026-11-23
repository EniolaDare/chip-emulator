#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>


struct Chip8 {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint16_t PC;
    uint8_t SP;
    uint16_t stack[16];
    uint8_t keypad[16];
    uint8_t display[32][64];
    bool draw;
    uint16_t opcode;
};

void chip8Reset(Chip8 &);
void chip8LoadFont(Chip8 &);
void chip8Init(Chip8 &);
void chip8LoadTest(Chip8 &);
bool chip8LoadTestRom(Chip8 &, const char *);
void chip8Execute(Chip8 &);
void chip8Cycle(Chip8 &);
void chip8UpdateTimers(Chip8 &);
void chip8Draw(Chip8 &, uint8_t, uint8_t, uint8_t);
uint8_t chip8GetRandom();

size_t chip8GetCapacity(Chip8 &);
uint8_t *chip8GetProgramStart(Chip8 &);

#endif