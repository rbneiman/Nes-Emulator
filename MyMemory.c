#include <stdint.h>

int8_t memory[0xFFFF];

int8_t readMemory8(uint16_t address){
    return memory[address];
}

int16_t readMemory16(uint16_t address){
    return *((uint16_t*) &memory + address);;
}