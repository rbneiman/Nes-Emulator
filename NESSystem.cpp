//
// Created by alec on 8/23/2021.
//

#include "NESSystem.h"

NESSystem::NESSystem(const std::string& romFile){
    rom = new RomFile(romFile);
    cpu = new CPU6502();
    ppu = new PPU(cpu, rom);
    memory = new CPUMemory(rom, ppu);
    cpu->setMemory(memory);
    cpu->loadRom();
}
