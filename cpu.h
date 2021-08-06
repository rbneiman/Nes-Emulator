

#ifndef EMULATORTEST_CPU_H
#define EMULATORTEST_CPU_H

#include "memory.h"
#include "rom.h"
#include "utilities.h"
//#define PUSH(arg) (writeMemory8(sp--,arg))
//#define POP(arg)  (readMemory8(sp++))

class CPUMemory;

class CPU6502{
private:
    uint8_t acc;
    uint8_t xindex;
    uint8_t yindex;
    uint8_t status; //flags bit 7-0: N,V,1(unused),B,D,I,Z,C
    uint8_t sp;
    uint16_t pc;

    DebugLogFile debugLogFile;
    int debugNumCycles;

    uint32_t cpuTime;
    CPUMemory* memory;
public:
    CPU6502();
    void inc(int units);
    void cycle(int numClocks);
    void setRom(RomFile *rom);

    void printMemoryDebug(int start, int end);

    void printStatus() const;
};



#endif //EMULATORTEST_CPU_H
