#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <shared_mutex>
#include <list>
#include <vector>
#include <string>
#include "logger.h"
#include "threadpool.h"
#include "socket.h"
#include "config.h"
#include "lru_k_cache.h"

// 性能相关常量
#define MAX_EVENTS 10000
#define LISTEN_BACKLOG 1024
#define MAX_BATCH_ACCEPT 16
#define MAX_SIZE 8192

/**
 * @brief 帧头部类，用于管理缓存数据
 * 实现了数据的存储和访问控制
 */
class FrameHeader {
    friend class Server;
public:
    explicit FrameHeader(frame_id_t frame_id);

private:
    auto GetData() const -> const char *;
    auto GetDataMut() -> char *;
    void Reset();

    const frame_id_t frame_id_;
    
    std::vector<char> data_;
};

/**
 * @brief 性能监控类
 * 用于记录和统计服务器性能指标
 */
class PerformanceMonitor {
public:
    void recordRequest() { requests_++; }
    void recordError() { errors_++; }
    void recordResponseTime(uint64_t time_ms) { 
        total_response_time_ += time_ms;
        if (time_ms > max_response_time_) max_response_time_ = time_ms;
    }
    
    void printStats() {
        uint64_t total_reqs = requests_.load();
        uint64_t total_errs = errors_.load();
        uint64_t avg_time = total_reqs > 0 ? total_response_time_.load() / total_reqs : 0;
        
        logger.info("Performance Stats:\n"
                   "- Total Requests: " + std::to_string(total_reqs) + "\n"
                   "- Total Errors: " + std::to_string(total_errs) + "\n"
                   "- Average Response Time: " + std::to_string(avg_time) + "ms\n"
                   "- Max Response Time: " + std::to_string(max_response_time_.load()) + "ms");
    }

private:
    std::atomic<uint64_t> requests_{0};
    std::atomic<uint64_t> errors_{0};
    std::atomic<uint64_t> total_response_time_{0};
    std::atomic<uint64_t> max_response_time_{0};
    Logger logger;
};

/**
 * @brief 高性能Web服务器类
 * 实现了基于epoll的事件驱动模型和多线程处理
 */
class Server {
public:
    /**
     * @brief 构造函数
     * @param num_frames 缓存帧数
     * @param port 监听端口
     * @param thread_count 线程池大小
     * @param k_dist LRU-K中的K值
     */
    Server(size_t num_frames, int port, int thread_count, size_t k_dist);
    ~Server();

    // 初始化服务器（socket, epoll 等）
    bool init();
    // 进入主循环，处理 I/O 事件
    void run();

private:
    // 服务器配置
    size_t num_frames_;
    int port;
    Socket socka;
    int listen_fd;
    int epoll_fd;

    // 线程池相关
    std::atomic<client_id_t> next_client_id_;
    ThreadPool thread_pool;
    std::shared_ptr<std::mutex> bpm_latch_;
    std::shared_mutex cache_mutex_;  // 替换原来的mutex_

    // 缓存相关
    std::vector<std::shared_ptr<FrameHeader>> frames_;
    std::vector<frame_id_t> free_frames_;
    std::unordered_map<std::string, frame_id_t> client_table_;  // 修改为使用string作为key
    std::shared_ptr<LRUKCache> cache_;
    
    // 性能监控
    PerformanceMonitor perf_monitor_;
    
    /**
     * @brief 内存池类
     * 用于高效管理内存分配和释放
     */
    class MemoryPool {
    public:
        char* allocate() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (free_buffers_.empty()) {
                buffers_.push_back(std::make_unique<char[]>(MAX_SIZE));
                free_buffers_.push(buffers_.back().get());
            }
            char* buffer = free_buffers_.front();
            free_buffers_.pop();
            return buffer;
        }
        
        void deallocate(char* buffer) {
            std::lock_guard<std::mutex> lock(mutex_);
            free_buffers_.push(buffer);
        }
        
    private:
        std::vector<std::unique_ptr<char[]>> buffers_;
        std::queue<char*> free_buffers_;
        std::mutex mutex_;
    };
    
    MemoryPool memory_pool_;
    
    // 日志
    Logger logger;


    // // 设置监听 socket 为非阻塞模式
    // bool setNonBlocking(int fd);

    // 建立并初始化监听 socket
    bool setupSocket();
    // 初始化 epoll 实例，并将监听 socket 添加到 epoll
    bool setupEpoll();
    // 处理 epoll 返回的事件
    void handleEvents();

    void handleClient(int client_fd);

    auto DeleteClient(client_id_t client_id) -> bool;

    void cacheManage(const std::string& cache_key, std::string buf);
};

