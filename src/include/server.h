#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>
#include "logger.h"
#include <string>
#include "threadpool.h"
#include "socket.h"
#include <list>
#include <vector>
#include "config.h"
#include "lru_k_cache.h"

#define MAX_EVENTS 1024

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

class Server {
public:
    Server(size_t num_frames, int port, int thread_count, size_t k_dist);
    ~Server();

    // 初始化服务器（socket, epoll 等）
    bool init();
    // 进入主循环，处理 I/O 事件
    void run();

private:
    int listen_fd;  // 监听 socket
    int epoll_fd;   // epoll 文件描述符
    int port;       // 监听端口

    const size_t num_frames_;

    std::atomic<client_id_t> next_client_id_;

    std::shared_ptr<std::mutex> bpm_latch_;

    std::mutex mutex_;

    std::vector<std::shared_ptr<FrameHeader>> frames_;

    std::unordered_map<client_id_t, frame_id_t> client_table_;

    std::list<frame_id_t> free_frames_;

    std::shared_ptr<LRUKCache> cache_;

    Socket socka; // 套接字

    Logger logger;  // 日志对象

    ThreadPool thread_pool;


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

    void cacheManage(client_id_t client_id, std::string buf);
};

