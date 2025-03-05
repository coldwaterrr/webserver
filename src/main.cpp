#include "server.h"
// #define PORT 8080
#include <iostream>
#include <string>
#include "threadpool.h"


int main(int argc, char *argv[]) {
    std::string port = argv[1];
    // 创建一个服务器实例，监听 8080 端口
    Server server(std::stoi(port), 100);
    if (!server.init()) {
        return -1;
    }
    server.run();
    return 0;
}
