#include "ecnic.h"
#include <iostream>
#include <vector>
#include <trank/rtdk.h>

std::vector<int> scanPCI() {
    return {0x1234, 0x5678, 0x9ABC};
}

int readRegister(int reg) {
    rt_printf("Reading register: %d\n", reg);
    return reg + 10; 
}

void writeRegister(int reg, int value) {
    rt_printf("Writing %d to register: %d\n", value, reg);
}

