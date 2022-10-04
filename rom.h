#ifndef EMULATORTEST_ROM_H
#define EMULATORTEST_ROM_H
#include <string>
#include <vector>
#include "utilities.h"
#include "Mapper.h"



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
    Mapper* mapper;
    const std::vector<char> contents;

    std::string title;



    console_type_t consoleType;

    bool nes20;
    int mapperNum;

    void setMapper();

    ErrorMessage initNES10();
    ErrorMessage initNES20();
public:
    explicit RomFile(const std::string& filePath);

    uint8_t read8(uint16_t address);
    uint16_t read16(uint16_t address);

    void write8(uint16_t address, uint8_t arg);
};
#endif //EMULATORTEST_ROM_H

