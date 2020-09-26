#include <cstdint>
#include "ppu.h"
#include "cpu.h"

#define cpuInc(arg) cpuTime += 15 * arg

uint8_t memory[0xFFFF];
uint16_t scrollTemp;
uint16_t scroll;
uint16_t addrTemp;
uint16_t addr;

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
//$4020-$FFFF	$BFE0	Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)

void writeMemory8(uint16_t address, uint8_t arg);

void InitMemory(){
    for(uint16_t i = 0x4000; i<0x4014; i++){
        memory[i] = 0;
    }
    memory[0x4015] = 0;
    memory[0x4017] = 0;

    scrollTemp = 0;
    scroll = 0;
    addrTemp = 0;
    addr = 0;

    //TEST
    const uint8_t prog[] = {0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x2, 0xCC, 0x0, 0x02};
    for(uint16_t i = 0; i< sizeof(prog)/ sizeof(uint8_t); i++){
       memory[i+0x600] = prog[i];
    }
    //
}

uint8_t readMemory8(uint16_t address){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}

    if(address==0x2002){ //PPUCTRL
        uint8_t temp = memory[address];
        scrollTemp = 0;
        scroll = 0;
        addrTemp = 0;
        scroll = 0;
        writeMemory8(address, temp & ~0x80);
        return temp;
    }
    if(address==0x2004){return OAM[readMemory8(0x2003)];} //OAMDATA
    if(address==0x2007) { return readPPUMemory8(addr);} //PPUDATA
    return memory[address];
}

void writeMemory8(uint16_t address, uint8_t arg){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}

    if(address==0x2005){ //PPUSCROLL
        if(scrollTemp==0){
            scrollTemp = arg;
        }
        else{
            scroll = (scrollTemp<<8) | arg;
            scrollTemp = 0;
        }
        return;
    }
    if(address==0x2006){ //PPUADDR
        if(addrTemp==0){
            addrTemp = arg;
        }
        else{
            addr = (addrTemp<<8) | arg;
            addrTemp = 0;
        }
        return;
    }
    if(address==0x2007){ //PPUDATA
        writePPUMemory8(addr,arg);
    }
    if(address == 0x4014){ //OAMDMA TODO add +1 if on odd cpu cycle
       uint16_t upper = arg<<8;
       for(int i=0; i<0x100; i++){
           OAM[i] = readMemory8(upper + i);
       }
       cpuInc(513);
    }

    memory[address] = arg;
}

uint8_t* getMemory8(uint16_t address){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}
    return &memory[address];
}

uint16_t readMemory16(uint16_t address){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address = address % 0x8u + 0x2000u;}
    uint16_t* out = (uint16_t *) &(memory[address]);
    return *out;
}