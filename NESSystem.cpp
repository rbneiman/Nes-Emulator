//
// Created by alec on 8/23/2021.
//

#include "NESSystem.h"

NESSystem::NESSystem(const std::string& romFile){
    rom = new RomFile(romFile);
    ppu = new PPU(cpu, rom);
    memory = new CPUMemory(rom, ppu);
    cpu = new CPU6502(memory);
    cpu->loadRom();
}
