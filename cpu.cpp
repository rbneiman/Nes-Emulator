#include <cstdint>
#include <cstdio>
#include <iostream>
#include "cpu.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
#pragma ide diagnostic ignored "UnreachableCode"
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma clang diagnostic pop

#define PCL pc&0xFF
#define PCH (pc&0xFF00)>>8

#define zeropage(arg)       ((arg)%0x100)
#define zpageInd(arg,index) ((arg + index)%256)
#define indIndir(arg) ( memory->readMemory8((arg + xindex)% 256) + memory->readMemory8((arg + xindex + 1)%256) )*256
#define indirInd(arg) (memory->readMemory8(arg) + memory->readMemory8((arg + 1) % 256) * 256 + yindex)

#define setC(arg) (status = (status&0xFEu) | (arg))
#define setZ(arg) (status = (status&0xFDu) | ((arg==0)<<1))
#define setI(arg) (status = (status&0xFBu) | (arg<<2))
#define setD(arg) (status = (status&0xF7u) | (arg<<3))
#define setB(arg) (status = (status&0xEFu) | (arg<<4))
#define setV(arg) (status = (status&0xBFu) | (((uint8_t) arg)<<6))
#define setN(arg) (status = (status&0x7Fu) | ((((int8_t) arg)<0)<<7))

#define PUSH(arg) (memory->writeMemory8(0x100 + (sp--),arg))
#define POP(arg)  (memory->readMemory8(0x100 + (++sp)))

#define pageCross(arg1,arg2) (((arg1)&0xFF00) != ((arg2)&0xFF00))

#define cpuInc(arg) (cpuTime += 15 * arg)

//#define DEBUG

//uint32_t cpuTime;

