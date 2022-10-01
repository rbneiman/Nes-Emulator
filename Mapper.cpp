//
// Created by alec on 8/27/2021.
//

#include "Mapper.h"
#include "utilities.h"
#include <iostream>

#define inRange(num, low, high) (num >= low && num <= high)

uint8_t Mapper::read8(uint16_t address) {
    return read16(address);
}

Mapper::Mapper(const std::vector<char> &contents): contents(contents){
    prgSize = contents[4] * 0x4000; //16 kiB
    chrSize = contents[5] * 0x2000; //8 kiB
    hasChrRam = !chrSize;
    uint8_t flags6 = contents[6];
    mirrorType = static_cast<mirrorType_t>(flags6 & 0b1);
    has_persistent = flags6 & 0b10;
    has_trainer = flags6 & 0b100;
    ignore_mirror = flags6 & 0b1000;
    prgRamSize = contents[8];
    prgRam.resize(0x8000);
    if(hasChrRam){
        chrRam.resize(0x8000);
    }
}

void Mapper::printMemoryDebug(int start, int end){
    printf("\n      ");
    for(int i = 0; i<0x10; i++){
        printf("%02x ", i);
    }
    printf("\n\n%04x  ", start-start%16);

    for(int i=start-start%16; i<=end; i++){
        if(i%0x10==0 && i!=start-start%16){
            printf("\n%04x  ", i);
        }
        printf("%02x ", read8(i));
    }
    printf("\n");
    fflush(stdout);
}

//MAPPER 0

Mapper0::Mapper0(const std::vector<char> &contents): Mapper{contents} {
    prgStart = 16;
    chrStart = 16 + prgSize;
}

uint16_t Mapper0::read16(uint16_t address) {
    switch(address){
        case 0x0000 ... 0x1FFF:
            if(hasChrRam){
                return *((uint16_t*) (chrRam.data() + address));
            }else {
                return *((uint16_t *) (contents.data() + chrStart + address));
            }
        case 0x2000 ... 0x2FFF:
            address -= 0x2000;
            if(mirrorType == VERTICAL){
                address %= 0x800;
            }else{
                address = ((address) / 0x800) * 0x800 + ((address) % 0x400);
            }
            return *((uint16_t*) (vram + address));
        case 0x6000 ... 0x7FFF: //prg ram just in case
            return *((uint16_t*) (prgRam.data() + (address - 0x6000)));
        case 0x8000 ... 0xBFFF:
            return *((uint16_t*) (contents.data() + prgStart + address - 0x8000));
        case 0xC000 ... 0xFFFF:
            if(prgSize > 0x4000)
                return *((uint16_t*) (contents.data() + prgStart + address - 0x8000));
            else
                return *((uint16_t*) (contents.data() + prgStart + address - 0xC000));
        default:
            std::cerr << "Bad ROM address: " << std::hex << address << std::endl;
            return 0;
    }
}


void Mapper0::write8(uint16_t address, uint8_t arg) {
    switch (address) {
        case 0x0000 ... 0x1FFF:
            if (hasChrRam) {
                chrRam[address] = arg;
            } else {
                std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
                return;
            }
        case 0x2000 ... 0x2FFF:
            address -= 0x2000;
            if(mirrorType == VERTICAL){
                address %= 0x800;
            }else{
                address = ((address) / 0x800) * 0x800 + ((address) % 0x400);
            }
            vram[address] = arg;
            break;
        case 0x6000 ... 0x7FFF: //prg ram
            prgRam[address - 0x6000] = arg;
            break;
        default:
            std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
    }
}

//MAPPER 1

Mapper1::Mapper1(const std::vector<char> &contents) : Mapper(contents){
    prgRomStart = 16;
    chrRomStart = 16 + prgSize;
    prgRam.resize(0x8000);
    prgBank1 = ((prgSize/0x4000)-1) * 0x4000;
}


//  CPU $6000-$7FFF: 8 KB PRG RAM bank, (optional)
//  CPU $8000-$BFFF: 16 KB PRG ROM bank, either switchable or fixed to the first bank
//  CPU $C000-$FFFF: 16 KB PRG ROM bank, either fixed to the last bank or switchable
//  PPU $0000-$0FFF: 4 KB switchable CHR bank
//  PPU $1000-$1FFF: 4 KB switchable CHR bank

