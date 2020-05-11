#include <stdint.h>

int8_t memory[0xFFFF];

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

void memoryInit(){
    for(int i = 0x4000; i<0x4014; i++){
        memory[i] = 0;
    }
    memory[0x4015] = 0;
    memory[0x4017] = 0;
}

uint8_t readMemory8(uint16_t address){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address %= 0x8u;}
    return memory[address];
}

void writeMemory8(uint16_t address, uint8_t arg){
    if(address<0x2000){ address %= 0x800u;} //mirror
    else if(address<0x4000){ address %= 0x8u;}
    memory[address] = arg;
}

int16_t readMemory16(uint16_t address){
    return *((uint16_t*) &memory + address);;
}