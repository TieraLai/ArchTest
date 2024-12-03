#include "unistd.h"
#include "ecm.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

void printCurrentTime(int index)
{
    auto now = std::chrono::system_clock::now();

    // 转换为时间格式
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t); // 本地时间（带时区）

    // 获取毫秒部分
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()% 1000000;

    // 打印本地时间，格式为 HH:MM:SS:mm
    std::cout << "["
              << std::put_time(&now_tm, "%H:%M:%S") << ":"
              << std::setw(6) << milliseconds
              << "](" << index << "): "              ;
}
void userCallback(int value)
{
    printCurrentTime(99);
    std::cout << "Callback received value: " << value << std::endl;
}

int main()
{
    printCurrentTime(0);
    std::cout << "User registerCallback" << std::endl;
    registerCallback(userCallback);
    printCurrentTime(1);
    std::cout << "User program running..." << std::endl;
    startECM();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 模拟用户程序持续运行
    }
    return 0;
}