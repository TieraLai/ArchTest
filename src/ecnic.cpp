#include "ecnic.h"
#include <iostream>
#include <vector>

std::vector<int> scanPCI() {
    // 模拟扫描并返回一些网卡信息
    return {0x1234, 0x5678, 0x9ABC};
}

int readRegister(int reg) {
    // 模拟从网卡读取寄存器
    std::cout << "Reading register: " << reg << std::endl;
    return reg + 10; // 假定返回值
}

void writeRegister(int reg, int value) {
    // 模拟写入寄存器
    std::cout << "Writing " << value << " to register: " << reg << std::endl;
}