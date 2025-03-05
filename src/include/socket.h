#ifndef PGS_SOCKET_HPP
#define PGS_SOCKET_HPP

// #include "common.h"
#include "logger.h"

class Socket
{
public:
    explicit Socket(int port);
    ~Socket();

    void bind();
    void listen();
    void closeSocket();
    [[nodiscard]] int acceptConnection(std::string &clientIp);
    [[nodiscard]] int getSocketFd() const;
    int getListendFd() const;

    static std::string durationToString(const std::chrono::steady_clock::duration &duration);

private:
    int server_fd; // Server socket file descriptor
    int listend_fd;
    int client_fd;
    int port;      // Port number
    Logger logger;
};

#endif // PGS_SOCKET_HPP