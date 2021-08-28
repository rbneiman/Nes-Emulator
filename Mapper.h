//
// Created by alec on 8/27/2021.
//

#ifndef EMULATORTEST_MAPPER_H
#define EMULATORTEST_MAPPER_H
#include <cstdint>
#include <vector>

typedef enum{
    HORIZONTAL = 0,
    VERTICAL = 1
}mirror_type_t;

class Mapper{
protected:
    const std::vector<char>& contents;

    int prgSize;
    int chrSize;
    mirror_type_t mirror_type;
    bool has_persistent;
    bool has_trainer;
    bool ignore_mirror;
    uint8_t prgRamSize;
public:
    explicit Mapper(const std::vector<char>& contents);

    virtual uint8_t read8(uint16_t address);
    virtual uint16_t read16(uint16_t address) = 0;

    virtual void write8(uint16_t address, uint8_t arg) = 0;
};

class Mapper0 : public Mapper{
private:
    uint8_t vram[0x2000]{};
    std::vector<char> prg;
    std::vector<char> chr;

public:
    explicit Mapper0(const std::vector<char>& contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};

class Mapper1 : public Mapper{
private:
    uint8_t vram[0x2000]{};
    std::vector<char> prg;
    std::vector<char> prgRAM;
    std::vector<char> chr;

    //im not sure of the offsets here
    int chrBank0Off{0x0};
    int chrBank1Off{0x1000};

    int prgBank0Off{0x0};
    int prgBank1Off{0x4000};

    int prgRAMBankOff{0x0};

    uint8_t loadProgress{0};
    uint8_t loadReg{0};
    bool loadDone{false};

    uint8_t control{0};
    uint8_t mirroring{0};
    uint8_t prgBankMode{0};
    uint8_t chrBankMode{0};
    uint8_t chrBank0{0};
    uint8_t chrBank1{0};
    uint8_t prgBank{0};

    uint16_t mirror(uint16_t address) const;
    void setControl(uint8_t arg);

    uint8_t writeLoadReg(uint8_t arg);
    void writeControl(uint8_t arg);
    void writeChrBank0(uint8_t arg);
    void writeChrBank1(uint8_t arg);
    void writePrgBank(uint8_t arg);

public:
    explicit Mapper1(const std::vector<char>& contents);
    uint16_t read16(uint16_t address) override;
    void write8(uint16_t address, uint8_t arg) override;
};

#endif //EMULATORTEST_MAPPER_H
