#include "socket.h"
#include <sys/socket.h> // 创建socket,bind..
#include <unistd.h> // read,write,sleep...
#include <netinet/in.h> // 创结构体
#include <arpa/inet.h> // inet_ntoa
#include <fcntl.h> // 非阻塞模式
#include <string>
#include <cstring>
#include <netinet/tcp.h>

/**
 * @brief 创建并初始化Socket
 * @param port 监听端口
 * 直接创建非阻塞socket，提高性能
 */
Socket::Socket(int port) : port(port) {
    logger.info("Creating socket on port:");
    // 创建非阻塞TCP socket
    listend_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(listend_fd == -1) {
        logger.error("socket creation failed");
        throw "Socket creation failed!";
    }
}

Socket::~Socket() {
    logger.info("Closing server socket");
    close(listend_fd);
    close(client_fd);
}

/**
 * @brief 绑定socket到指定端口
 * 设置多个TCP优化选项提升性能
 */
void Socket::bind() {
    // TCP优化设置
    int opt = 1;
    // 允许地址重用，快速重启服务器
    setsockopt(listend_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    // 设置发送和接收缓冲区，提高吞吐量
    int buffer_size = 64 * 1024; // 64KB
    setsockopt(listend_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    setsockopt(listend_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    
    // 禁用Nagle算法，减少延迟
    setsockopt(listend_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    
    // 启用TCP快速打开，减少连接建立时间
    int qlen = 5;
    setsockopt(listend_fd, IPPROTO_TCP, TCP_FASTOPEN, &qlen, sizeof(qlen));

    // 绑定地址和端口
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(listend_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger.error("Bind failed!");
        throw std::runtime_error("Bind failed!");
    }
    logger.success("Socket successfully bound to port " + std::to_string(port));
}

/**
 * @brief 开始监听连接
 * 使用最大连接队列长度，提高并发处理能力
 */
void Socket::listen() {
    ::listen(listend_fd, SOMAXCONN);
    logger.info("Server listening on port " + std::to_string(port));

    // 确保监听socket为非阻塞模式
    int flags = fcntl(listend_fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(listend_fd, F_SETFL, flags | O_NONBLOCK);
    }
}

/**
 * @brief 接受新的客户端连接
 * @param clientIp 存储客户端IP地址
 * @return 新连接的socket描述符
 * 
 * 使用accept4直接创建非阻塞socket，
 * 设置TCP优化选项提升性能
 */
int Socket::acceptConnection(std::string &clientIp) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    
    // 使用accept4直接创建非阻塞socket
    client_fd = accept4(listend_fd, (struct sockaddr *)&address, &addrlen, SOCK_NONBLOCK);

    if (client_fd >= 0) {
        clientIp = inet_ntoa(address.sin_addr);
        logger.success("New connection accepted from " + clientIp);

        // TCP连接优化
        int opt = 1;
        // 禁用Nagle算法，减少延迟
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
        
        // 配置TCP Keep-Alive
        int keepalive = 1;
        int keepidle = 60;    // 空闲60秒后开始探测
        int keepinterval = 10; // 探测间隔10秒
        int keepcount = 3;     // 探测3次无响应则断开连接
        setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
        setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
        setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval));
        setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            logger.error("Failed to accept connection: " + std::string(strerror(errno)));
        }
    }
    return client_fd;
}

int Socket::getSocketFd() const {
    return client_fd;
}

int Socket::getListendFd() const {
    return listend_fd;
}