CPU6502::CPU6502()
#ifndef DEBUG_CPU
{
#else
: debugLogFile("../trace.txt"){
#endif

    cpuTime = 0;

//    this->status = 0x24u;
    this->status = 4;
    acc = 0;
    xindex = 0;
    yindex = 0;
    sp = 0xFDu;
    pc = 0xFFFC;
    DMACycleNum = 514;
    instrProgress = 0;
}

void CPU6502::inc(int units){
    cpuTime += 15*units;
}

void CPU6502::printMemoryDebug(int start, int end){
    this->memory->printMemoryDebug(start, end);
}

uint64_t CPU6502::cycle(uint64_t runTo) {
    static uint8_t lastAcc = 0;
    static uint8_t lastAccTemp = acc;
    bool isOverflow;
//    runTo += cpuTime;
    while (cpuTime < runTo) {
        doDMACycles(runTo);
        if(cpuTime >= runTo){
            break;
        }

        execute();
        cpuInc(1);
    }
    if(acc != lastAccTemp){
        lastAcc = lastAccTemp;
    }
    return cpuTime - runTo;
}

void CPU6502::execute(){
    static uint16_t pcPause = 0xE442;

    bool check;
    switch (instrProgress) {
        case 0:
        case 1: //fetch opcode
#ifdef DEBUG_CPU
            static int num = 0;
            printStatus();
            check = debugLogFile.checkLine(num++, pc, acc, xindex, yindex, status, sp, cpuTime/15 + 8);
            if(!check){ //TODO sprite overflow crap
                std::cout << "Expected:" << std::endl;
                debugLogFile.printLine(num-1);
                fflush(stdout);
            }
            if(pc == pcPause && (this->cpuTime/15 + 8 == 655171)){
                int j = 0;
            }
#endif
            currentOpcode =  memory->readMemory8(pc);
            pc += 1;
            instrProgress = 2;
            break;
        case 2:
            switch(currentOpcode){
                case 0x00: //BRK implied/immediate
                case 0x08: //PHP implied
                case 0x28: //PLP implied
                case 0x40: //RTI Implied/Immediate
                case 0x48: //PHA implied
                case 0x60: //RTS implied/immediate
                case 0x68: //PLA implied
                    instrProgress = 3;
                    break;
                case 0x20: //JSR absolute
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x01: //ORA indirect x
                case 0x21: //AND indirect x
                case 0x41: //EOR indirect x
                case 0x61: //ADC indirect x
                case 0xA1: //LDA indirect x
                case 0x81: //STA indirect x
                case 0xC1: //CMP indirect x
                case 0xE1: //SBC indirect x
                    arg0 = memory->readMemory16(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x05: //ORA zeropage
                case 0x06: //ASL zeropage
                case 0x24: //BIT zeropage
                case 0x25: //AND zeropage
                case 0x26: //ROL zeropage
                case 0x45: //EOR zeropage
                case 0x46: //LSR zeropage
                case 0x65: //ADC zeropage
                case 0x66: //ROR zeropage
                case 0x84: //STY zeropage
                case 0x85: //STA zeropage
                case 0x86: //STX zeropage
                case 0xA4: //LDY zeropage
                case 0xA5: //LDA zeropage
                case 0xA6: //LDX zeropage
                case 0xC4: //CPY zeropage
                case 0xC5: //CMP zeropage
                case 0xC6: //DEC zeropage
                case 0xE4: //CPX zeropage
                case 0xE5: //SBC zeropage
                case 0xE6: //INC zeropage
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x0D: //ORA absolute
                case 0x0E: //ASL absolute
                case 0x2C: //BIT absolute
                case 0x2D: //AND absolute
                case 0x2E: //ROL absolute
                case 0x4C: //JMP absolute
                case 0x4D: //EOR absolute
                case 0x4E: //LSR absolute
                case 0x6D: //ADC absolute
                case 0x6E: //ROR absolute
                case 0x8C: //STY absolute
                case 0x8D: //STA absolute
                case 0x8E: //STX absolute
                case 0xAC: //LDY absolute
                case 0xAD: //LDA absolute
                case 0xAE: //LDX absolute
                case 0xCC: //CPY absolute
                case 0xCD: //CMP absolute
                case 0xCE: //DEC absolute
                case 0xEC: //CPX absolute
                case 0xED: //SBC absolute
                case 0xEE: //INC absolute
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x11: //ORA indirect y
                case 0x31: //AND indirect y
                case 0x51: //EOR indirect y
                case 0x71: //ADC indirect y
                case 0x91: //STA indirect y
                case 0xB1: //LDA indirect y
                case 0xD1: //CMP indirect y
                case 0xF1: //SBC indirect y
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x15: //ORA zeropage x
                case 0x16: //ASL zeropage x
                case 0x35: //AND zeropage x
                case 0x36: //ROL zeropage x
                case 0x55: //EOR zeropage x
                case 0x56: //LSR zeropage x
                case 0x75: //ADC zeropage x
                case 0x76: //ROR zeropage x
                case 0x94: //STY zeropage x
                case 0x95: //STA zeropage x
                case 0x96: //STX zeropage y
                case 0xB4: //LDY zeropage x
                case 0xB5: //LDA zeropage x
                case 0xB6: //LDX zeropage y
                case 0xD5: //CMP zeropage x
                case 0xD6: //DEC zeropage x
                case 0xF5: //SBC zeropage x
                case 0xF6: //INC zeropage x
                    arg0 = memory->readMemory16(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x19: //ORA absolute y
                case 0x1D: //ORA absolute x
                case 0x1E: //ASL absolute x
                case 0x39: //AND absolute y
                case 0x3D: //AND absolute x
                case 0x3E: //ROL absolute x
                case 0x59: //EOR absolute y
                case 0x5D: //EOR absolute x
                case 0x5E: //LSR absolute x
                case 0x79: //ADC absolute y
                case 0x7D: //ADC absolute x
                case 0x7E: //ROR absolute x
                case 0x99: //STA absolute y
                case 0x9D: //STA absolute x
                case 0xB9: //LDA absolute y
                case 0xBC: //LDY absolute x
                case 0xBD: //LDA absolute x
                case 0xBE: //LDX absolute y
                case 0xD9: //CMP absolute y
                case 0xDD: //CMP absolute x
                case 0xDE: //DEC absolute x
                case 0xF9: //SBC absolute y
                case 0xFD: //SBC absolute x
                case 0xFE: //INC absolute x
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x09: //ORA immediate
                    arg0 = memory->readMemory8(pc);
                    acc |= arg0;
                    setZ(acc);
                    setN(acc);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0x0A: //ASL Acc/Implied
                    a = acc;
                    acc = ((int8_t) acc) << 1;
                    setC((a & 0x80) >> 7);
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x10: //BPL relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if (((int8_t) status) > 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0x18: //CLC implied
                    setC(0);
                    instrProgress = 1;
                    break;
                case 0x29: //AND immediate
                    arg0 = memory->readMemory8(pc);
                    acc &= arg0;
                    setZ(acc);
                    setN(acc);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0x2A: //ROL acc/Implied
                    a = acc;
                    acc = (acc << 1) | (status & 0x1);
                    setC((a & 0x80u) >> 7);
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x30: //BMI relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if (((int8_t) status) < 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0x38: //SEC implied
                    setC(1);
                    instrProgress = 1;
                    break;
                case 0x49: //EOR immediate
                    arg0 = memory->readMemory8(pc);
                    acc ^= arg0;
                    setZ(acc);
                    setN(acc);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0x4A: //LSR acc/Implied
                    setC(acc & 0x1u);
                    acc = acc >> 1;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x50: //BVC relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x40u) == 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0x58: //CLI implied
                    setI(0);
                    instrProgress = 1;
                    break;
                case 0x69: //ADC immediate
                    b = memory->readMemory8(pc);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0x6A: //ROR acc/Implied
                    a = acc;
                    acc = (acc >> 1) | ((status & 0x1u) << 7);
                    setC(a & (0x1u));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x6C: //JMP absolute indirect
                    arg0 = memory->readMemory8(pc);
                    pc+=1;
                    instrProgress = 3;
                    break;
                case 0x70: //BVS relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x40u) != 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0x78: //SEI implied
                    setI(1);
                    instrProgress = 1;
                    break;
                case 0x88: //DEY implied
                    yindex -= 1;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0x8A: //TXA acc/Implied
                    acc = xindex;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x90: //BCC relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x1) == 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0x98: //TYA implied
                    acc = yindex;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x9A: //TXS Implied
                    sp = xindex;
                    instrProgress = 1;
                    break;
                case 0xA0: //LDY Implied/Immediate
                    arg0 = memory->readMemory8(pc);
                    yindex = arg0;
                    setZ(yindex);
                    setN(yindex);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0xA2: //LDX ?/Immediate
                    arg0 = memory->readMemory8(pc);
                    xindex = arg0;
                    setZ(xindex);
                    setN(xindex);
                    pc+=1;
                    instrProgress = 1;
                    break;
                case 0xA8: //TAY implied
                    yindex = acc;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xA9: //LDA Immediate
                    arg0 = memory->readMemory8(pc);
                    acc = arg0;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    pc+=1;
                    break;
                case 0xAA: //TAX acc/Implied
                    xindex = acc;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xB0: //BCS relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x1) != 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0xB8: //CLV implied
                    setV(0);
                    instrProgress = 1;
                    break;
                case 0xBA: //TSX implied
                    xindex = sp;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xC0: //CPY Implied/Immediate
                    arg0 = memory->readMemory8(pc);
                    setC(yindex >= arg0);
                    setZ(yindex - arg0);
                    setN((yindex - arg0));
                    pc += 1;
                    instrProgress = 1;
                    break;
                case 0xC8: //INY implied
                    yindex++;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xC9: //CMP immediate
                    arg0 = memory->readMemory8(pc);
                    setC(acc >= arg0);
                    setZ(acc - arg0);
                    setN((acc - arg0));
                    pc += 1;
                    instrProgress = 1;
                    break;
                case 0xCA: //DEX acc/Implied
                    xindex--;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xD0: //BNE relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x2) == 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0xD8: //CLD implied
                    setD(0);
                    instrProgress = 1;
                    break;
                case 0xE0: //CPX Implied/Immediate
                    arg0 = memory->readMemory8(pc);
                    setC(xindex >= arg0);
                    setZ(xindex - arg0);
                    setN((xindex - arg0));
                    pc += 1;
                    instrProgress = 1;
                    break;
                case 0xE8: //INX implied
                    xindex++;
                    setZ(xindex);
                    setN((xindex));
                    instrProgress = 1;
                    break;
                case 0xE9: //SBC immediate
                    arg0 = memory->readMemory8(pc);
                    a = acc;
                    b = arg0;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = acc - arg0 - 1 + (status & 0x1u);

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    pc += 1;
                    instrProgress = 1;
                    break;
                case 0xEA: //NOP acc/Implied
                    instrProgress = 1;
                    break;
                case 0xF0: //BEQ relative
                    rel = memory->readMemory8(pc);
                    pc += 1;
                    if ((status & 0x2u) != 0){
                        instrProgress = 3;
                    }else{
                        instrProgress = 1;
                    }
                    break;
                case 0xF8: //SED implied
                    setD(1);
                    instrProgress = 1;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << int(currentOpcode)
                    << '.' << instrProgress << std::endl;
                    break;
            }
            break;
        case 3:
            switch(currentOpcode){
                case 0x00: //BRK implied/immediate
                    PUSH(((pc & 0xFF00)>>8));
                    setB(1);
                    instrProgress = 4;
                    break;
                case 0x08: //PHP implied
                    arg1 = status;
                    arg1 |= 0b110000;
                    PUSH(arg1);
                    instrProgress = 1;
                    break;
                case 0x20: //JSR absolute
                case 0x28: //PLP implied
                case 0x40: //RTI Implied/Immediate
                case 0x60: //RTS implied/immediate
                case 0x68: //PLA implied
                    instrProgress = 4;
                    break;
                case 0x48: //PHA implied
                    PUSH(acc);
                    instrProgress = 1;
                    break;
                case 0x01: //ORA indirect x
                case 0x21: //AND indirect x
                case 0x41: //EOR indirect x
                case 0x61: //ADC indirect x
                case 0xA1: //LDA indirect x
                case 0x81: //STA indirect x
                case 0xC1: //CMP indirect x
                case 0xE1: //SBC indirect x
                    instrProgress = 4;
                    break;
                case 0x06: //ASL zeropage
                case 0x26: //ROL zeropage
                case 0x46: //LSR zeropage
                case 0x66: //ROR zeropage
                case 0xC6: //DEC zeropage
                case 0xE6: //INC zeropage
                    a = memory->readMemory8(arg0);
                    instrProgress = 4;
                    break;
                case 0x05: //ORA zeropage
                    a = memory->readMemory8(arg0);
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x24: //BIT zeropage
                    a = memory->readMemory8(arg0);
                    arg0 = a & acc;
                    status = (status & ~0x2) | ((arg0==0)<<1); //z bit
                    setV((a & 0x40)>>6);
                    setN(a);
                    instrProgress = 1;
                    break;
                case 0x25: //AND zeropage
                    a = memory->readMemory8(arg0);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x45: //EOR zeropage
                    a = memory->readMemory8(arg0);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x65: //ADC zeropage
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x84: //STY zeropage
                    memory->writeMemory8(zeropage(arg0), yindex);
                    instrProgress = 1;
                    break;
                case 0x85: //STA zeropage
                    memory->writeMemory8(zeropage(arg0), acc);
                    instrProgress = 1;
                    break;
                case 0x86: //STX zeropage
                    memory->writeMemory8(zeropage(arg0), xindex);
                    instrProgress = 1;
                    break;
                case 0xA4: //LDY zeropage
                    a = memory->readMemory8(arg0);
                    yindex = a;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xA5: //LDA zeropage
                    a = memory->readMemory8(arg0);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xA6: //LDX zeropage
                    a = memory->readMemory8(arg0);
                    xindex = a;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xC4: //CPY zeropage
                    a = memory->readMemory8(arg0);
                    setC(yindex >= a);
                    setZ(yindex - a);
                    setN((yindex - a));
                    instrProgress = 1;
                    break;
                case 0xC5: //CMP zeropage
                    a = memory->readMemory8(arg0);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xE4: //CPX zeropage
                    a = memory->readMemory8(arg0);
                    setC(xindex >= a);
                    setZ(xindex - a);
                    setN((xindex - a));
                    instrProgress = 1;
                    break;
                case 0xE5: //SBC zeropage
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x0D: //ORA absolute
                case 0x0E: //ASL absolute
                case 0x2C: //BIT absolute
                case 0x2D: //AND absolute
                case 0x2E: //ROL absolute
                case 0x4D: //EOR absolute
                case 0x4E: //LSR absolute
                case 0x6D: //ADC absolute
                case 0x6E: //ROR absolute
                case 0x8C: //STY absolute
                case 0x8D: //STA absolute
                case 0x8E: //STX absolute
                case 0xAC: //LDY absolute
                case 0xAD: //LDA absolute
                case 0xAE: //LDX absolute
                case 0xCC: //CPY absolute
                case 0xCD: //CMP absolute
                case 0xCE: //DEC absolute
                case 0xEC: //CPX absolute
                case 0xED: //SBC absolute
                case 0xEE: //INC absolute
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    instrProgress = 4;
                    break;
                case 0x4C: //JMP absolute
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc = arg0;
                    instrProgress = 1;
                    break;
                case 0x11: //ORA indirect y
                case 0x31: //AND indirect y
                case 0x51: //EOR indirect y
                case 0x71: //ADC indirect y
                case 0x91: //STA indirect y
                case 0xB1: //LDA indirect y
                case 0xD1: //CMP indirect y
                case 0xF1: //SBC indirect y
                    arg1 = memory->readMemory8(zeropage(arg0));
                    instrProgress = 4;
                    break;
                case 0x15: //ORA zeropage x
                case 0x16: //ASL zeropage x
                case 0x35: //AND zeropage x
                case 0x36: //ROL zeropage x
                case 0x55: //EOR zeropage x
                case 0x56: //LSR zeropage x
                case 0x75: //ADC zeropage x
                case 0x76: //ROR zeropage x
                case 0x94: //STY zeropage x
                case 0x95: //STA zeropage x
                case 0xB4: //LDY zeropage x
                case 0xB5: //LDA zeropage x
                case 0xD5: //CMP zeropage x
                case 0xD6: //DEC zeropage x
                case 0xF5: //SBC zeropage x
                case 0xF6: //INC zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    instrProgress = 4;
                    break;
                case 0x96: //STX zeropage y
                case 0xB6: //LDX zeropage y
                    a = memory->readMemory8(arg0 + yindex);
                    instrProgress = 4;
                    break;
                case 0x19: //ORA absolute y
                case 0x39: //AND absolute y
                case 0x59: //EOR absolute y
                case 0x79: //ADC absolute y
                case 0xB9: //LDA absolute y
                case 0xBE: //LDX absolute y
                case 0xD9: //CMP absolute y
                case 0xF9: //SBC absolute y
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    if (pageCross(arg0, arg0 + yindex)) {
                        instrProgress = 4;
                    }else{
                        instrProgress = 5;
                    }
                    arg0 +=  yindex;
                    break;
                case 0x99: //STA absolute y
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    arg0 +=  yindex;
                    instrProgress = 4;
                    break;
                case 0x1D: //ORA absolute x
                case 0x1E: //ASL absolute x
                case 0x3D: //AND absolute x
                case 0x3E: //ROL absolute x
                case 0x5D: //EOR absolute x
                case 0x5E: //LSR absolute x
                case 0x7D: //ADC absolute x
                case 0x7E: //ROR absolute x
                case 0xBC: //LDY absolute x
                case 0xBD: //LDA absolute x
                case 0xDD: //CMP absolute x
                case 0xDE: //DEC absolute x
                case 0xFD: //SBC absolute x
                case 0xFE: //INC absolute x
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    if (pageCross(arg0, arg0 + xindex)) {
                        instrProgress = 4;
                    }else{
                        instrProgress = 5;
                    }
                    arg0 += xindex;
                    break;
                case 0x9D: //STA absolute x
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    arg0 += xindex;
                    instrProgress = 4;
                    break;
                case 0x10: //BPL relative
                case 0x30: //BMI relative
                case 0x50: //BVC relative
                case 0x70: //BVS relative
                case 0x90: //BCC relative
                case 0xB0: //BCS relative
                case 0xD0: //BNE relative
                case 0xF0: //BEQ relative
                    arg1 = pc + ((int8_t)rel);
                    if ((arg1&0xff00) != (pc&0xff00)) { //new page penalty
                        instrProgress = 4;
                    }else{
                        pc = arg1;
                        instrProgress = 1;
                    }
                    break;
                case 0x6C: //JMP absolute indirect
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc+=1;
                    instrProgress = 4;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << currentOpcode << '.' << instrProgress << std::endl;
                    break;
            }
            break;
        case 4:
            switch(currentOpcode){
                case 0x00: //BRK implied/immediate
                    PUSH((pc & 0x00FF));
                    instrProgress = 5;
                    break;
                case 0x20: //JSR absolute
                    PUSH((((pc) & 0xFF00)>>8));
                    instrProgress = 5;
                    break;
                case 0x28: //PLP implied
                    arg0 = POP();
                    status = (status&0b00110000) | (arg0&0b11001111);
                    instrProgress = 1;
                    break;
                case 0x40: //RTI Implied/Immediate
                    arg0 = POP();
                    status = (status&0b00110000) | (arg0&0b11001111);
                    instrProgress = 5;
                    break;
                case 0x60: //RTS implied/immediate
                    pc = POP();
                    instrProgress = 5;
                    break;
                case 0x68: //PLA implied
                    acc = POP();
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x01: //ORA indirect x
                case 0x21: //AND indirect x
                case 0x41: //EOR indirect x
                case 0x61: //ADC indirect x
                case 0xA1: //LDA indirect x
                case 0x81: //STA indirect x
                case 0xC1: //CMP indirect x
                case 0xE1: //SBC indirect x
                    instrProgress = 5;
                    break;
                case 0x06: //ASL zeropage
                    b = a;
                    a = ((int8_t) a) << 1;
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 5;
                    break;
                case 0x26: //ROL zeropage
                    b = a;
                    a = (a << 1) | ((status & 0x1));
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 5;
                    break;
                case 0x46: //LSR zeropage
                    setC(a & 0x1u);
                    a = a >> 1;
                    setZ(a);
                    setN(a);
                    instrProgress = 5;
                    break;
                case 0x66: //ROR zeropage
                    b = a;
                    a = (a >> 1) | ((status & 0x1u) << 7);
                    setC(b & (0x1u));
                    setZ(a);
                    setN(a);
                    instrProgress = 5;
                    break;
                case 0xC6: //DEC zeropage
                    a--;
                    setZ(a);
                    setN(a);
                    instrProgress = 5;
                    break;
                case 0xE6: //INC zeropage
                    a++;
                    setZ(a);
                    setN((a));
                    instrProgress = 5;
                    break;
                case 0x0D: //ORA absolute
                    a = memory->readMemory8(arg0);
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x0E: //ASL absolute
                case 0x2E: //ROL absolute
                case 0x4E: //LSR absolute
                case 0x6E: //ROR absolute
                case 0xCE: //DEC absolute
                case 0xEE: //INC absolute
                    a = memory->readMemory8(arg0);
                    instrProgress = 5;
                    break;
                case 0x2C: //BIT absolute
                    a = memory->readMemory8(arg0);
                    arg0 = a & acc;
                    status = (status & ~0x2) | ((arg0==0)<<1); //z bit
                    setV((a & 0x40u)>>6);
                    setN(a);
                    instrProgress = 1;
                    break;
                case 0x2D: //AND absolute
                    a = memory->readMemory8(arg0);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x4D: //EOR absolute
                    a = memory->readMemory8(arg0);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x6D: //ADC absolute
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x8C: //STY absolute
                    memory->writeMemory8(arg0, yindex);
                    instrProgress = 1;
                    break;
                case 0x8D: //STA absolute
                    memory->writeMemory8(arg0, acc);
                    instrProgress = 1;
                    break;
                case 0x8E: //STX absolute
                    memory->writeMemory8(arg0, xindex);
                    instrProgress = 1;
                    break;
                case 0xAC: //LDY absolute
                    a = memory->readMemory8(arg0);
                    yindex = a;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xAD: //LDA absolute
                    a = memory->readMemory8(arg0);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xAE: //LDX absolute
                    a = memory->readMemory8(arg0);
                    xindex = a;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xCC: //CPY absolute
                    a = memory->readMemory8(arg0);
                    setC(yindex >= a);
                    setZ(yindex - a);
                    setN((yindex - a));
                    instrProgress = 1;
                    break;
                case 0xCD: //CMP absolute
                    a = memory->readMemory8(arg0);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xEC: //CPX absolute
                    a = memory->readMemory8(arg0);
                    setC(xindex >= a);
                    setZ(xindex - a);
                    setN((xindex - a));
                    instrProgress = 1;
                    break;
                case 0xED: //SBC absolute
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x11: //ORA indirect y
                case 0x31: //AND indirect y
                case 0x51: //EOR indirect y
                case 0x71: //ADC indirect y
                case 0xB1: //LDA indirect y
                case 0xD1: //CMP indirect y
                case 0xF1: //SBC indirect y
                    arg1 |= ((uint16_t)memory->readMemory8(zeropage(arg0+1)))<<8;
                    if (pageCross(arg1, arg1 + yindex)) {
                        instrProgress = 5;
                    }else{
                        instrProgress = 6;
                    }
                    arg1 += yindex;
                    break;
                case 0x91: //STA indirect y
                    arg1 |= ((uint16_t)memory->readMemory8(arg0+1))<<8;
                    arg1 += yindex;
                    instrProgress = 5;
                    break;
                case 0x15: //ORA zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x16: //ASL zeropage x
                case 0x36: //ROL zeropage x
                case 0x56: //LSR zeropage x
                case 0x76: //ROR zeropage x
                case 0xD6: //DEC zeropage x
                case 0xF6: //INC zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    instrProgress = 5;
                    break;
                case 0x35: //AND zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x55: //EOR zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x75: //ADC zeropage x
                    b = memory->readMemory8(zpageInd(arg0, xindex));
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x94: //STY zeropage x
                    memory->writeMemory8(zpageInd(arg0, xindex), yindex);
                    instrProgress = 1;
                    break;
                case 0x95: //STA zeropage x
                    memory->writeMemory8(zpageInd(arg0, xindex), acc);
                    instrProgress = 1;
                    break;
                case 0xB4: //LDY zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    yindex = a;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xB5: //LDA zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xD5: //CMP zeropage x
                    a = memory->readMemory8(zpageInd(arg0, xindex));
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xF5: //SBC zeropage x
                    b = memory->readMemory8(zpageInd(arg0, xindex));
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x96: //STX zeropage y
                    memory->writeMemory8(zpageInd(arg0, yindex), xindex);
                    instrProgress = 1;
                    break;
                case 0xB6: //LDX zeropage y
                    a = memory->readMemory8(zpageInd(arg0, yindex));
                    xindex = a;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0x19: //ORA absolute y
                case 0x39: //AND absolute y
                case 0x59: //EOR absolute y
                case 0x79: //ADC absolute y
                case 0x99: //STA absolute y
                case 0xB9: //LDA absolute y
                case 0xBE: //LDX absolute y
                case 0xD9: //CMP absolute y
                case 0xF9: //SBC absolute y
                    instrProgress = 5;
                    break;
                case 0x1D: //ORA absolute x
                case 0x1E: //ASL absolute x
                case 0x3D: //AND absolute x
                case 0x3E: //ROL absolute x
                case 0x5D: //EOR absolute x
                case 0x5E: //LSR absolute x
                case 0x7D: //ADC absolute x
                case 0x7E: //ROR absolute x
                case 0xBC: //LDY absolute x
                case 0xBD: //LDA absolute x
                case 0xDD: //CMP absolute x
                case 0xDE: //DEC absolute x
                case 0xFD: //SBC absolute x
                case 0xFE: //INC absolute x
                    instrProgress = 5;
                    break;
                case 0x9D: //STA absolute x
                    instrProgress = 5;
                    break;
                case 0x10: //BPL relative
                case 0x30: //BMI relative
                case 0x50: //BVC relative
                case 0x70: //BVS relative
                case 0x90: //BCC relative
                case 0xB0: //BCS relative
                case 0xD0: //BNE relative
                case 0xF0: //BEQ relative
                    pc = arg1;
                    instrProgress = 1;
                    break;
                case 0x6C: //JMP absolute indirect
                    instrProgress = 5;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << int(currentOpcode)
                              << '.' << instrProgress << std::endl;
                    break;
            }
            break;
        case 5:
            switch(currentOpcode){
                case 0x00: //BRK implied/immediate
                    instrProgress = 6;
                    break;
                case 0x20: //JSR absolute
                    PUSH(((pc) & 0x00FF));
                    instrProgress = 6;
                    break;
                case 0x40: //RTI Implied/Immediate
                    pc = POP();
                    instrProgress = 6;
                    break;
                case 0x60: //RTS implied/immediate
                    pc += POP() << 8;
                    instrProgress = 6;
                    break;
                case 0x01: //ORA indirect x
                case 0x21: //AND indirect x
                case 0x41: //EOR indirect x
                case 0x61: //ADC indirect x
                case 0xA1: //LDA indirect x
                case 0x81: //STA indirect x
                case 0xC1: //CMP indirect x
                case 0xE1: //SBC indirect x
                    arg1 = memory->readMemory16(zeropage(arg0 + xindex), true);
                    instrProgress = 6;
                    break;
                case 0x06: //ASL zeropage
                case 0x26: //ROL zeropage
                case 0x46: //LSR zeropage
                case 0x66: //ROR zeropage
                case 0xC6: //DEC zeropage
                case 0xE6: //INC zeropage
                    memory->writeMemory8(zeropage(arg0), a);
                    instrProgress = 1;
                    break;
                case 0x0E: //ASL absolute
                    a = memory->readMemory8(arg0);
                    b = a;
                    a = ((int8_t) a) << 1;
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x2E: //ROL absolute
                    a = memory->readMemory8(arg0);
                    b = a;
                    a = (a << 1) | (status & 0x1);
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x4E: //LSR absolute
                    a = memory->readMemory8(arg0);
                    setC(a & 0x1u);
                    a = a >> 1;
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x6E: //ROR absolute
                    a = memory->readMemory8(arg0);
                    b = a;
                    a = (a >> 1) | ((status & 0x1u) << 7);
                    setC(b & (0x1u));
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0xCE: //DEC absolute
                    a = memory->readMemory8(arg0);
                    a--;
                    setZ(a);
                    setN((a));
                    instrProgress = 6;
                    break;
                case 0xEE: //INC absolute
                    a = memory->readMemory8(arg0);
                    a++;
                    setZ(a);
                    setN((a));
                    instrProgress = 6;
                    break;
                case 0x11: //ORA indirect y
                case 0x31: //AND indirect y
                case 0x51: //EOR indirect y
                case 0x71: //ADC indirect y
                case 0x91: //STA indirect y
                case 0xB1: //LDA indirect y
                case 0xD1: //CMP indirect y
                case 0xF1: //SBC indirect y
                    instrProgress = 6;
                    break;
                case 0x16: //ASL zeropage x
                    b = a;
                    a = ((int8_t) a) << 1;
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x36: //ROL zeropage x
                    b = a;
                    a = (a << 1) | (status & 0x1);
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x56: //LSR zeropage x
                    setC(a & 0x1u);
                    a = a >> 1;
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0x76: //ROR zeropage x
                    b = a;
                    a = (a >> 1) | ((status & 0x1u) << 7);
                    setC(b & (0x1u));
                    setZ(a);
                    setN(a);
                    instrProgress = 6;
                    break;
                case 0xD6: //DEC zeropage x
                    a--;
                    setZ(a);
                    setN((a));
                    instrProgress = 6;
                    break;
                case 0xF6: //INC zeropage x
                    a++;
                    setZ(a);
                    setN((a));
                    instrProgress = 6;
                    break;
                case 0x19: //ORA absolute y
                    a = memory->readMemory8(arg0);
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x39: //AND absolute y
                    a = memory->readMemory8(arg0);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x59: //EOR absolute y
                    a = memory->readMemory8(arg0);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x79: //ADC absolute y
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x99: //STA absolute y
                    memory->writeMemory8(arg0, acc);
                    instrProgress = 1;
                    break;
                case 0xB9: //LDA absolute y
                    a = memory->readMemory8(arg0);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xBE: //LDX absolute y
                    a = memory->readMemory8(arg0);
                    xindex = a;
                    setZ(xindex);
                    setN(xindex);
                    instrProgress = 1;
                    break;
                case 0xD9: //CMP absolute y
                    a = memory->readMemory8(arg0);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xF9: //SBC absolute y
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x1D: //ORA absolute x
                    a = memory->readMemory8(arg0);
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x1E: //ASL absolute x
                case 0x3E: //ROL absolute x
                case 0x5E: //LSR absolute x
                case 0x7E: //ROR absolute x
                case 0xDE: //DEC absolute x
                case 0xFE: //INC absolute x
                    a = memory->readMemory8(arg0);
                    instrProgress = 6;
                    break;
                case 0x3D: //AND absolute x
                    a = memory->readMemory8(arg0);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x5D: //EOR absolute x
                    a = memory->readMemory8(arg0);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x7D: //ADC absolute x
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x9D: //STA absolute x
                    memory->writeMemory8(arg0, acc);
                    instrProgress = 1;
                    break;
                case 0xBC: //LDY absolute x
                    a = memory->readMemory8(arg0);
                    yindex = a;
                    setZ(yindex);
                    setN(yindex);
                    instrProgress = 1;
                    break;
                case 0xBD: //LDA absolute x
                    a = memory->readMemory8(arg0);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xDD: //CMP absolute x
                    a = memory->readMemory8(arg0);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xFD: //SBC absolute x
                    b = memory->readMemory8(arg0);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x6C: //JMP absolute indirect
                    //add bug in the 6502 causing memory to fetch msb from wrong location on page boundary
                    if((arg0&0x00ff) == 0xff)
                        pc = (memory->readMemory8(arg0&0xff00) << 8) + memory->readMemory8(arg0);
                    else
                        pc = memory->readMemory16(arg0);
                    instrProgress = 1;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << int(currentOpcode)
                              << '.' << instrProgress << std::endl;
                    break;
            }
            break;
        case 6:
            switch(currentOpcode){
                case 0x00: //BRK implied/immediate
                    pc = memory->readMemory16(0xFFFE);
                    setB(1);
                    instrProgress = 1;
                    break;
                case 0x20: //JSR absolute
                    arg0 |= ((uint16_t)memory->readMemory8(pc))<<8;
                    pc = arg0;
                    instrProgress = 1;
                    break;
                case 0x40: //RTI Implied/Immediate
                    pc += POP() << 8;
                    instrProgress = 1;
                    break;
                case 0x60: //RTS implied/immediate
                    pc += 1;
                    instrProgress = 1;
                    break;
                case 0x01: //ORA indirect x
                    a = memory->readMemory8(arg1);
                    acc |= a;
                    setN(acc);
                    setZ(acc);
                    instrProgress = 1;
                    break;
                case 0x21: //AND indirect x
                    a = memory->readMemory8(arg1);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x41: //EOR indirect x
                    a = memory->readMemory8(arg1);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x61: //ADC indirect x
                    b = memory->readMemory8(arg1);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x81: //STA indirect x
                    memory->writeMemory8(arg1, acc);
                    instrProgress = 1;
                    break;
                case 0xA1: //LDA indirect x
                    a = memory->readMemory8(arg1);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xC1: //CMP indirect x
                    a = memory->readMemory8(arg1);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xE1: //SBC indirect x
                    b = memory->readMemory8(arg1);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x0E: //ASL absolute
                case 0x2E: //ROL absolute
                case 0x4E: //LSR absolute
                case 0x6E: //ROR absolute
                case 0xCE: //DEC absolute
                case 0xEE: //INC absolute
                    memory->writeMemory8(arg0, a);
                    instrProgress = 1;
                    break;
                case 0x11: //ORA indirect y
                    a = memory->readMemory8(arg1);
                    acc |= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x31: //AND indirect y
                    a = memory->readMemory8(arg1);
                    acc &= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x51: //EOR indirect y
                    a = memory->readMemory8(arg1);
                    acc ^= a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x71: //ADC indirect y
                    b = memory->readMemory8(arg1);
                    a = acc;
                    checkV = a + b + (status & 0x1u);
                    acc = checkV;

                    setV(isOverflowAdd(a, b, acc));
                    setC(acc < ((uint32_t) checkV));
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0x91: //STA indirect y
                    memory->writeMemory8(arg1, acc);
                    instrProgress = 1;
                    break;
                case 0xB1: //LDA indirect y
                    a = memory->readMemory8(arg1);
                    acc = a;
                    setZ(acc);
                    setN(acc);
                    instrProgress = 1;
                    break;
                case 0xD1: //CMP indirect y
                    a = memory->readMemory8(arg1);
                    setC(acc >= a);
                    setZ(acc - a);
                    setN((acc - a));
                    instrProgress = 1;
                    break;
                case 0xF1: //SBC indirect y
                    b = memory->readMemory8(arg1);
                    a = acc;
                    checkVUnsigned = (a - b - 1 + (status & 0x1u));
                    checkV =(int) (((int8_t) a) - ((int8_t) b) - 1 + (status & 0x1u));
                    acc = checkV;

                    setV((checkV > ((int8_t) acc)) || (checkV < ((int8_t) acc)));
                    setC( !(((int) checkVUnsigned) < 0));
                    setZ(acc);
                    setN((acc));
                    instrProgress = 1;
                    break;
                case 0x16: //ASL zeropage x
                case 0x36: //ROL zeropage x
                case 0x56: //LSR zeropage x
                case 0x76: //ROR zeropage x
                case 0xD6: //DEC zeropage x
                case 0xF6: //INC zeropage x
                    memory->writeMemory8(zpageInd(arg0, xindex), a);
                    instrProgress = 1;
                    break;
                case 0x1E: //ASL absolute x
                    b = a;
                    a = ((int8_t) a) << 1;
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 7;
                    break;
                case 0x3E: //ROL absolute x
                    b = a;
                    a = (a << 1) | (status & 0x1);
                    setC((b & 0x80u) >> 7);
                    setZ(a);
                    setN(a);
                    instrProgress = 7;
                    break;
                case 0x5E: //LSR absolute x
                    setC(a & 0x1u);
                    a = a >> 1;
                    setZ(a);
                    setN(a);
                    instrProgress = 7;
                    break;
                case 0x7E: //ROR absolute x
                    b = a;
                    a = (a >> 1) | ((status & 0x1u) << 7);
                    setC(b & (0x1u));
                    setZ(a);
                    setN(a);
                    instrProgress = 7;
                    break;
                case 0xDE: //DEC absolute x
                    a--;
                    setZ(a);
                    setN((a));
                    instrProgress = 7;
                    break;
                case 0xFE: //INC absolute x
                    a++;
                    setZ(a);
                    setN((a));
                    instrProgress = 7;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << int(currentOpcode)
                              << '.' << instrProgress << std::endl;
                    break;
            }
            break;
        case 7:
            switch(currentOpcode){
                case 0x1E: //ASL absolute x
                case 0x3E: //ROL absolute x
                case 0x5E: //LSR absolute x
                case 0x7E: //ROR absolute x
                case 0xDE: //DEC absolute x
                case 0xFE: //INC absolute x
                    memory->writeMemory8(arg0 + xindex, a);
                    instrProgress = 1;
                    break;
                default:
                    std::cerr << "Unknown opcode: " << std::hex << int(currentOpcode)
                              << '.' << instrProgress << std::endl;
                    break;
            }
            break;
    }
}

void CPU6502::loadRom() {
    this->pc = this->memory->readMemory16(0xFFFC);
//    this->pc = 0xc000;
}

void CPU6502::printStatus() const{
    printf("%04x A:%02x X:%02x Y:%02x P:%02x SP:%02x CYC:%lld\n", pc, acc, xindex, yindex, status, sp, cpuTime/15 + 8);
}

void CPU6502::doNMI(){

    PUSH(((pc & 0xFF00)>>8));
    PUSH((pc & 0x00FF));
    PUSH(status);
    this->pc = this->memory->readMemory16(0xFFFA);
}

void CPU6502::setMemory(CPUMemory *memory) {
    this->memory = memory;
}

void CPU6502::startOAMDMA(uint8_t DMAArgIn){
    DMACycleNum = 0;
    this->DMAArg = DMAArgIn;
}

void CPU6502::doDMACycles(uint64_t runTo){
    while(DMACycleNum < 513 && cpuTime < runTo){
        if(DMACycleNum == 0){
            cpuInc(1);
            ++DMACycleNum;
        }else{
            memory->writeDMA(DMAArg, DMACycleNum/2);
            cpuInc(2);
            DMACycleNum += 2;
        }
    }
}
