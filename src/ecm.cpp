// ecm.cpp
#include "ecm.h"
#include "ecnic.h"
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>

NEC_RtCyclicCallback callbackFunc;

// Xenomai task
// RT_TASK ecmTaskHandle;

void registerCallback(NEC_RtCyclicCallback cb){
    callbackFunc = cb;
}

void scanLoop() {
    while (true) {
        std::cout << "Start task : \n"<< std::endl;
        std::cout << "Task name: ScanTask\n"<< std::endl;
        auto regs = scanPCI();        
        for (int reg : regs) {
            int value = readRegister(reg);
            if (callbackFunc) {
                callbackFunc(value);
            }
        }
        std::cout << "End task. \n\n"<< std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟周期性扫描
    }
}

void startECM() 
{
    std::thread(scanLoop).detach(); // 启动周期性扫描线程
}