//
// Created by Alec on 3/4/2020.
//

#ifndef EMULATORTEST_MYMEMORY_H
#define EMULATORTEST_MYMEMORY_H
#include <stdint.h>

extern uint16_t scroll;

void InitMemory();
int8_t readMemory8(uint16_t address);
void writeMemory8(uint16_t address, uint8_t arg);
uint8_t* getMemory8(uint16_t address);
int16_t readMemory16(uint16_t address);

#endif //EMULATORTEST_MYMEMORY_H
