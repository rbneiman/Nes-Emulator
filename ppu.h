

#ifndef EMULATORTEST_PPU_H
#define EMULATORTEST_PPU_H

#define VBLANK_TIME 10 //TODO temporary value

#include <cstdint>

extern uint32_t ppuTime;

extern uint8_t OAM[0x100u];

void InitPpu();
uint8_t readPPUMemory8(uint16_t address);
void writePPUMemory8(uint16_t address, uint8_t arg);

#endif //EMULATORTEST_PPU_H
