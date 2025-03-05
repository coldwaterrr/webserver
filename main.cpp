#include "src/include/threadpool.h"
#include <iostream>
#include <chrono>

void testTask(int id) {
    std::cout << "Task " << id << " is running on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 模拟任务执行时间
    std::cout << "Task " << id << " completed." << std::endl;
}

int main() {
    ThreadPool pool(5);  // 创建包含 4 个线程的线程池
    
    std::cout << "Submitting tasks...\n";
    for (int i = 0; i < 10; i++) {
        pool.enqueue([i] { testTask(i); });  // 提交任务到线程池
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));  // 等待任务执行完毕
    std::cout << "All tasks submitted.\n";
    return 0;
}
