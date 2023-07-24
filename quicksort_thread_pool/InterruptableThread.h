#pragma once
#include<thread>
#include<chrono> 
#include <mutex>
#include "ThreadUtils.h" 

class ThreadPool;
class InterruptableThread {
    std::mutex _defender;
    bool* _pFlag;
    std::thread _thread;
    void startFunc(ThreadPool*, size_t);
public:
    InterruptableThread(ThreadPool*, size_t);
    ~InterruptableThread();
    static bool checkInterrupted();
    void interrupt();
    void join();
};