uint16_t getMirrorAddress(uint16_t address, mirrorType_t mirrorType){
    address -= 0x2000;
    switch (mirrorType) {
        case HORIZONTAL:
            return ((address) / 0x800) * 0x800 + ((address) % 0x400);
        case VERTICAL:
            return address %= 0x800;
        case ONE_SCREEN_LOWER:
            return address %= 0x400;
        case ONE_SCREEN_UPPER:
            return address = (address % 0x400) + 0x400; //no clue what this maps to
    }
}



uint16_t Mapper1::read16(uint16_t address) {
    uint32_t casted = address;
    switch(address){
        case 0x0000 ... 0x0FFF: //chrBank0
            if(hasChrRam){
                return *((uint16_t*) (chrRam.data() + chrBank0 + casted));
            }else{
                return *((uint16_t*) (contents.data() + chrRomStart + chrBank0 + casted));
            }
        case 0x1000 ... 0x1FFF: //chrBank1
            if(hasChrRam){
                return *((uint16_t*) (chrRam.data() + chrBank1 + casted - 0x1000));
            }else{
                return *((uint16_t*) (contents.data() + chrRomStart + chrBank1 + casted - 0x1000));
            }
        case 0x2000 ... 0x2FFF: //vram
            return *((uint16_t*) (vram + getMirrorAddress(address, mirrorType)));
        case 0x6000 ... 0x7FFF: //prg ram
            return *((uint16_t*) (prgRam.data() + (casted - 0x6000)));
        case 0x8000 ... 0xBFFFL: //prgRom0
            return *((uint16_t*) (contents.data() + (prgRomStart + prgBank0 + casted - 0x8000)));
        case 0xC000 ... 0xFFFF: //prgRom1
            return *((uint16_t*) (contents.data() + (prgRomStart + prgBank1 + casted - 0xC000)));
        default:
            std::cerr << "Bad ROM read address: " << std::hex << address << std::endl;
            break;
    }
    return 0;
}


// Control: 0x8000 - 0x9FFF
//    4bit0
//    -----
//    CPPMM
//    |||||
//    |||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
//    |||               2: vertical; 3: horizontal)
//    |++--- PRG ROM bank mode (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
//    |                         2: fix first bank at $8000 and switch 16 KB bank at $C000;
//    |                         3: fix last bank at $C000 and switch 16 KB bank at $8000)
//    +----- CHR ROM bank mode (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)

// CHR Bank 0: 0xA000 - 0xBFFF
//    4bit0
//    -----
//    CCCCC
//    |||||
//    +++++- Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode)

// CHR Bank 1: 0xC000 - 0xDFFF
//    4bit0
//    -----
//    CCCCC
//    |||||
//    +++++- Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode)

// PRG Bank: 0xE000 - 0xFFFF
//    4bit0
//    -----
//    RPPPP
//    |||||
//    |++++- Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
//    +----- MMC1B and later: PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
//    MMC1A: Bit 3 bypasses fixed bank logic in 16K mode (0: affected; 1: bypassed)


