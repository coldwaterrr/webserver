#include "threadpool.h"
#include "logger.h"


ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    std::string boldNumThreads =  std::to_string(numThreads) + " threads" ; 
    
    logger.info("Initializing thread pool with " + boldNumThreads,"");
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::workerThread, this); // workThread此时不会执行，只会在线程里面开始执行
    }
}

ThreadPool::~ThreadPool() {
    logger.info("Shutting down thread pool","");
    {
        std::unique_lock<std::mutex> lock(queueMutex); // lock the queue
        stop = true;
    }
    condition.notify_all(); // ?
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task; 
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]
                           { return stop || !tasks.empty(); }); // wait until there is a task
            if (stop && tasks.empty())
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}