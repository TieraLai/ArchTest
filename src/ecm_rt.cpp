// ecm.cpp
#include "ecm.h"
#include "ecnic.h"
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>
#include <alchemy/task.h>

NEC_RtCyclicCallback callbackFunc;

// Xenomai task
RT_TASK ecmTaskHandle;

void registerCallback(NEC_RtCyclicCallback cb){
    callbackFunc = cb;
}

void scanLoop() {
    RT_TASK_INFO taskInfo;
    while (true) {
        std::cout << "Start task : \n"<< std::endl;
        rt_tasl_inquire(NULL, &taskInfo);
        std::cout << "Task name: " << taskInfo.name  << "\n"<< std::endl;
        auto regs = scanPCI();
        for (int reg : regs) {
            int value = readRegister(reg);
            if (callbackFunc) {
                callbackFunc(value);
            }
        }
        std::cout << "End task. \n\n"<< std::endl;
        rt_task_sleep(1000000000);  // 1 秒
    }
}

void startECM() 
{
    // 创建 Xenomai 实时任务
    int err = rt_task_create(&ecmTaskHandle, "ScanTask", 0, 99, T_JOINABLE);
    if (err) {
        std::cerr << "Error creating real-time task: " << strerror(-err) << std::endl;
        return;
    }

    // 启动任务
    rt_task_start(&ecmTaskHandle, &scanLoop, NULL);
}