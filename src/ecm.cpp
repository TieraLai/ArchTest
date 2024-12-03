// ecm.cpp
#include "ecm.h"
#include "ecnic.h"
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>
#include <iomanip>
NEC_RtCyclicCallback callbackFunc;

static void printStr(int index, const char *str)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000000;

    std::cout << "["
              << std::put_time(&now_tm, "%H:%M:%S") << ":"
              << std::setw(6) << milliseconds
              << "](" << index << "): "
              << str << std::endl;
}

static void printCurrentTime(int index)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000000;

    std::cout << "["
              << std::put_time(&now_tm, "%H:%M:%S") << ":"
              << std::setw(6) << milliseconds
              << "](" << index << "): ";
}

void registerCallback(NEC_RtCyclicCallback cb)
{
    callbackFunc = cb;
}

void scanLoop()
{

    printStr(100, "Task:scanLoop Start");
    while (true)
    {
        printStr(101, "Start task , Task name: ScanTask");
        auto regs = scanPCI();
        for (int reg : regs)
        {
            printCurrentTime(199);
            int value = readRegister(reg);
            if (callbackFunc)
            {
                callbackFunc(value);
            }
        }
        printStr(102, "End task.");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void startECM()
{
    std::thread(scanLoop).detach();
}

