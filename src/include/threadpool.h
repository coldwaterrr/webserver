#pragma once

#include<vector>
#include<queue>
#include<mutex>
#include<thread>
#include<condition_variable>
#include<functional>
#include"logger.h"

class ThreadPool {
public:
    ThreadPool(size_t numThreads);  // 线程池构造函数，创建 numThreads 个工作线程
    ~ThreadPool();  // 析构函数，销毁线程池，确保所有线程正确退出
    void enqueue(std::function<void()> task);  // 添加任务到任务队列

private:
    std::vector<std::thread> workers;        // 线程池中的工作线程
    std::queue<std::function<void()>> tasks; // 任务队列
    std::mutex queueMutex;                   // 保护任务队列的互斥锁
    std::condition_variable condition;       // 线程同步条件变量
    bool stop;                               // 线程池是否停止的标志
    Logger logger;                         

    void workerThread();  // 线程函数，每个工作线程循环从任务队列取任务执行
};
