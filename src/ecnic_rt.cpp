#include "ecnic.h"
#include <iostream>
#include <vector>
#include <trank/rtdk.h>

std::vector<int> scanPCI() {
    // 模拟扫描并返回一些网卡信息
    return {0x1234, 0x5678, 0x9ABC};
}

int readRegister(int reg) {
    // 模拟从网卡读取寄存器
    rt_printf("Reading register: %d\n", reg);
    return reg + 10; // 假定返回值
}

void writeRegister(int reg, int value) {
    // 模拟写入寄存器
    rt_printf("Writing %d to register: %d\n", value, reg);
}