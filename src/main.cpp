#include "server.h"
// #define PORT 8080
#include <iostream>
#include <string>
#include "threadpool.h"
#define MAX_SIZE 8192

int main(int argc, char *argv[]) {
    // 获取CPU核心数
    int cpu_cores = std::thread::hardware_concurrency();
        
    // 配置参数
    int thread_count = cpu_cores * 2;  // 线程数 = CPU核心数 * 2
    size_t memory_mb = 8;  // 期望使用的内存大小（MB）
    size_t num_frames = (memory_mb * 1024 * 1024) / MAX_SIZE;  // 根据期望内存大小计算帧数
    size_t k_dist = 2;  // LRU-K中的K值
    int port = 8080;  // 服务端口
    // 创建一个服务器实例，监听 8080 端口
    Server server(num_frames, port, thread_count, k_dist); // size_t num_frames, int port, int thread_count, size_t k_dist
    
    // 输出配置信息
    std::cout << "Server Configuration:" << std::endl
              << "- Thread Count: " << thread_count << std::endl
              << "- Cache Frames: " << num_frames << std::endl
              << "- Cache Size: " << (num_frames * MAX_SIZE / 1024 / 1024) << "MB" << std::endl
              << "- LRU-K Value: " << k_dist << std::endl
              << "- Port: " << port << std::endl;
    
    if (!server.init()) {
        return -1;
    }
    server.run();
    return 0;
}
