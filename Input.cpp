//
// Created by alec on 8/26/2021.
//

#include "Input.h"
#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)

Controller::Controller() {

}

/*
1 - right
2 - left
3 - down
4 - up
5 - start
6 - select
7 - b
8 - a
 */
void Controller::updateState(uint8_t controllerState) {
    readState = controllerState;
}

void Controller::write(uint8_t data) {
    if(data & 0x1)
        nextBit = 7;
    lastWrite = data;
}

uint8_t Controller::read(int controllerPort){
    if(controllerPort == 2)
        return 0;
    return CHECK_BIT(readState, nextBit--);
}