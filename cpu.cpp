#include <cstdint>
#include <cstdio>
#include "memory.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma clang diagnostic pop

#define PCL pc&0xFF
#define PCH (pc&0xFF00)>>8

#define zeropage(arg)       (arg)%256
#define zpageInd(arg,index) (arg + index)%256
#define indIndir(arg) ( readMemory8((arg + xindex)% 256) + readMemory8((arg + xindex + 1)%256) )*256
#define indirInd(arg) (readMemory8(arg) + readMemory8((arg + 1) % 256) * 256 + yindex)

#define setC(arg) (status = (status&0xFEu) | (arg))
#define setZ(arg) (status = (status&0xFDu) | (arg==0)<<1)
#define setI(arg) (status = (status&0xFBu) | (arg<<2))
#define setD(arg) (status = (status&0xF7u) | (arg<<3))
#define setB(arg) (status = (status&0xEFu) | (arg<<4))
#define setV(arg) (status = (status&0xBFu) | (arg<<6))
#define setN(arg) (status = (status&0x7Fu) | (arg&0x80u))

#define PUSH(arg) (writeMemory8(sp--,arg))
#define POP(arg)  (readMemory8(sp++))

#define pageCross(arg1,arg2) ((arg1/256) != (arg2/256))

#define cpuInc(arg) cpuTime += 15 * arg

uint8_t acc;
uint8_t xindex;
uint8_t yindex;
uint8_t status; //flags bit 7-0: N,V,1(unused),B,D,I,Z,C
uint8_t sp;
uint16_t pc;

uint32_t cpuTime;

void InitCPU(){
    cpuTime = 0;

    status = 0x34u;
    acc = 0;
    xindex = 0;
    yindex = 0;
    sp = 0xFDu;
    pc = 0x600;
    InitMemory();
}


