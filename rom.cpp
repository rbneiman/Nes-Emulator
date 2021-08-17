//
// Created by Alec on 8/3/2021.
//

#include <iostream>
#include <vector>
#include <cstdint>
#include "rom.h"
#include "utilities.h"

//#define MIRROR_HORIZONTAL 0
//#define MIRROR_VERTICAL 1

#define inRange(num, low, high) (num >= low && num <= high)

ErrorMessage RomFile::initNES10(const std::vector<char>& contents){
    nes20 = false;

    int prgSize = contents[4] * 0x4000;
    int chrSize = contents[5] * 0x2000;

    uint8_t flags6 = contents[6];
    mirror_type = static_cast<mirror_type_t>(flags6 & 0b1);
    has_persistent = flags6 & 0b10;
    has_trainer = flags6 & 0b100;
    ignore_mirror = flags6 & 0b1000;
    uint8_t flags7 = contents[7];
    consoleType = static_cast<console_type_t>((flags7 & 0b10) | (flags7 & 0b01));

    mapperNum = (flags7 & 0b11110000) | (flags6 & 0b00001111);
    uint8_t prgRamSize = contents[8];
    auto tvSystem = static_cast<tv_system>(contents[9] & 0b1);

    if(consoleType != NES){
        return {NON_SUPPORTED_CONSOLE, "Not an NES game."};
    }

    if(contents[11] != 0 || contents[12] != 0 || contents[13] !=0 || contents[14] !=0 || contents[15] != 0){
        mapperNum &= 0b00001111;
    }

    int index = 16;
    if(has_trainer){
        trainer = {contents.data() + index, contents.data() + index + 512};
        index += 512;
    }
    prg_rom = {contents.data() + index, contents.data() + index + prgSize};
    index += prgSize;
    chr_rom = {contents.data() + index, contents.data() + index + chrSize};
    index += chrSize;
    misc_rom = {contents.data() + index, contents.data() + contents.size()};

    return {NONE, "fine"};
}

ErrorMessage RomFile::initNES20(const std::vector<char>& contents){
    nes20 = true;

    int prgSize = contents[4] * 0x4000;
    int chrSize = contents[5] * 0x2000;

    uint8_t flags6 = contents[6];
    mirror_type = static_cast<mirror_type_t>(flags6 & 0b1);
    has_persistent = flags6 & 0b10;
    has_trainer = flags6 & 0b100;
    ignore_mirror = flags6 & 0b1000;

    uint8_t flags7 = contents[7];
    consoleType = static_cast<console_type_t>((flags7 & 0b10) | (flags7 & 0b01));
    mapperNum = ((contents[8] & 0b00001111)<<4) | ((flags7 & 0b11110000)) | ((flags6 & 0b11110000)>>4);
    uint8_t subMapper = ((contents[8] & 0b11110000)>>4);

    return {NONE, "fine"};
}

RomFile::RomFile(const std::string& filePath) {
    const std::vector<char> contents = loadFile(filePath);

    if(contents[0] != 0x4e || contents[1] != 0x45 || contents[2] != 0x53 || contents[3] != 0x1a){
        std::cerr << filePath << ": Not an iNES file!" << std::endl;
        return;
    }


    bool nes20 = contents[7] & 0b1000 + contents[7] & 0b100;
    ErrorMessage initError = nes20 ? initNES20(contents) : initNES10(contents);
    if(initError){
        std::cerr << initError.getMessage() << std::endl;
        return;
    }

    int i = 0;
}

uint8_t RomFile::read8(uint16_t address){
    return read16(address);
}

uint16_t RomFile::read16(uint16_t address){
    switch (mapperNum) {
        case 0:
            if(inRange(address, 0x0000, 0x1fff)){ //CHR_ROM
                return *((uint16_t*) (chr_rom.data() + address));
            }else if(inRange(address, 0x2000, 0x2fff)){ //VRAM
                return *((uint16_t*) (vram.data() + address - 0x2000));
            }else if(inRange(address, 0x8000, 0xbfff)){
                return *((uint16_t*) (prg_rom.data() + address - 0x8000));
            }else if(inRange(address, 0xC000, 0xFFFF)){
                if(prg_rom.size() > 0x4000)
                    return *((uint16_t*) (prg_rom.data() + address - 0x8000));
                else
                    return *((uint16_t*) (prg_rom.data() + address - 0xC000));
            }else{
                std::cerr << "Bad ROM address: " << address << std::endl;
                return 0;
            }
            break;
        default:
            std::cerr << "Unknown mapper type: " << this->mapperNum << std::endl;
            break;
    }
}


void RomFile::write8(uint16_t address, uint8_t arg){
    switch (mapperNum) {
        case 0:
            if(inRange(address, 0x2000, 0x2fff)){ //VRAM
                vram[address - 0x2000] = arg;
            }else{
                std::cerr << "Bad ROM write address: " << address << std::endl;
            }
            break;
        default:
            std::cerr << "Unknown mapper type: " << this->mapperNum << std::endl;
            break;
    }
}







