//
// Created by alec on 8/27/2021.
//

#include "Mapper.h"
#include <iostream>

#define inRange(num, low, high) (num >= low && num <= high)

uint8_t Mapper::read8(uint16_t address) {
    return read16(address);
}

Mapper::Mapper(const std::vector<char> &contents): contents(contents){
    prgSize = contents[4] * 0x4000; //16 kiB
    chrSize = contents[5] * 0x2000; //8 kiB

    uint8_t flags6 = contents[6];
    mirror_type = static_cast<mirror_type_t>(flags6 & 0b1);
    has_persistent = flags6 & 0b10;
    has_trainer = flags6 & 0b100;
    ignore_mirror = flags6 & 0b1000;
    prgRamSize = contents[8];
}

//MAPPER 0

Mapper0::Mapper0(const std::vector<char> &contents): Mapper{contents} {
    int index = 16;
    prg = {contents.data() + index, contents.data() + index + prgSize};
    index += prgSize;
    chr = {contents.data() + index, contents.data() + index + chrSize};
    index += chrSize;
}

uint16_t Mapper0::read16(uint16_t address) {
    switch(address){
        case 0x0000 ... 0x1FFF:
            return *((uint16_t*) (chr.data() + address));
        case 0x2000 ... 0x2FFF:
            return *((uint16_t*) (vram + address - 0x2000));
        case 0x8000 ... 0xBFFF:
            return *((uint16_t*) (prg.data() + address - 0x8000));
        case 0xC000 ... 0xFFFF:
            if(prg.size() > 0x4000)
                return *((uint16_t*) (prg.data() + address - 0x8000));
            else
                return *((uint16_t*) (prg.data() + address - 0xC000));
        default:
            std::cerr << "Bad ROM address: " << std::hex << address << std::endl;
            return 0;
    }
}

void Mapper0::write8(uint16_t address, uint8_t arg) {
    if(inRange(address, 0x2000, 0x2fff)){ //VRAM
        vram[address - 0x2000] = arg;
    }else{
        std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
    }
}

//MAPPER 1

Mapper1::Mapper1(const std::vector<char> &contents) : Mapper(contents), prgRAM(0x8000){
    int index = 16;
    prg = {contents.data() + index, contents.data() + index + prgSize};
    index += prgSize;
    chr = {contents.data() + index, contents.data() + index + chrSize};
    index += chrSize;
}

void Mapper1::setControl(uint8_t arg) {
    mirroring = arg & 0b11; //0: one-screen, lower bank; 1: one-screen, upper bank;  2: vertical; 3: horizontal
    switch ((arg & 0b01100)>>2) { //PRG ROM bank mode
        case 0 ... 1: // switch 32 KB at $8000, ignoring low bit of bank number;
            prgBankMode = 0;
            break;
        case 2: // fix first bank at $8000 and switch 16 KB bank at $C000;
            prgBankMode = 2;
            break;
        case 3: // fix last bank at $C000 and switch 16 KB bank at $8000
            prgBankMode = 3;
            break;
    }

    if((arg & 0b10000)){ //CHR bank mode
        chrBankMode = 0; //switch 8 KB at a time
    }else{
        chrBankMode = 1; //switch two separate 4 KB banks
    }

    control = arg;
}

uint8_t Mapper1::writeLoadReg(uint8_t arg) {
    if(arg & 0x80){ //reset bit
        setControl(control | 0x0C);
        loadReg = 0b10000;
    }
    uint8_t out;
    if(loadReg & 0x1){
        loadReg = ((arg & 0x1) << 4) | (loadReg >> 1);
        loadDone = true;
        out = loadReg & 0b1111;
        loadReg = 0b10000;
    }else{
        loadReg = ((arg & 0x1) << 4) | (loadReg >> 1);
        out = loadReg & 0b1111;
    }
    return out;
}

void Mapper1::writeControl(uint8_t arg) {
    uint8_t result = writeLoadReg(arg);
    if(loadDone){
        setControl(result);
        loadDone = false;
    }
}

