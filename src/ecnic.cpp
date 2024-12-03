#include "ecnic.h"
#include <iostream>
#include <vector>

std::vector<int> scanPCI() {
    return {0x1234, 0x5678, 0x9ABC};
}

int readRegister(int reg) {
    std::cout << "Reading register: " << reg << std::endl;
    return reg + 10; 
}

void writeRegister(int reg, int value) {
    std::cout << "Writing " << value << " to register: " << reg << std::endl;
}