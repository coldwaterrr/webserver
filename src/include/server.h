#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>
#include "logger.h"
#include <string>
#include "threadpool.h"
#include "socket.h"

#define MAX_EVENTS 1024

class Server {
public:
    Server(int port, int thread_count);
    ~Server();

    // 初始化服务器（socket, epoll 等）
    bool init();
    // 进入主循环，处理 I/O 事件
    void run();

private:
    int listen_fd;  // 监听 socket
    int epoll_fd;   // epoll 文件描述符
    int port;       // 监听端口
    Socket socka; // 套接字

    Logger logger;  // 日志对象

    ThreadPool thread_pool;

    // 设置监听 socket 为非阻塞模式
    bool setNonBlocking(int fd);
    // 建立并初始化监听 socket
    bool setupSocket();
    // 初始化 epoll 实例，并将监听 socket 添加到 epoll
    bool setupEpoll();
    // 处理 epoll 返回的事件
    void handleEvents();

    void handleClient(int client_fd);
};

