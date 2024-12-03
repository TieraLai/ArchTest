// ecm.cpp
#include "ecm.h"
#include "ecnic.h"
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>
#include <alchemy/task.h>
#include <trank/rtdk.h>

#define PERIOD_S 1000000000 // 1 s = 10000000000 ns

NEC_RtCyclicCallback callbackFunc;
RT_TASK ecmTaskHandle;

void rt_printCurrentTime(int index)
{
    auto now = std::chrono::system_clock::now();

    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count()% 1000000;

    rt_printf("[%02d:%02d:%02d:%06d](%d): ",
              now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, milliseconds,index);

}
void registerCallback(NEC_RtCyclicCallback cb)
{
    callbackFunc = cb;
}

void scanLoop(void *arg)
{
    rt_printCurrentTime(100);
    rt_printf("Task:scanLoop Start \n" );
    RT_TASK_INFO taskInfo;
    rt_task_set_periodic(NULL, TM_NOW, PERIOD_S);
    rt_task_inquire(NULL, &taskInfo);

    while (true)
    {
        rt_printCurrentTime(101);
        rt_printf("Start task ,Task name: %s\n", taskInfo.name);

        auto regs = scanPCI();
        for (int reg : regs)
        {
            rt_printCurrentTime(199);
            int value = readRegister(reg);
            if (callbackFunc)
            {
                callbackFunc(value);
            }
        }
        rt_printCurrentTime(102);
        rt_printf("End task.\n");
        
        // wait for the next period
        rt_task_wait_period(NULL);
    }
}

void startECM()
{
    rt_print_cleanup();
    rt_print_auto_init(1);
    //ceate real time task
    int err = rt_task_create(&ecmTaskHandle, "ScanTask", 0, 50, T_JOINABLE);
    if (err)
    {
        rt_printf("Error creating real-time task: %s\n", strerror(-err));
        return;
    }

    // start real time task
    rt_task_start(&ecmTaskHandle, &scanLoop, NULL);
}

