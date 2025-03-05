#include "server.h"       // 包含 Server 类声明
#include "threadpool.h"   // 包含 ThreadPool 类声明
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

// 一个简单的任务，用于测试线程池
void task(int id) {
    std::cout << "Task " << id << " is running on thread " 
              << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 模拟任务执行时间
    std::cout << "Task " << id << " completed." << std::endl;
}

// 测试线程池：提交多个任务到线程池中执行
void testTask() {
    ThreadPool pool(5);  // 创建一个包含 5 个线程的线程池
    std::cout << "Submitting threadpool tasks...\n";
    for (int i = 0; i < 10; i++) {
        pool.enqueue([i] { task(i); });  // 提交任务
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));  // 等待所有任务执行完毕
    std::cout << "All threadpool tasks submitted.\n";
}

int count = 0;

// 模拟客户端连接：简单连接到服务器，发送消息并接收回显
void clientSimulation() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Client: socket creation failed." << std::endl;
        return;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);  // 与服务器端口一致
    inet_pton(AF_INET, "172.17.55.172", &serverAddr.sin_addr);
    
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Client: connect failed." << std::endl;
        close(sock);
        return;
    }
    
    std::string m = "It's the {" + std::to_string(count++) + "} client";
    const char* msg = m.c_str();
    send(sock, msg, strlen(msg), 0);
    
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << "Client received: " << buffer << std::endl;
    } else {
        std::cerr << "Client: recv failed." << std::endl;
    }
    close(sock);
}



// 测试服务端多线程处理：启动服务器，并模拟多个客户端连接
void testServer() {
    // // 创建服务器对象，监听端口 8080
    // Server server(8080, 10);
    // if (!server.init()) {
    //     std::cerr << "Server initialization failed." << std::endl;
    //     return;
    // }
    
    // // 将服务器运行放入一个独立线程（因为 run() 是阻塞调用）
    // std::thread serverThread([&server]() {
    //     server.run();
    // });
    
    // // 等待一段时间，让服务器启动
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // std::cout << "Server is running on port 8080." << std::endl;
    
    // 模拟多个客户端同时连接到服务器
    const int clientCount = 20;
    for (int i = 0; i < clientCount; i++) {
        std::thread(clientSimulation).detach();
    }
    
    // 让服务器继续运行一段时间，处理所有客户端请求
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // // 此处如果 Server 类有停止机制，应调用停止函数；否则直接 detach 服务器线程（仅用于测试）
    // serverThread.detach();
}

int main() {
    // 先测试线程池任务执行
    // testTask();
    
    // 再测试服务端多线程处理（模拟客户端连接）
    testServer();
    
    return 0;
}
