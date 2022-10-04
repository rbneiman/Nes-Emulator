#ifndef EMULATORTEST_INPUT_H
#define EMULATORTEST_INPUT_H
#include <cstdint>

typedef enum{
    STANDARD,
    ZAPPER
}controllerType_t;

class Controller {
private:
    controllerType_t type1 = STANDARD;
    controllerType_t type2 = STANDARD;
    uint8_t readState1 = 0, readState2 = 0;
    uint8_t lastWrite;
    uint8_t nextBit1, nextBit2;
public:
    Controller();

    void updateState(int controller, uint8_t controllerState);

    void write(uint8_t data);

    uint8_t read(int controllerPort);
};


#endif //EMULATORTEST_INPUT_H
