//
// Created by alec on 8/27/2021.
//

#ifndef EMULATORTEST_MAPPER_H
#define EMULATORTEST_MAPPER_H
#include <cstdint>
#include <vector>
#include <memory>

typedef enum{
    HORIZONTAL = 0,
    VERTICAL = 1,
    ONE_SCREEN_LOWER = 2,
    ONE_SCREEN_UPPER = 3
}mirrorType_t;

class Mapper{
protected:
    const std::vector<char>& contents;
    uint8_t vram[0x2000]{};
    int prgSize;
    int chrSize;
    mirrorType_t mirrorType;
    bool hasChrRam;
    bool has_persistent;
    bool has_trainer;
    bool ignore_mirror;
    uint8_t prgRamSize;
public:
    void printMemoryDebug(int start, int end);
    explicit Mapper(const std::vector<char>& contents);

    virtual uint8_t read8(uint16_t address);
    virtual uint16_t read16(uint16_t address) = 0;

    virtual void write8(uint16_t address, uint8_t arg) = 0;
};

class Mapper0 : public Mapper{
protected:
    uint32_t prgStart;
    uint32_t chrStart;
public:
    explicit Mapper0(const std::vector<char>& contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};

class Mapper1 : public Mapper{
protected:
    std::vector<uint8_t> chrRam{};
    std::vector<uint8_t> prgRam{};
    int prgRomStart;
    int chrRomStart;
    int prgBank0{0};
    int prgBank1{0};
    int chrBank0{0};
    int chrBank1{0};
    int shiftProgress{0};
    uint8_t shiftRegister{0};
    uint8_t prgRomBankMode{0};
    uint8_t chrRomBankMode{0};  //0: switch 8 KB at a time; 1: switch two separate 4 KB banks
public:
    explicit Mapper1(const std::vector<char>& contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};

//Mapper2 is only used by punch-out basically

class Mapper3 : public Mapper0{
protected:
    uint32_t chrBankOff = 0;
public:
    explicit Mapper3(const std::vector<char>& contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};


class Mapper66 : public Mapper0{
protected:
    uint32_t chrBankOff = 0;
    uint32_t prgBankOff = 0;

public:
    explicit Mapper66(const std::vector<char> &contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};


#endif //EMULATORTEST_MAPPER_H
