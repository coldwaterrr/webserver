#include "server.h"
#include <sys/socket.h>  // socket、bind、listen、accept 等函数
#include <netinet/in.h>  // sockaddr_in 结构体
#include <fcntl.h>       // fcntl 函数，用于设置非阻塞
#include <unistd.h>      // close 函数
#include <cstring>       // memset 函数
#include <iostream>
#include "threadpool.h"
#include "socket.h"
#include "http.h"
#include "router.h"


// 构造函数：初始化端口，设置监听套接字和 epoll 文件描述符的初始值
Server::Server(int port,int thread_count) : port(port),socka(port), listen_fd(-1), epoll_fd(-1), thread_pool(thread_count) {

 }

// 析构函数：关闭监听套接字和 epoll 文件描述符（如果已创建）
Server::~Server() {
    if(listen_fd != -1) close(listen_fd);
    if(epoll_fd != -1) close(epoll_fd);
}

// 建立并初始化监听 socket
bool Server::setupSocket() {
    // 创建 TCP 套接字
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1) {
        logger.error("socket creation failed");
        return false;
    }

    // 绑定套接字到指定地址和端口
    socka.bind();
    
    // 进入监听状态
    socka.listen();

    logger.info("Socket setup complete on port " + std::to_string(port));
    return true;
}


// 初始化 epoll 实例，并将监听套接字添加到 epoll 中
bool Server::setupEpoll() {

    // 创建 epoll 实例，返回一个 epoll 文件描述符
    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1) {
        logger.error("epoll_create1 failed");
        return false;
    }

    // 配置监听套接字的 epoll 事件：关注可读事件（EPOLLIN）
    epoll_event event;
    event.events = EPOLLIN; // 表示关注数据可读事件
    event.data.fd = socka.getListendFd();
    // 将监听套接字添加到 epoll 监控列表中
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socka.getListendFd(), &event) < 0) {
        logger.error("epoll_ctl failed to add listen_fd");
        return false;
    }

    logger.info("Epoll setup complete", "xxxx");
    return true;
}

// 初始化服务器：建立 socket 和 epoll 实例
bool Server::init() {
    if(!setupSocket())
        return false;
    if(!setupEpoll())
        return false;
    return true;
}

// 处理客户端
void Server::handleClient(int client_fd) {
    char buffer[1024];
    int count = read(client_fd, buffer, sizeof(buffer));
    if (count <= 0) {
        close(client_fd);  // 读取失败或关闭连接
        return;
    }

    logger.info("Received data: " + std::string(buffer, count));

    // 获取该套接字的IP地址
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char ip[INET_ADDRSTRLEN];
    if (getpeername(client_fd, (struct sockaddr *)&addr, &addr_len) == 0) {
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    } else {
        const char* msg = "get IP address failed";
        strncpy(ip, msg, sizeof(msg));
    }

    // 对Http请求进行返回
    // Http::sendResponse(client_fd, Http::getRequestPath(buffer), "GET", 200, ip);

    Router router("/home/zbw/www/");
    router.route("/index.html", client_fd, ip);

    // 回显数据
    write(client_fd, buffer, count);
}

// 处理 epoll 返回的所有事件
void Server::handleEvents() {
    // 定义事件数组，用于存放 epoll_wait 返回的事件
    epoll_event events[MAX_EVENTS];
    // 阻塞等待事件发生，-1 表示无限等待
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if(nfds < 0) {
        logger.error("epoll_wait error");
        return;
    }

    // 遍历所有返回的事件
    for (int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        // 如果事件来自监听套接字，表示有新连接请求
        if(fd == socka.getListendFd()) {

            // 获取该套接字的IP地址
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            char ip[INET_ADDRSTRLEN];
            if (getpeername(fd, (struct sockaddr *)&addr, &addr_len) == 0) {
                inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
            } else {
                const char* msg = "get IP address failed";
                strncpy(ip, msg, sizeof(msg));
            }
            std::string client_ip(ip);

            // 接受新连接
            int client_fd = socka.acceptConnection(client_ip);
            if(client_fd == -1) {
                logger.error("accept failed");
                continue;
            }

            // 配置新连接的 epoll 事件，使用边缘触发模式（EPOLLET）
            epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = client_fd;
            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                logger.error("epoll_ctl add client failed");
                close(client_fd);
                continue;
            }
            logger.info("Accepted new connection",std::string(ip));
        } else {
            thread_pool.enqueue([this, fd]() { handleClient(fd); });
        }
    }
}

// 服务器主循环：不断处理 epoll 事件
void Server::run() {
    logger.info("Server running on port " + std::to_string(port),"xxxx");
    while(true) {
        handleEvents();
    }
}
