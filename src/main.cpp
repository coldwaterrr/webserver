#include "server.h"
#define PORT 8080
#include <iostream>
#include "threadpool.h"


int main() {
    // 创建一个服务器实例，监听 8080 端口
    Server server(PORT, 5);
    if (!server.init()) {
        return -1;
    }
    server.run();
    return 0;
}
