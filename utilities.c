#include <stdint.h>

int checkBit(uint8_t a,uint8_t bit){ //true if bit 1
    return (a && (1<<bit)) > 0;
}

