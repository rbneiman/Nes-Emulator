
#ifndef EMULATORTEST_MEMORY_H
#define EMULATORTEST_MEMORY_H
#include <cstdint>
#include "ppu.h"
#include "rom.h"
#include "input.h"

class PPU;
class CPU6502;

class CPUMemory{
private:
    RomFile* rom;
    PPU* ppu;
    Controller* controller;
    CPU6502* cpu;

    uint8_t memory[0xFFFF]{};

    uint16_t addr;

public:
    explicit CPUMemory(RomFile* rom, PPU* , Controller* controller, CPU6502* cpu);

    void writeDMA(uint8_t arg, int index);
    uint8_t readMemory8(uint16_t address);
    void writeMemory8(uint16_t address, uint8_t arg);
    uint16_t readMemory16(uint16_t address, bool zPage = false);

    void printMemoryDebug(int start, int end);


};




#endif //EMULATORTEST_MEMORY_H
