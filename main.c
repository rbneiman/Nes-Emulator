#include <stdio.h>
#include <stdint.h>

uint8_t stuff[] = {0x0A,0x33,4,2};
uint16_t test(uint16_t address){
    return *((uint16_t*) &stuff + address);
}

int main() {
    printf("%X\n", test(0));
    return 0;
}
