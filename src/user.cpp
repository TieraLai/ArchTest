#include "unistd.h"
#include "ecm.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

void userCallback(int value) {
    std::cout << "Callback received value: " << value << std::endl;
}

int main() {
    registerCallback(userCallback);
    std::cout << "User program running..." << std::endl;
    startECM();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 模拟用户程序持续运行
    }
    return 0;
}