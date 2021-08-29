//
// Created by Alec on 8/3/2021.
//

#include <iostream>
#include <cstdint>
#include "rom.h"

#define inRange(num, low, high) (num >= low && num <= high)

void RomFile::setMapper() {
    switch (mapperNum) {
        case 0:
            mapper = new Mapper0(contents);
            break;
        case 1:
            mapper = new Mapper1(contents);
            break;
        default:
            std::cerr << "Unknown mapper type: " << this->mapperNum << std::endl;
            break;
    }
}


ErrorMessage RomFile::initNES10(){

    uint8_t flags6 = contents[6];
    uint8_t flags7 = contents[7];
    consoleType = static_cast<console_type_t>((flags7 & 0b10) | (flags7 & 0b01));
    mapperNum = (flags7 & 0b11110000) | ((flags6 & 0b11110000)>>4);

    if(consoleType != NES){
        return {NON_SUPPORTED_CONSOLE, "Not an NES game."};
    }

    if(contents[11] != 0 || contents[12] != 0 || contents[13] !=0 || contents[14] !=0 || contents[15] != 0){
        mapperNum &= 0b00001111;
    }

    return {NONE, "fine"};
}

ErrorMessage RomFile::initNES20(){

    uint8_t flags6 = contents[6];
    uint8_t flags7 = contents[7];
    consoleType = static_cast<console_type_t>((flags7 & 0b10) | (flags7 & 0b01));
    mapperNum = ((contents[8] & 0b00001111)<<4) | ((flags7 & 0b11110000)) | ((flags6 & 0b11110000)>>4);
    uint8_t subMapper = ((contents[8] & 0b11110000)>>4);

    return {NONE, "fine"};
}

RomFile::RomFile(const std::string& filePath):
    contents(loadFile(filePath)){

    if(contents[0] != 0x4e || contents[1] != 0x45 || contents[2] != 0x53 || contents[3] != 0x1a){
        std::cerr << filePath << ": Not an iNES file!" << std::endl;
        return;
    }

    nes20 = contents[7] & 0b1000 + contents[7] & 0b100;
    ErrorMessage initError = nes20 ? initNES20() : initNES10();
    if(initError){
        std::cerr << initError.getMessage() << std::endl;
        return;
    }
    setMapper();
}

uint8_t RomFile::read8(uint16_t address){
    return mapper->read8(address);
}

uint16_t RomFile::read16(uint16_t address){
    return mapper->read16(address);
}


void RomFile::write8(uint16_t address, uint8_t arg){
    mapper->write8(address, arg);
}









