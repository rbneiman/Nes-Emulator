#include <cstdint>
#include "cpu.h"
#include "memory.h"

//#define cpuInc(arg) cpuTime += 15 * arg


//CPU MEMORY MAP
//Address range	Size	Device
//$0000-$07FF	$0800	2KB internal RAM
//$0800-$0FFF	$0800	Mirror of $0000-$07FF
//$1000-$17FF	$0800   Mirror of $0000-$07FF
//$1800-$1FFF	$0800   Mirror of $0000-$07FF
//$2000-$2007	$0008	NES PPU registers
//$2008-$3FFF	$1FF8	Mirrors of $2000-2007 (repeats every 8 bytes)
//$4000-$4017	$0018	NES APU and I/O registers
//$4018-$401F	$0008	APU and I/O functionality that is normally disabled. See CPU Test Mode.
//$4020-$FFFF	$BFE0	Cartridge space: PRG system->rom, PRG RAM, and mapper registers (See Note)


//void writeMemory8(uint16_t address, uint8_t arg);

CPUMemory::CPUMemory(RomFile* rom, PPU* ppu, Controller* controller, CPU6502* cpu):
    rom(rom),
    ppu(ppu),
    controller(controller),
    cpu(cpu){
    for(uint16_t i = 0x4000; i<0x4014; i++){
        memory[i] = 0;
    }
    memory[0x4015] = 0;
    memory[0x4017] = 0;

    addr = 0;
}

uint8_t CPUMemory::readMemory8(uint16_t address){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}
    else if(address > 0x4019){return rom->read16(address);}
    uint8_t temp;
    switch (address) {
        case 0x2002: //PPUSTATUS
            return ppu->getPpuStatus();
        case 0x2004: //OAMDATA
            return ppu->getOamData();
        case 0x2007: //PPUDATA
            return ppu->getPpuData();
        case 0x4016:
            temp = controller->read(1);
            return temp;
        case 0x4017:
            return controller->read(2);
        default:
            return memory[address];
    }
}

void CPUMemory::writeDMA(uint8_t arg, int index){
    uint16_t pageAddr = ((uint16_t) arg) << 8;
    ppu->setOamDma(readMemory8(pageAddr + index));
}

void CPUMemory::writeMemory8(uint16_t address, uint8_t arg){
    if(address<0x2000){ //mirror
        address %= 0x800u;
    }
    else if(address<0x4000){
        address = address % 0x8u + 0x2000u;
    }

    switch (address) {
        case 0x2000: //PPUCTRL
            ppu->setPpuCtrl(arg);
            break;
        case 0x2001: //PPUMASK
            ppu->setPpuMask(arg);
            break;
        case 0x2003: //OAMADDR
            ppu->setOamAddr(arg);
            break;
        case 0x2004: //OAMDATA
            ppu->setOamData(arg);
            break;
        case 0x2005: //PPUSCROLL
            ppu->setPpuScroll(arg);
            break;
        case 0x2006: //PPUADDR
            ppu->setPpuAddr(arg);
            break;
        case 0x2007: //PPUDATA
            ppu->setPpuData(arg);
            break;
        case 0x4014: //OAMDMA
            cpu->startOAMDMA(arg);
            break;
        case 0x4016:
            controller->write(arg);
            break;
        case 0x4020 ... 0xffff:
            rom->write8(address, arg);
            break;
        default:
            memory[address] = arg;
    }
}


uint16_t CPUMemory::readMemory16(uint16_t address, bool zPage){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}
    else if(address > 0x4019){return rom->read16(address);}
    uint16_t out;
    if(zPage && address == 0x00FF)
        out = (memory[0]<<8) + memory[address];
    else
        out = *((uint16_t *) &(memory[address]));
    return out;
}

void CPUMemory::printMemoryDebug(int start, int end){
    printf("\n      ");
    for(int i = 0; i<0x10; i++){
        printf("%02x ", i);
    }
    printf("\n\n%04x  ", start-start%16);

    for(int i=start-start%16; i<=end; i++){
        if(i%0x10==0 && i!=start-start%16){
            printf("\n%04x  ", i);
        }
        printf("%02x ", readMemory8(i));
    }
    printf("\n");
    fflush(stdout);
}