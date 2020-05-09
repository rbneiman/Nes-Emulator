#include <stdint.h>
#include "MyMemory.h"

uint8_t acc;
uint8_t xindex;
uint8_t yindex;
int8_t status; //flags bit 7-0: N,V,1(unused),B,D,I,Z,C
uint8_t sp;
uint16_t pc;

void init(){

}

void cycleCPU(int numClocks){

    while(numClocks>0){
        uint8_t opcode = readMemory8(pc);
        switch(opcode){
            case 0x00: //BRK implied/immediate
                status |= 16;
                //insert stack stuff here
                pc = readMemory16(0xFFFE);
                numClocks -= 7;
            case 0x01: //ORA indirect x
                uint8_t arg = readMemory8(pc + 1);
                int8_t a = readMemory8(readMemory8(arg + xindex)% 256) + readMemory8((arg + xindex + 1) %256)*256);
                if();
            case 0x05: //ORA zeropage
                ;
            case 0x06: //ASL zeropage
                ;
            case 0x08: //PHP implied
                ;
            case 0x09: //ORA immediate
                ;
            case 0x0A: //ASL Acc/Implied
                ;
            case 0x0D: //ORA absolute
                ;
            case 0x0E: //ASL absolute
                ;
            case 0x10: //BPL relative
                ;
            case 0x11: //ORA indirect y
                ;
            case 0x15: //ORA zeropage x
                ;
            case 0x16: //ASL zeropage x
                ;
            case 0x18: //CLC implied
                ;
            case 0x19: //ORA absolute y
                ;
            case 0x1D: //ORA absolute x
                ;
            case 0x1E: //ASL absolute x
                ;

            case 0x20: //JSR implied/immediate
                ;
            case 0x21: //AND indirect x
                ;
            case 0x24: //BIT zeropage
                ;
            case 0x25: //AND zeropage
                ;
            case 0x26: //ROL zeropage
                ;
            case 0x28: //PLP implied
                ;
            case 0x29: //AND immediate
                ;
            case 0x2A: //ROL acc/Implied
                ;
            case 0x2C: //BIT absolute
                ;
            case 0x2D: //AND absolute
                ;
            case 0x2E: //ROL absolute
                ;
            case 0x30: //BMI relative
                ;
            case 0x31: //AND indirect y
                ;
            case 0x35: //AND zeropage x
                ;
            case 0x36: //ROL zeropage x
                ;
            case 0x38: //SEC implied
                ;
            case 0x39: //AND absolute y
                ;
            case 0x3D: //AND absolute x
                ;
            case 0x3E: //ROL absolute x
                ;

            case 0x40: //RTI Implied/Immediate
                ;
            case 0x41: //EOR indirect x
                ;
            case 0x45: //EOR zeropage
                ;
            case 0x46: //LSR zeropage
                ;
            case 0x48: //PHA implied
                ;
            case 0x49: //EOR immediate
                ;
            case 0x4A: //LSR acc/Implied
                ;
            case 0x4C: //JMP absolute
                ;
            case 0x4D: //EOR absolute
                ;
            case 0x4E: //LSR absolute
                ;
            case 0x50: //BVC relative
                ;
            case 0x51: //EOR indirect y
                ;
            case 0x55: //EOR zeropage x
                ;
            case 0x56: //LSR zeropage x
                ;
            case 0x58: //CLI implied
                ;
            case 0x59: //EOR absolute y
                ;
            case 0x5D: //EOR absolute x
                ;
            case 0x5E: //LSR absolute x
                ;

            case 0x60: //RTS implied/immediate
                ;
            case 0x61: //ADC indirect x
                ;
            case 0x65: //ADC zeropage
                ;
            case 0x66: //ROR zeropage
                ;
            case 0x68: //PLA implied
                ;
            case 0x69: //ADC immediate
                ;
            case 0x6A: //ROR acc/Implied
                ;
            case 0x6C: //JMP indirect
                ;
            case 0x6D: //ADC absolute
                ;
            case 0x6E: //ROR absolute
                ;
            case 0x70: //BVS relative
                ;
            case 0x71: //ADC indirect y
                ;
            case 0x75: //ADC zeropage x
                ;
            case 0x76: //ROR zeropage x
                ;
            case 0x78: //SEI implied
                ;
            case 0x79: //ADC absolute y
                ;
            case 0x7D: //ADC absolute x
                ;
            case 0x7E: //ROR absolute x
                ;

            case 0x81: //STA indirect x
                ;
            case 0x84: //STY zeropage
                ;
            case 0x85: //STA zeropage
                ;
            case 0x86: //STX zeropage
                ;
            case 0x88: //DEY implied
                ;
            case 0x8A: //TXA acc/Implied
                ;
            case 0x8C: //STY absolute
                ;
            case 0x8D: //STA absolute
                ;
            case 0x8E: //STX absolute
                ;
            case 0x90: //BCC relative
                ;
            case 0x91: //STA indirect y
                ;
            case 0x94: //STY zeropage x
                ;
            case 0x95: //STA zeropage x
                ;
            case 0x96: //STX zeropage y
                ;
            case 0x98: //TYA implied
                ;
            case 0x99: //STA absolute y
                ;
            case 0x9A: //TXS absolute y
                ;
            case 0x9D: //STA absolute x
                ;

            case 0xA0: //LDY Implied/Immediate
                ;
            case 0xA1: //LDA indirect x
                ;
            case 0xA2: //LDX ?/Immediate
                ;
            case 0xA4: //LDY zeropage
                ;
            case 0xA5: //LDA zeropage
                ;
            case 0xA6: //LDX zeropage
                ;
            case 0xA8: //TAY implied
                ;
            case 0xA9: //LDA Immediate
                ;
            case 0xAA: //TAX acc/Implied
                ;
            case 0xAC: //LDY absolute
                ;
            case 0xAD: //LDA absolute
                ;
            case 0xAE: //LDX absolute
                ;
            case 0xB0: //BCS relative
                ;
            case 0xB1: //LDA indirect y
                ;
            case 0xB4: //LDY zeropage x
                ;
            case 0xB5: //LDA zeropage x
                ;
            case 0xB6: //STX zeropage y
                ;
            case 0xB8: //CLV implied
                ;
            case 0xB9: //LDA absolute y
                ;
            case 0xBA: //TSX absolute y
                ;
            case 0xBC: //LDY absolute x
                ;
            case 0xBD: //LDA absolute x
                ;
            case 0xBE: //LDX absolute y
                ;

            case 0xC0: //CPY Implied/Immediate
                ;
            case 0xC1: //CMP indirect x
                ;
            case 0xC4: //CPY zeropage
                ;
            case 0xC5: //CMP zeropage
                ;
            case 0xC6: //DEC zeropage
                ;
            case 0xC8: //INY implied
                ;
            case 0xC9: //CMP immediate
                ;
            case 0xCA: //DEX acc/Implied
                ;
            case 0xCC: //CPY absolute
                ;
            case 0xCD: //CMP absolute
                ;
            case 0xCE: //DEC absolute
                ;
            case 0xD0: //BNE relative
                ;
            case 0xD1: //CMP indirect y
                ;
            case 0xD5: //CMP zeropage x
                ;
            case 0xD6: //DEC zeropage x
                ;
            case 0xD8: //CLD implied
                ;
            case 0xD9: //CMP absolute y
                ;
            case 0xDD: //CMP absolute x
                ;
            case 0xDE: //DEC absolute x
                ;

            case 0xE0: //CPX Implied/Immediate
                ;
            case 0xE1: //SBC indirect x
                ;
            case 0xE4: //CPX zeropage
                ;
            case 0xE5: //SBC zeropage
                ;
            case 0xE6: //INC zeropage
                ;
            case 0xE8: //INX implied
                ;
            case 0xE9: //SBC immediate
                ;
            case 0xEA: //NOP acc/Implied
                ;
            case 0xEC: //CPX absolute
                ;
            case 0xED: //SBC absolute
                ;
            case 0xEE: //INC absolute
                ;
            case 0xF0: //BEQ relative
                ;
            case 0xF1: //SBC indirect y
                ;
            case 0xF5: //SBC zeropage x
                ;
            case 0xF6: //INC zeropage x
                ;
            case 0xF8: //SED implied
                ;
            case 0xF9: //SBC absolute y
                ;
            case 0xFD: //SBC absolute x
                ;
            case 0xFE: //INC absolute x
                ;

        }
    }
}
