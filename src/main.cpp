#include "server.h"
// #define PORT 8080
#include <iostream>
#include <string>
#include "threadpool.h"


int main(int argc, char *argv[]) {
    // std::string port = argv[1];
    std::string port = "8080";
    // 创建一个服务器实例，监听 8080 端口
    Server server(20, std::stoi(port), 10, 5); // size_t num_frames, int port, int thread_count, size_t k_dist
    if (!server.init()) {
        return -1;
    }
    server.run();
    return 0;
}
