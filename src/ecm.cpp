// ecm.cpp
#include "ecm.h"
#include "ecnic.h"
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>

NEC_RtCyclicCallback callbackFunc;

void registerCallback(NEC_RtCyclicCallback cb){
    callbackFunc = cb;
}

void scanLoop() {
    while (true) {
        auto regs = scanPCI();
        for (int reg : regs) {
            int value = readRegister(reg);
            if (callbackFunc) {
                callbackFunc(value);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟周期性扫描
    }
}

void startECM() 
{
    std::thread(scanLoop).detach(); // 启动周期性扫描线程
}