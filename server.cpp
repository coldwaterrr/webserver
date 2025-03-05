#include <iostream>
#include <cstring>      // 用于 memset 等函数
#include <sys/socket.h> // socket, bind, listen, accept, send 等函数
#include <netinet/in.h> // sockaddr_in 结构体
#include <arpa/inet.h>  // inet_addr 等函数
#include <unistd.h>     // close 函数

#define stringlength 100


int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        perror("socket create error");
    }

    sockaddr_in addr; 
    // 置0
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET; // ①协议族，固定填AF_INET。
    // 设置端口号，例如8080,注意要转换为网络字节序
    addr.sin_port = htons(8080); // ②指定服务端的通信端口
    // 设置IP地址，转换为网络字节序
    // addr.sin_addr.s_addr = inet_addr("172.17.55.172");
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // ③如果操作系统有多个IP，全部的IP都可以用于通讯。
    
    
    // 将套接字绑定到一个具体的ip地址和端口
    if(bind(sock,(struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(sock);
        perror("bind error");
        return 0;
    }

    // 将套接字置于监听状态
    if(listen(sock, 5) == -1) {
        close(sock);
        perror("listen error");
        sock = -1;
        // return 0;
    }

    socklen_t addr_c = sizeof(addr);
    int client_sock = accept(sock, (struct sockaddr*)&addr, &addr_c);
    // 从监听队列中接受一个传入的连接请求，返回一个新的套接字描述符，用于与客户端进行通信。
    if(client_sock < 0) {
        perror("accept error");
        return 0;
    }

    char data[stringlength] = "helloworld!";

    if(sock == -1) {
        return 0;
    }
    // 在已建立连接的套接字上发送数据
    if(send(client_sock, &data, sizeof(data), 0) < 0) return 0;

    char buf[stringlength] = "";
    // 从已建立的套接字接收数据
    if(recv(client_sock, &buf, sizeof(buf), 0) <= 0) return 0;

    std::cout << buf << std::endl;
    
    close(client_sock);

    close(sock ); // 关闭套接字
   return 0;
}