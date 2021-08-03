#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>

int checkBit(uint8_t a,uint8_t bit){ //true if bit 1
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
