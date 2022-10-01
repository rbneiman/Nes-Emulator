

#ifndef EMULATORTEST_CPU_H
#define EMULATORTEST_CPU_H

#include "utilities.h"
#include "memory.h"

//#define DEBUG_CPU
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

//    DebugLogFile debugLogFile;
    int debugNumCycles;

    uint64_t cpuTime;
    int DMACycleNum;
    uint8_t DMAArg;
public:
    CPU6502();
    void inc(int units);
    uint64_t cycle(uint64_t runTo);

    void printMemoryDebug(int start, int end);

    void printStatus() const;

    void doNMI();

    void loadRom();

    void setMemory(CPUMemory *memory);

    void startOAMDMA(uint8_t DMAArgIn);

    void doDMACycles(uint64_t runTo);
};



#endif //EMULATORTEST_CPU_H
