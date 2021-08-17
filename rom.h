//
// Created by Alec on 8/3/2021.
//

#ifndef EMULATORTEST_ROM_H
#define EMULATORTEST_ROM_H
#include <string>
#include <vector>
#include "utilities.h"

typedef enum{
    HORIZONTAL = 0,
    VERTICAL = 1
}mirror_type_t;

typedef enum{
    NTSC = 0,
    PAL = 1
}tv_system;

typedef enum{
    NES = 0,
    VS_SYSTEM = 1,
    PLAYCHOICE10 = 2,
    EXTENDED = 3
}console_type_t;

class RomFile{
private:
    std::vector<char> vram;

    std::vector<char> trainer;
    std::vector<char> prg_rom;
    std::vector<char> chr_rom;
    std::vector<char> inst_rom;
    std::vector<char> misc_rom;
    std::string title;

    mirror_type_t mirror_type;
    bool has_persistent;
    bool has_trainer;
    bool ignore_mirror;
    console_type_t consoleType;

    bool nes20;
    int mapperNum;

    ErrorMessage initNES10(const std::vector<char> &contents);
    ErrorMessage initNES20(const std::vector<char> &contents);
public:
    explicit RomFile(const std::string& filePath);

    uint8_t read8(uint16_t address);
    uint16_t read16(uint16_t address);

    void write8(uint16_t address, uint8_t arg);
};
#endif //EMULATORTEST_ROM_H