void cycleCPU(int runTo) {
    int32_t checkV;
    uint16_t arg0;
    uint8_t a;
    uint8_t b;
    while (runTo > cpuTime) {
        uint8_t opcode = readMemory8(pc);
        switch (opcode) {                          //TODO debug cases
            case 0x00: //BRK implied/immediate
                status |= 0x10u;
                PUSH(pc);
                PUSH(status);
                pc = readMemory16(0xFFFE);
                setB(1);
                cpuInc(7);
                break;

            case 0x01:; //ORA indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indIndir(arg0));
                acc |= a;
                setN(acc);
                setZ(acc);
                cpuTime += 15;
                cpuInc(6);
                pc += 2;
                break;
            case 0x05: //ORA zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                acc |= a;
                setZ(acc);
                setN(acc);
                cpuInc(3);
                pc += 2;
                break;
            case 0x06: //ASL zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(a & 0x7Fu);
                a = ((int8_t) a) << 1;
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);
                cpuInc(5);
                pc += 2;
                break;
            case 0x08: //PHP implied
                PUSH(status);
                cpuInc(3);
                pc += 1;
                break;
            case 0x09: //ORA immediate
                arg0 = readMemory8(pc + 1);
                acc |= arg0;
                setZ(acc);
                setN(acc);
                cpuInc(2);
                pc += 2;
                break;
            case 0x0A: //ASL Acc/Implied
                setC(acc & 0x7Fu);
                acc = ((int8_t) acc) << 1;
                setZ(acc);
                setN(acc);
                cpuInc(2);
                pc += 2;
                break;
            case 0x0D: //ORA absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                acc |= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);
                pc += 3;
                break;
            case 0x0E: //ASL absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(a & 0x7Fu);
                a = ((int8_t) a) << 1;
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);
                pc += 3;
                break;
            case 0x10: //BPL relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if (((int8_t) status) > 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);
                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);
                break;
            case 0x11: //ORA indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                acc |= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);
                }
                cpuInc(5);
                pc += 2;
                break;
            case 0x15: //ORA zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                acc |= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);
                pc += 2;
                break;
            case 0x16: //ASL zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                setC(a & 0x7Fu);
                a = ((int8_t) a) << 1;
                setZ(a);
                setN(a);
                writeMemory8(zpageInd(arg0, xindex), a);
                cpuInc(6);
                pc += 2;
                break;
            case 0x18: //CLC implied
                setC(0);
                cpuInc(2);
                pc += 1;
                break;
            case 0x19: //ORA absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                acc |= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);
                }
                cpuInc(4);
                pc += 3;
                break;
            case 0x1D: //ORA absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                acc |= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);
                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x1E: //ASL absolute x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(arg0 + xindex);
                setC(a & 0x7Fu);
                a = ((int8_t) a) << 1;
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;

            case 0x20: //JSR absolute
                arg0 = readMemory16(pc + 1);
                pc = arg0;
                cpuInc(6);

                break;
            case 0x21: //AND indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indIndir(arg0));
                acc &= a;
                setZ(acc);
                setN(acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0x24: //BIT zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                arg0 = a & acc;
                setZ(arg0);
                setV(a & 0xBEu);
                setV(a);
                cpuInc(3);

                pc += 2;
                break;
            case 0x25: //AND zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                acc &= a;
                setZ(acc);
                setN(acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0x26: //ROL zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC((a & 0x80u) >> 7);
                a = (a << 1) | ((a & 0x80u) >> 7);
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);

                pc += 2;
                break;
            case 0x28: //PLP implied
                status = POP();
                cpuInc(4);

                pc += 1;
                break;
            case 0x29: //AND immediate
                arg0 = readMemory8(pc + 1);
                acc &= arg0;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 2;
                break;
            case 0x2A: //ROL acc/Implied
                setC((acc & 0x80u) >> 7);
                acc = (acc << 1) | ((acc & 0x80u) >> 7);
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 1;
                break;
            case 0x2C: //BIT absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                arg0 = a & acc;
                setZ(arg0);
                setV(a & 0xBEu);
                setV(a);
                cpuInc(4);

                pc += 3;
                break;
            case 0x2D: //AND absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                acc &= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0x2E: //ROL absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC((a & 0x80u) >> 7);
                a = (a << 1) | ((a & 0x80u) >> 7);
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);

                pc += 3;
                break;
            case 0x30: //BMI relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if (((int8_t) status) < 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0x31: //AND indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                acc &= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0x35: //AND zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                acc &= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 2;
                break;
            case 0x36: //ROL zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                setC((a & 0x80u) >> 7);
                a = (a << 1) | ((a & 0x80u) >> 7);
                setZ(a);
                setN(a);
                cpuInc(6);

                pc += 2;
                break;
            case 0x38: //SEC implied
                setC(1);
                cpuInc(2);

                pc += 1;
                break;
            case 0x39: //AND absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                acc &= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x3D: //AND absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                acc &= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x3E: //ROL absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                setC((a & 0x80u) >> 7);
                a = (a << 1) | ((a & 0x80u) >> 7);
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;

            case 0x40: //RTI Implied/Immediate
                status = POP();
                pc = POP();
                cpuInc(6);

                break;
            case 0x41: //EOR indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indIndir(arg0));
                acc ^= a;
                setZ(acc);
                setN(acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0x45: //EOR zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                acc ^= a;
                setZ(acc);
                setN(acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0x46: //LSR zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(a & 0x1u);
                a = a >> 1;
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);

                pc += 2;
                break;
            case 0x48: //PHA implied
                PUSH(acc);
                cpuInc(3);

                pc += 1;
                break;
            case 0x49: //EOR immediate
                arg0 = readMemory8(pc + 1);
                acc ^= arg0;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 2;
                break;
            case 0x4A: //LSR acc/Implied
                setC(acc & 0x1u);
                acc = acc >> 1;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 1;
                break;
            case 0x4C: //JMP absolute
                arg0 = readMemory16(pc + 1);
                pc = arg0;
                cpuInc(3);

                break;
            case 0x4D: //EOR absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                acc ^= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0x4E: //LSR absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(a & 0x1u);
                a = a >> 1;
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);

                pc += 3;
                break;
            case 0x50: //BVC relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x40u) == 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0x51: //EOR indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                acc ^= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0x55: //EOR zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                acc ^= a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 2;
                break;
            case 0x56: //LSR zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                setC(a & 0x1u);
                a = a >> 1;
                setZ(a);
                setN(a);
                writeMemory8(zpageInd(arg0, xindex), a);
                cpuInc(6);

                pc += 2;
                break;
            case 0x58: //CLI implied
                setI(0);
                cpuInc(2);

                pc += 1;
                break;
            case 0x59: //EOR absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                acc ^= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x5D: //EOR absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                acc ^= a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x5E: //LSR absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                setC(a & 0x1u);
                a = a >> 1;
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;

            case 0x60: //RTS implied/immediate
                pc = POP() - 1;
                cpuInc(6);

                break;
            case 0x61: //ADC indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indIndir(arg0));
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0x65: //ADC zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0x66: //ROR zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(a & (0x1u));
                a = (a >> 1) || ((status & 0x1u) << 7);
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);

                pc += 2;
                break;
            case 0x68: //PLA implied
                acc = POP();
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 1;
                break;
            case 0x69: //ADC immediate
                a = readMemory8(pc + 1);
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 2;
                break;
            case 0x6A: //ROR acc/Implied
                setC(acc & (0x1u));
                acc = (acc >> 1) || ((status & 0x1u) << 7);
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 1;
                break;
            case 0x6C: //JMP indirect
                arg0 = readMemory16(pc + 1);
                pc = readMemory16(arg0);

                break;
            case 0x6D: //ADC absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0x6E: //ROR absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(a & (0x1u));
                a = (a >> 1) || ((status & 0x1u) << 7);
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);

                pc += 3;
                break;
            case 0x70: //BVS relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x40u) != 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0x71: //ADC indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0x75: //ADC zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, zpageInd(arg0, xindex))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0x76: //ROR zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                setC(a & (0x1u));
                a = (a >> 1) || ((status & 0x1u) << 7);
                setZ(a);
                setN(a);
                writeMemory8(zpageInd(arg0, xindex), a);
                cpuInc(6);

                pc += 2;
                break;
            case 0x78: //SEI implied
                setI(1);
                cpuInc(2);

                pc += 1;
                break;
            case 0x79: //ADC absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x7D: //ADC absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                b = acc;
                checkV = acc + a + (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b > acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0x7E: //ROR absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                setC(a & (0x1u));
                a = (a >> 1) || ((status & 0x1u) << 7);
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;

            case 0x81: //STA indirect x
                arg0 = readMemory8(pc + 1);
                writeMemory8(indIndir(arg0), acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0x84: //STY zeropage
                arg0 = readMemory8(pc + 1);
                writeMemory8(zeropage(arg0), yindex);
                cpuInc(3);

                pc += 2;
                break;
            case 0x85: //STA zeropage
                arg0 = readMemory8(pc + 1);
                writeMemory8(zeropage(arg0), acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0x86: //STX zeropage
                arg0 = readMemory8(pc + 1);
                writeMemory8(zeropage(arg0), xindex);
                cpuInc(3);

                pc += 2;
                break;
            case 0x88: //DEY implied
                yindex -= 1;
                setZ(yindex);
                setN(yindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0x8A: //TXA acc/Implied
                acc = xindex;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 1;
                break;
            case 0x8C: //STY absolute
                arg0 = readMemory16(pc + 1);
                writeMemory8(arg0, yindex);
                cpuInc(4);

                pc += 3;
                break;
            case 0x8D: //STA absolute
                arg0 = readMemory16(pc + 1);
                writeMemory8(arg0, acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0x8E: //STX absolute
                arg0 = readMemory16(pc + 1);
                writeMemory8(arg0, xindex);
                cpuInc(4);

                pc += 3;
                break;
            case 0x90: //BCC relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x1) == 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0x91: //STA indirect y
                arg0 = readMemory8(pc + 1);
                writeMemory8(indirInd(arg0), acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0x94: //STY zeropage x
                arg0 = readMemory8(pc + 1);
                writeMemory8(zpageInd(arg0, xindex), yindex);
                cpuInc(4);

                pc += 2;
                break;
            case 0x95: //STA zeropage x
                arg0 = readMemory8(pc + 1);
                writeMemory8(zpageInd(arg0, xindex), acc);
                cpuInc(4);

                pc += 2;
                break;
            case 0x96: //STX zeropage y
                arg0 = readMemory8(pc + 1);
                writeMemory8(zpageInd(arg0, yindex), xindex);
                cpuInc(4);

                pc += 2;
                break;
            case 0x98: //TYA implied
                acc = yindex;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 1;
                break;
            case 0x99: //STA absolute y
                arg0 = readMemory16(pc + 1);
                writeMemory8(arg0 + yindex, acc);

                pc += 3;
                break;
            case 0x9A: //TXS Implied
                sp = xindex;
                cpuInc(2);

                pc += 1;
                break;
            case 0x9D: //STA absolute x
                arg0 = readMemory16(pc + 1);
                writeMemory8(arg0 + xindex, acc);

                pc += 3;
                break;

            case 0xA0: //LDY Implied/Immediate
                arg0 = readMemory8(pc + 1);
                yindex = arg0;
                setZ(yindex);
                setN(yindex);
                cpuInc(2);

                pc += 2;
                break;
            case 0xA1: //LDA indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                acc = a;
                setZ(acc);
                setN(acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0xA2: //LDX ?/Immediate
                arg0 = readMemory8(pc + 1);
                xindex = arg0;
                setZ(xindex);
                setN(xindex);
                cpuInc(2);

                pc += 2;
                break;
            case 0xA4: //LDY zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                yindex = a;
                setZ(yindex);
                setN(yindex);
                cpuInc(3);

                pc += 2;
                break;
            case 0xA5: //LDA zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                acc = a;
                setZ(acc);
                setN(acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0xA6: //LDX zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                xindex = a;
                setZ(xindex);
                setN(xindex);
                cpuInc(3);

                pc += 2;
                break;
            case 0xA8: //TAY implied
                yindex = acc;
                setZ(yindex);
                setN(yindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0xA9: //LDA Immediate
                arg0 = readMemory8(pc + 1);
                acc = arg0;
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 2;
                break;
            case 0xAA: //TAX acc/Implied
                xindex = acc;
                setZ(xindex);
                setN(xindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0xAC: //LDY absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                yindex = a;
                setZ(yindex);
                setN(yindex);
                cpuInc(4);

                pc += 3;
                break;
            case 0xAD: //LDA absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                acc = a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0xAE: //LDX absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                xindex = a;
                setZ(xindex);
                setN(xindex);
                cpuInc(4);

                pc += 3;
                break;
            case 0xB0: //BCS relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x1) != 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0xB1: //LDA indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                acc = a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0xB4: //LDY zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                yindex = a;
                setZ(yindex);
                setN(yindex);
                cpuInc(4);

                pc += 2;
                break;
            case 0xB5: //LDA zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                acc = a;
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 2;
                break;
            case 0xB6: //LDX zeropage y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, yindex));
                xindex = a;
                setZ(xindex);
                setN(xindex);
                cpuInc(4);

                pc += 2;
                break;
            case 0xB8: //CLV implied
                setV(0);
                cpuInc(2);

                pc += 1;
                break;
            case 0xB9: //LDA absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                acc = a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xBA: //TSX absolute y
                xindex = sp;
                setZ(xindex);
                setN(xindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0xBC: //LDY absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                yindex = a;
                setZ(yindex);
                setN(yindex);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xBD: //LDA absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                acc = a;
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xBE: //LDX absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                yindex = a;
                setZ(yindex);
                setN(yindex);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;

            case 0xC0: //CPY Implied/Immediate
                arg0 = readMemory8(pc + 1);
                setC(yindex >= arg0);
                setZ(yindex - arg0);
                setN(((yindex - arg0) & 0x80u) != 0);
                cpuInc(2);

                pc += 2;
                break;
            case 0xC1: //CMP indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                cpuInc(6);

                pc += 2;
                break;
            case 0xC4: //CPY zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(yindex >= a);
                setZ(yindex - a);
                setN(((yindex - a) & 0x80u) != 0);
                cpuInc(3);

                pc += 2;
                break;
            case 0xC5: //CMP zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                cpuInc(3);

                pc += 2;
                break;
            case 0xC6: //DEC zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                a--;
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);

                pc += 2;
                break;
            case 0xC8: //INY implied
                yindex++;
                setZ(yindex);
                setN(yindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0xC9: //CMP immediate
                arg0 = readMemory8(pc + 1);
                setC(acc >= arg0);
                setZ(acc - arg0);
                setN(((acc - arg0) & 0x80u) != 0);
                cpuInc(2);

                pc += 2;
                break;
            case 0xCA: //DEX acc/Implied
                xindex--;
                setZ(xindex);
                setN(xindex);
                cpuInc(2);

                pc += 1;
                break;
            case 0xCC: //CPY absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(yindex >= a);
                setZ(yindex - a);
                setN(((yindex - a) & 0x80u) != 0);
                cpuInc(4);

                pc += 3;
                break;
            case 0xCD: //CMP absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                cpuInc(4);

                pc += 3;
                break;
            case 0xCE: //DEC absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                a--;
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);

                pc += 3;
                break;
            case 0xD0: //BNE relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x2) == 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0xD1: //CMP indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }

                pc += 2;
                break;
            case 0xD5: //CMP zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                cpuInc(4);

                pc += 2;
                break;
            case 0xD6: //DEC zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                a--;
                setZ(a);
                setN(a);
                writeMemory8(zpageInd(arg0, xindex), a);
                cpuInc(6);

                pc += 2;
                break;
            case 0xD8: //CLD implied
                setD(0);
                cpuInc(2);

                pc += 1;
                break;
            case 0xD9: //CMP absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xDD: //CMP absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                setC(acc >= a);
                setZ(acc - a);
                setN(((acc - a) & 0x80u) != 0);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xDE: //DEC absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                a--;
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;

            case 0xE0: //CPX Implied/Immediate
                arg0 = readMemory8(pc + 1);
                setC(xindex >= arg0);
                setZ(xindex - arg0);
                setN(((xindex - arg0) & 0x80u) != 0);
                cpuInc(2);

                pc += 2;
                break;
            case 0xE1: //SBC indirect x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indIndir(arg0));
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                cpuInc(6);

                pc += 2;
                break;
            case 0xE4: //CPX zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                setC(xindex >= a);
                setZ(xindex - a);
                setN(((xindex - a) & 0x80u) != 0);
                cpuInc(3);

                pc += 2;
                break;
            case 0xE5: //SBC zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                cpuInc(3);

                pc += 2;
                break;
            case 0xE6: //INC zeropage
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zeropage(arg0));
                a++;
                setZ(a);
                setN(a);
                writeMemory8(zeropage(arg0), a);
                cpuInc(5);

                pc += 2;
                break;
            case 0xE8: //INX implied
                xindex++;
                setZ(xindex);
                setN(xindex);
                cpuInc(5);

                pc += 2;
                break;
            case 0xE9: //SBC immediate
                arg0 = readMemory8(pc + 1);
                b = acc;
                checkV = acc - arg0 + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                cpuInc(2);

                pc += 2;
                break;
            case 0xEA: //NOP acc/Implied
                cpuInc(2);

                pc += 1;
                break;
            case 0xEC: //CPX absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                setC(xindex >= a);
                setZ(xindex - a);
                setN(((xindex - a) & 0x80u) != 0);
                cpuInc(4);

                pc += 3;
                break;
            case 0xED: //SBC absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 3;
                break;
            case 0xEE: //INC absolute
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0);
                a++;
                setZ(a);
                setN(a);
                writeMemory8(arg0, a);
                cpuInc(6);

                pc += 3;
                break;
            case 0xF0: //BEQ relative
                arg0 = readMemory8(pc + 1);
                pc += 2;
                if ((status & 0x2u) != 0) {
                    if (pageCross(pc, pc + ((int8_t) arg0))) {
                        cpuInc(1);

                    } //new page penalty
                    pc += (int8_t) arg0;
                    cpuInc(1);

                }
                cpuInc(2);

                break;
            case 0xF1: //SBC indirect y
                arg0 = readMemory8(pc + 1);
                a = readMemory8(indirInd(arg0));
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, indirInd(arg0))) {
                    cpuInc(1);

                }
                cpuInc(5);

                pc += 2;
                break;
            case 0xF5: //SBC zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                cpuInc(4);

                pc += 2;
                break;
            case 0xF6: //INC zeropage x
                arg0 = readMemory8(pc + 1);
                a = readMemory8(zpageInd(arg0, xindex));
                a++;
                setZ(a);
                setN(a);
                writeMemory8(zpageInd(arg0, xindex), a);
                cpuInc(6);

                pc += 2;
                break;
            case 0xF8: //SED implied
                setD(1);
                cpuInc(2);

                pc += 1;
                break;
            case 0xF9: //SBC absolute y
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + yindex);
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + yindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xFD: //SBC absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                b = acc;
                checkV = acc - a + 1 - (status & 0x1u);
                acc = checkV;
                setV((checkV > 127) || (checkV < -128));
                setC(b < acc);
                setZ(acc);
                setN(acc);
                if (pageCross(pc, arg0 + xindex)) {
                    cpuInc(1);

                }
                cpuInc(4);

                pc += 3;
                break;
            case 0xFE: //INC absolute x
                arg0 = readMemory16(pc + 1);
                a = readMemory8(arg0 + xindex);
                a++;
                setZ(a);
                setN(a);
                writeMemory8(arg0 + xindex, a);
                cpuInc(7);

                pc += 3;
                break;
            default:
                printf("Unknown opcode!\n");
                break;

        }
    }
}


