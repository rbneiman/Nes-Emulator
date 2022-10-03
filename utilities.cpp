#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include "utilities.h"


int checkBit(uint8_t a, uint8_t bit){ //true if bit 1
    return (a && (1<<bit)) > 0;
}

int abs(int a){
    if(a<0){return -a;}
    else{ return a;}
}

std::vector<char> loadFile(const std::string& filepath){
    std::ifstream inf(filepath, std::ios::ate | std::ios::binary);
    if(!inf)
        throw std::runtime_error(filepath + ": " + std::strerror(errno));

    auto end = inf.tellg();
    inf.seekg(0, std::ios::beg);
    auto size = std::size_t(end - inf.tellg());

    std::vector<char> out(size+1);

    inf.read(out.data(), size);
    out[size] = 0;

    inf.close();
    return out;
}

/*
a>0 b>0:
	c>0: no
	c<0: yes
a<0 b>0:
	c>0: no
	c<0: no
a>0 b<0:
	c>0: no
	c<0: no
a<0 b<0:
	c>0: yes
	c<0: no
 */
bool isOverflowAdd(int8_t a, int8_t b, int8_t result){
    bool bothPos = (a>0) && (b>0);
    bool bothNeg = (a<0) && (b<0);
    return (bothPos && (result<0)) || (bothNeg && (result>0));
}
/*
a>0 b>0:
	c>0: no
	c<0: no
a<0 b>0:
	c>0: yes
	c<0: no
a>0 b<0:
	c>0: no
	c<0: yes
a<0 b<0:
	c>0: no
	c<0: no
 */
bool isOverflowSubtract(int8_t a, int8_t b, int8_t result){
    bool case1 = (a<0) && (b>0);
    bool case2 = (a>0) && (b<0);
    return (case1 && (result>0)) || (case2 && (result<0));
}

unsigned bitSlice(unsigned num, unsigned high, unsigned low){
    unsigned mask = (1 << (high+1)) - 1;
    return (num & mask) >> low;
}

void DebugLogFile::parseLine(const std::string &line) {
    if(line[0] == '[')
        return;

    uint64_t cycles;
    uint16_t pc;
    uint8_t acc, xIndex, yIndex, status, sp;
    pc = std::stoi(line, nullptr, 16);
    acc = std::stoi(line.c_str() + 7, nullptr, 16);
    xIndex = std::stoi(line.c_str() + 12, nullptr, 16);
    yIndex = std::stoi(line.c_str() + 17, nullptr, 16);
    status = std::stoi(line.c_str() + 22, nullptr, 16);
    sp = std::stoi(line.c_str() + 28, nullptr, 16);
    cycles = std::stol(line.c_str() + 35, nullptr, 10);
    pcs.emplace_back(pc);
    accs.emplace_back(acc);
    xIndexes.emplace_back(xIndex);
    yIndexes.emplace_back(yIndex);
    statuses.emplace_back(status);
    sps.emplace_back(sp);
    times.emplace_back(cycles);
}

DebugLogFile::DebugLogFile(const std::string &filePath) : inf(filePath){
    if(!inf)
        throw std::runtime_error(filePath + ": " + std::strerror(errno));
}

void DebugLogFile::printLine(int num){
    if(lineNum <= num){
        for(int start = 0; start < 100 || lineNum<=num; ++start){
            std::string line;
            endOfFile = std::getline(inf, line).eof();
            if(endOfFile){
                break;
            }
            parseLine(line);
            ++lineNum;
        }
    }

    printf("%04x A:%02x X:%02x Y:%02x P:%02x SP:%02x CYC:%lld\n",
           pcs[num], accs[num], xIndexes[num], yIndexes[num], statuses[num], sps[num], times[num]);
}
bool DebugLogFile::checkLine(int num, uint16_t pc, uint8_t acc, uint8_t xIndex, uint8_t yIndex, uint8_t status, uint8_t sp, uint64_t cycle) {
    if(lineNum <= num){
        for(int start = 0; start < 100 || lineNum<=num; ++start){
            std::string line;
            endOfFile = std::getline(inf, line).eof();
            if(endOfFile){
                break;
            }
            parseLine(line);
            ++lineNum;
        }
        if(endOfFile){
            return false;
        }
    }
    return pcs[num] == pc &&
        accs[num] == acc &&
        xIndexes[num] == xIndex &&
        yIndexes[num] == yIndex &&
        statuses[num] == status &&
        sps[num] == sp &&
        times[num] == cycle;
}

bool DebugLogFile::checkTiming(int num, uint32_t cycles){
    return times[num] == cycles;
}


