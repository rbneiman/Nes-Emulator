
#ifndef EMULATORTEST_MEMORY_H
#define EMULATORTEST_MEMORY_H
#include <cstdint>
#include "cpu.h"
#include "rom.h"
//#include "cpu.h"

class CPU6502;

class CPUMemory{
private:
    CPU6502* cpu;
    RomFile* rom;

    uint8_t memory[0xFFFF]{};

    uint16_t scroll;
    uint16_t scrollTemp;
    uint16_t addrTemp;
    uint16_t addr;

public:
    explicit CPUMemory(CPU6502* cpu);

    uint8_t readMemory8(uint16_t address);
    void writeMemory8(uint16_t address, uint8_t arg);
    uint16_t readMemory16(uint16_t address);
};




#endif //EMULATORTEST_MEMORY_H
