

#ifndef EMULATORTEST_PPU_H
#define EMULATORTEST_PPU_H
#include <cstdint>
#include "cpu.h"
#include "rom.h"

class CPU6502;
class RomFile;

typedef struct{
    uint8_t y;
    uint8_t tile;
    uint8_t atrib;
    uint8_t x;
}sprite_t;

typedef struct{
    uint8_t nameTable;
    uint8_t attrTable;
    uint8_t patternTable[16];
}tile_t;


class PPU{
private:
    CPU6502* cpu;
    RomFile* rom;

    bool isOddFrame{false};

    uint8_t ppuMem[0x4000u]{};
    uint8_t OAM[0x100u]{};

    int currentTile{0};
    int tileProgress{0};

    uint8_t ppuCtrl{0};
    uint16_t baseNametableAddress{0x2000};
    int vramAddressIncrement{0};
    uint16_t spritePatternTableAddress{0};
    uint16_t bgPatternTableAddress{0};
    bool longSprites{false};
    bool masterSlaveMode{false};
    bool generateNMI{false};

    uint8_t ppuMask{};
    bool grayscale{false};
    bool showBackgroundLeft{false};
    bool showBackground{false};
    bool showSpritesLeft{false};
    bool showSprites{false};
    bool emphasizeRed{false};
    bool emphasizeGreen{false};
    bool emphasizeBlue{false};

    uint8_t ppuStatus{0};
    uint8_t oamAddr{0};
    uint8_t oamData{0};

    bool settingXScroll = true;
    uint8_t xScroll{0};
    uint8_t yScroll{0};

    bool settingPPUAddrMSB = true;
    uint16_t ppuAddr{0};
    uint8_t ppuData{0};
    uint8_t oamDMA{0};

    uint32_t ppuTime{0};
    uint32_t scanline{0};
    uint32_t scanCycle{0};
    uint32_t vBlankTime{0};

    uint16_t vramAddr{0};
    uint16_t tempAddr{0};

    uint8_t fineXScroll{0};

    sprite_t sprites[8]{};

    void fillSprites(int line);
    uint8_t readPPUMemory8(uint16_t address);
    void writePPUMemory8(uint16_t address, uint8_t arg);

    uint8_t nameTableTemp{0};
    uint8_t attrTableTemp{0};
    uint16_t patternTableTemp{0};
    tile_t tiles[0x20]{};

    void drawScanline();
public:


    explicit PPU(CPU6502* cpu, RomFile* rom);

    void cycle(int numCycles);

    void setPpuCtrl(uint8_t ppuCtrl);

    void setPpuMask(uint8_t ppuMask);

    uint8_t getPpuStatus();

    void setOamAddr(uint8_t oamAddr);

    uint8_t getOamData();

    void setOamData(uint8_t oamData);

    void setPpuScroll(uint8_t ppuScroll);

    void setPpuAddr(uint8_t ppuAddr);

    uint8_t getPpuData();

    void setPpuData(uint8_t ppuData);

    void setOamDma(uint8_t oamDma);
};

#endif //EMULATORTEST_PPU_H
