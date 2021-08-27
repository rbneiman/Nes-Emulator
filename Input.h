//
// Created by alec on 8/26/2021.
//

#ifndef EMULATORTEST_INPUT_H
#define EMULATORTEST_INPUT_H
#include <cstdint>


class Controller {
private:
    bool a, b, select, start, up, down, left, right;
    uint8_t readState;
    uint8_t lastWrite;
    uint8_t nextBit;
public:
    Controller();

    void updateState(uint8_t controllerState);

    void write(uint8_t data);

    uint8_t read(int controllerPort);
};


#endif //EMULATORTEST_INPUT_H
