//
// Created by alec on 8/23/2021.
//

#ifndef EMULATORTEST_NESSYSTEM_H
#define EMULATORTEST_NESSYSTEM_H
#include "rom.h"
#include "ppu.h"
#include "memory.h"
#include "cpu.h"
#include <string>

class CPU6502;

class NESSystem {
public:
    CPU6502* cpu;
    CPUMemory* memory;
    PPU* ppu;
    RomFile* rom;
    Controller* controller;

    NESSystem(const std::string& romFile);
};


#endif //EMULATORTEST_NESSYSTEM_H