void Mapper1::writeChrBank0(uint8_t arg) {
    uint8_t result = writeLoadReg(arg);
    if(loadDone){
        chrBank0 = result;
        if(!chrBankMode){ // 8 KB mode, move both CHR banks
            chrBank0Off = 0x1000 * result;
            chrBank0Off = 0x1000 * result + 0x1000;
        }else{ // 4 KB mode, move only bank 0
            chrBank0Off = 0x1000 * result;
        }
        loadDone = false;
    }
}

void Mapper1::writeChrBank1(uint8_t arg) {
    uint8_t result = writeLoadReg(arg);
    if(loadDone){
        chrBank1 = result;
        if(chrBankMode){ //only switch in 4 KB mode
            chrBank1Off = 0x1000 * result;
        }
        loadDone = false;
    }
}

void Mapper1::writePrgBank(uint8_t arg) {
    uint8_t result = writeLoadReg(arg);
    if(loadDone){
        prgBank = result;
        uint8_t switchAmount = result & 0b01111;
        if(result & 0b10000){} //PRG RAM enable TODO
        if(prgBankMode == 0){ //switch both
            prgBank0Off = 0x4000 * switchAmount;
            prgBank1Off = 0x4000 * switchAmount + 0x4000;
        } else if(prgBankMode == 2){ //bank 0 fixed
            prgBank1Off = 0x4000 * switchAmount;
        } else if(prgBankMode == 3){ //bank 1 fixed
            prgBank0Off = 0x4000 * switchAmount;
        }
        loadDone = false;
    }
}

uint16_t Mapper1::read16(uint16_t address) {
    switch(address){
        case 0x0000 ... 0x0FFF: //chr bank 0
            return *((uint16_t*) (chr.data() + address + chrBank0Off));
        case 0x1000 ... 0x1FFF: //chr bank 1
            return *((uint16_t*) (chr.data() + address - 0x1000 + chrBank1Off));
        case 0x2000 ... 0x2FFF: //VRAM
            return *((uint16_t*) (vram + address - 0x2000));
        case 0x6000 ... 0x7FFF: //prg RAM bank (optional)
            return *((uint16_t*) (prgRAM.data() + address - 0x6000 + prgRAMBankOff));
        case 0x8000 ... 0xBFFF: //prg ROM bank 0
            return *((uint16_t*) (prg.data() + address - 0x8000 + prgBank0Off));
        case 0xC000 ... 0xFFFF: //prg ROM bank 1
            return *((uint16_t*) (prg.data() + address - 0xC000 + prgBank1Off));
        default:
            std::cerr << "Bad ROM address: " << std::hex << address << std::endl;
            return 0;
    }
}

uint16_t Mapper1::mirror(uint16_t address) const {
    switch (mirroring) {
        case 0: // one-screen, lower bank, i.e. show only (0x2000 - 0x23FF)
            return 0x2000 + ((address - 0x2000) % 0x400);
        case 1: // one-screen, upper bank, i.e. show only (0x2400 - 0x27FF)
            return 0x2400 + ((address - 0x2000) % 0x400);
        case 2: // vertical
            return 0x2000 + ((address - 0x2000) % 0x800);
        case 3: // horizontal
            return 0x2000 + ((address - 0x2000) / 0x800) * 0x800 + ((address - 0x2000) % 0x400);
        default:
            return 0;
    }

}

void Mapper1::write8(uint16_t address, uint8_t arg) {
    switch (address) {
        case 0x2000 ... 0x2FFF:
            vram[mirror(address) - 0x2000] = arg;
            break;
        case 0x8000 ... 0x9FFF: // control
            writeControl(arg);
            break;
        case 0xA000 ... 0xBFFF: // chr bank 0
            writeChrBank0(arg);
            break;
        case 0xC000 ... 0xDFFF: // chr bank 1
            writeChrBank1(arg);
            break;
        case 0xE000 ... 0xFFFF: // prg bank
            writePrgBank(arg);
            break;
        default:
            std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
    }
}








