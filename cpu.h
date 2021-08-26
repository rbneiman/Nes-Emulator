

#ifndef EMULATORTEST_CPU_H
#define EMULATORTEST_CPU_H

#include "utilities.h"
#include "memory.h"

#define DEBUG_CPU
class CPUMemory;

class CPU6502{
private:
    CPUMemory* memory;

    uint8_t acc;
    uint8_t xindex;
    uint8_t yindex;
    uint8_t status; //flags bit 7-0: N,V,1(unused),B,D,I,Z,C
    uint8_t sp;
    uint16_t pc;

    DebugLogFile debugLogFile;
    int debugNumCycles;

    uint32_t cpuTime;
public:
    CPU6502(CPUMemory* memory);
    void inc(int units);
    void cycle(uint32_t numClocks);

    void printMemoryDebug(int start, int end);

    void printStatus() const;

    void doNMI();

    void loadRom();
};



#endif //EMULATORTEST_CPU_H
