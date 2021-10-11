

#ifndef EMULATORTEST_PPU_H
#define EMULATORTEST_PPU_H
#include <cstdint>
#include "cpu.h"
#include "rom.h"
#include "SFML/Window.hpp"

//#define DRAW_TILE_BOUNDS

class CPU6502;
class RomFile;

typedef struct{
    uint8_t y;
    uint8_t patternTable[2];
    uint8_t attribute;
    uint8_t x;
}sprite_t;

typedef struct{
    uint8_t nameTable;
    uint8_t attrTable;
    uint8_t patternTable[16];
}tile_t;


class PPU{
private:
    sf::Clock clock{};

    CPU6502* cpu;
    RomFile* rom;

    uint16_t vramAddr{0};
    uint16_t tVramAddr{0};
    uint8_t fineXScroll{0};


    uint8_t shiftQuadrant{0};


    uint8_t nameTableTemp{0};
    uint8_t shiftAttrTable[2]{0};
    uint8_t latchAttrTable[2]{0};
    uint8_t shiftAttrTableTemp{0};
    uint8_t shiftPatternTableTemp[2]{0};
    uint16_t shiftPatternTable[2];

    bool isOddFrame{false};

    uint8_t OAM[0x100u]{};
    uint8_t currentOAM{0};
    uint8_t paletteRAM[0x20]{};

    int currentTile{2};
    int tileProgress{0};
    int spriteProgress{0};

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
    bool loadBuffer = true;
    uint8_t oamDMA{0};

    uint64_t ppuTime{0};
    uint32_t scanline{0};
    uint32_t scanCycle{0};

    uint8_t secondaryOAM[64]{};
    uint16_t spriteTileTemp{0};

    uint8_t spriteEvalN{0};
    uint8_t spriteEvalM{0};
    uint8_t spriteEvalProgress{0};
    sprite_t spritesNext[8]{};
    sprite_t sprites[8]{};
    uint8_t numSpritesCurrent{0};
    uint8_t numSpritesNext{0};
    uint8_t spriteFetchCurrent{0};

    uint8_t attrTableTemp{0};
    tile_t tiles[0x20]{};

    uint16_t scanlineColors[256]{};

    void fetchTile(uint8_t offX=0, uint16_t offY=0);
    void evalSprite();
    void fetchSprite();
    void incrementHoriz();
    void incrementY();
    void drawScanline();
    void drawDot();

    uint8_t readPPUMemory8(uint16_t address);
    void writePPUMemory8(uint16_t address, uint8_t arg);
public:


    explicit PPU(CPU6502* cpu, RomFile* rom);

    void cycle(uint64_t numCycles);

    void setPpuCtrl(uint8_t ppuCtrl);

    void setPpuMask(uint8_t ppuMask);

    uint8_t getPpuStatus();

    void setOamAddr(uint8_t oamAddr);

    uint8_t getOamData() const;

    void setOamData(uint8_t oamData);

    void setPpuScroll(uint8_t ppuScroll);

    void setPpuAddr(uint8_t ppuAddr);

    uint8_t getPpuData();

    void setPpuData(uint8_t ppuData);

    void setOamDma(uint8_t addr, uint8_t data);


    void printMemoryDebug(int start, int end);



};

#endif //EMULATORTEST_PPU_H
