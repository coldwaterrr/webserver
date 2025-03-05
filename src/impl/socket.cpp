#include "socket.h"
#include <sys/socket.h> // 创建socket,bind..
#include <unistd.h> // read,write,sleep...
#include <netinet/in.h> // 创结构体
#include <arpa/inet.h> // inet_ntoa
#include <fcntl.h> // 非阻塞模式
#include <string>
#include <cstring>

Socket::Socket(int port) : port(port){
    logger.info("Creating socket on port:");
    listend_fd = socket(AF_INET, SOCK_STREAM, 0); // create a socket
    if(listend_fd == -1) {
        logger.error("socket creation failed");
        throw "Socket creation failed!";
    }
}

Socket::~Socket() {
    logger.info("Closing server socket");
    close(listend_fd);
    close(server_fd);
}

// 将socket与 IP地址和端口绑定
void Socket::bind() {
    int opt = 1;
    // 设置socket
    setsockopt(listend_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(listend_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        logger.error("Bind failed!");
        throw std::runtime_error("Bind failed!");
    }
    logger.success("Socket successfully bound to port " + std::to_string(port));
}

// 将socket设置为监听状态
void Socket::listen() {
    ::listen(listend_fd, 42); // 42表示最大连接数
    logger.info("Server listening on port " + std::to_string(port));

    // Set the new socket to non-blocking mode
    int flags = fcntl(listend_fd, F_GETFL, 0); // Get the socket flags
    if (flags == -1) {
        logger.error("Failed to get socket flags");
        close(client_fd);
    }
    if (fcntl(listend_fd, F_SETFL, flags | O_NONBLOCK) == -1) { // Set the socket to non-blocking mode
        logger.error("Failed to set socket to non-blocking mode");
        close(listend_fd);
    }
}

int Socket::acceptConnection(std::string &clientIp) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    client_fd = accept(listend_fd, (struct sockaddr *)&address, &addrlen); // Accept connection

    if (client_fd >= 0) {
        clientIp = inet_ntoa(address.sin_addr); // Get client IP address
        logger.success("New connection accepted from " + clientIp);

        // Set the new socket to non-blocking mode
        int flags = fcntl(client_fd, F_GETFL, 0); // Get the socket flags
        if (flags == -1) {
            logger.error("Failed to get socket flags");
            close(client_fd);
            return -1;
        }
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) { // Set the socket to non-blocking mode
            logger.error("Failed to set socket to non-blocking mode");
            close(client_fd);
            return -1;
        }
    }
    else
    {
        clientIp = "-";
        logger.error("Failed to accept connection");
    }
    return client_fd;
}

int Socket::getSocketFd() const {
    return server_fd;
}

int Socket::getListendFd() const {
    return listend_fd;
}