void Mapper1::write8(uint16_t address, uint8_t arg) {
    if(address >= 0x8000){
        if(arg & 0x80){
            shiftRegister = 0;
            shiftProgress = 0;
        }else{
            shiftRegister = (shiftRegister>>1) | ((arg&0x1)<<4);
            ++shiftProgress;
        }
        if(shiftProgress < 5){
            return;
        }
    }
    uint32_t casted = address;
    switch (address) {
        case 0x0000 ... 0x0FFF: //chrBank0
            if(hasChrRam){
                chrRam[chrBank0 + casted] = arg;
            }else{
                std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
                return;
            }
            break;
        case 0x1000 ... 0x1FFF: //chrBank1
            if(hasChrRam){
                chrRam[chrBank1 + casted - 0x1000] = arg;
            }else{
                std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
                return;
            }
            break;
        case 0x2000 ... 0x2FFF: //vram
            vram[address - 0x2000] = arg;
            break;
        case 0x6000 ... 0x7FFF: //prg ram
            prgRam[address - 0x6000] = arg;
            break;
        case 0x8000 ... 0x9FFF: { //control
            int mirror = shiftRegister & 0x3;
            switch (mirror) {
                case 0:
                    mirrorType = ONE_SCREEN_LOWER;
                    break;
                case 1:
                    mirrorType = ONE_SCREEN_UPPER;
                    break;
                case 2:
                    mirrorType = VERTICAL;
                    break;
                case 3:
                    mirrorType = HORIZONTAL;
                default:
                    break;
            }
            prgRomBankMode = bitSlice(shiftRegister, 3, 2);
            if(prgRomBankMode == 2){
                prgBank0 = 0;
            }else if(prgRomBankMode == 3){
                prgBank1 = ((0x20000/0x4000)-1) * 0x4000;
            }
            chrRomBankMode = bitSlice(shiftRegister, 4, 4);
            shiftProgress = 0;
            shiftRegister = 0;
            break;
        }
        case 0xA000 ... 0xBFFF: //chr bank 0
            if(chrSize <= 0x2000){ // roms with only 8KB of chr only use first bit
                shiftRegister &= 0x1;
            }
            if(!chrRomBankMode){ // low bit ignored in 8KB mode
                shiftRegister &= ~0x1;
                chrBank0 = shiftRegister * 0x1000;
                chrBank1 = shiftRegister * 0x1000 + 0x1000;
            }else{
                chrBank0 = shiftRegister * 0x1000;
            }
            shiftProgress = 0;
            shiftRegister = 0;
            break;
        case 0xC000 ... 0xDFFF: //chr bank 1
            if(chrRomBankMode){ // only do something in 4KB mode
                chrBank1 = shiftRegister * 0x1000;
            }
            shiftProgress = 0;
            shiftRegister = 0;
            break;
        case 0xE000 ... 0xFFFF: { //prg bank
            uint8_t bit4 = bitSlice(shiftRegister, 4, 4);
            shiftRegister = bitSlice(shiftRegister, 3, 0);
            switch(prgRomBankMode){
                case 0: //0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
                case 1:
                    shiftRegister &= ~0x1; //ignore low bit
                    prgBank0 = shiftRegister * 0x8000;
//                    prgBank1 = shiftRegister * 0x4000 + 0x4000;
                    break;
                case 2: // 2: fix first bank at $8000 and switch 16 KB bank at $C000;
                    prgBank1 = shiftRegister * 0x4000;
                    break;
                case 3: // 3: fix last bank at $C000 and switch 16 KB bank at $8000
                    prgBank0 = shiftRegister * 0x4000;
                    break;
                default:
                    break;
            }
            shiftProgress = 0;
            shiftRegister = 0;
            break;
        }
        default:
            std::cerr << "Bad ROM write address: " << std::hex << address << std::endl;
            break;
    }
}


Mapper66::Mapper66(const std::vector<char> &contents) : Mapper0(contents) {

}


uint16_t Mapper66::read16(uint16_t address) {
    uint32_t casted = address;
    switch (address) {
        case 0x0000 ... 0x1FFF:
            return *((uint16_t*) (contents.data() + (chrStart + chrBankOff + casted)));
        case 0x8000 ... 0xFFFF:
            return *((uint16_t*) (contents.data() + ((prgStart + prgBankOff + casted) - 0x8000)));
        default:
            return Mapper0::read16(address);
    }
}

//8KB == 0x2000
//32KB == 0x8000

//  7  bit  0
//  ---- ----
//  xxPP xxCC
//    ||   ||
//    ||   ++- Select 8 KB CHR ROM bank for PPU $0000-$1FFF
//    ++------ Select 32 KB PRG ROM bank for CPU $8000-$FFFF
void Mapper66::write8(uint16_t address, uint8_t arg){
    if(address > 0x8000){
        chrBankOff = bitSlice(arg, 2, 0) * 0x2000;
        prgBankOff = bitSlice(arg, 5, 4) * 0x8000;
    }else{
        Mapper0::write8(address, arg);
    }
}

Mapper3::Mapper3(const std::vector<char>& contents) : Mapper0(contents){

}

uint16_t Mapper3::read16(uint16_t address){
    switch (address) {
        case 0x0000 ... 0x2000:
            return *((uint16_t*) (contents.data() + (chrStart + chrBankOff + address)));
        default:
            return Mapper0::read16(address);
    }
}

void Mapper3::write8(uint16_t address, uint8_t arg){
    switch (address) {
        case 0x8000 ... 0xFFFF:
            chrBankOff = (arg & 0x3) * 0x2000;
            break;
        default:
            Mapper0::write8(address, arg);
    }

}





