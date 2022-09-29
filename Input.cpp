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
void Controller::updateState(int controller, uint8_t controllerState) {
    if(controller){
        readState2 = controllerState;
    }else{
        readState1 = controllerState;
    }
}

void Controller::write(uint8_t data) {
    if(data & 0x1){
        nextBit1 = 7;
        nextBit2 = 7;
    }
    lastWrite = data;
}

uint8_t Controller::read(int controllerPort){
    uint8_t* readState;
    controllerType_t controllerType;
    uint8_t* nextBit;
    if(controllerPort == 1){
        readState = &readState1;
        controllerType = type1;
        nextBit = &nextBit1;
    }else{
        readState = &readState2;
        controllerType = type2;
        nextBit = &nextBit2;
    }

    if(controllerType == STANDARD){
        return CHECK_BIT(*readState, (*nextBit)--);
    }else {
        return *readState;
    }
}