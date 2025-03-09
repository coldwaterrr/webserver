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
#include "lru_k_cache.h"
#include "config.h"

#define MAX_SIZE 8192

FrameHeader::FrameHeader(frame_id_t frame_id) : frame_id_(frame_id), data_(MAX_SIZE, 0) { Reset(); }

auto FrameHeader::GetData() const -> const char * {
    return data_.data();
}

auto FrameHeader::GetDataMut() -> char * {
    return data_.data();
}

void FrameHeader::Reset() {
    std::fill(data_.begin(), data_.end(), 0);
}

// 构造函数：初始化端口，设置监听套接字和 epoll 文件描述符的初始值, 设置缓存池大小和k大小
Server::Server(size_t num_frames, int port, int thread_count, size_t k_dist) 
    : num_frames_(num_frames),
      port(port),
      socka(port),
      listen_fd(-1), 
      epoll_fd(-1), 
      next_client_id_(0), 
      thread_pool(thread_count),
      bpm_latch_(std::make_shared<std::mutex>()),
      cache_(std::make_shared<LRUKCache>(num_frames, k_dist)) {
    
    next_client_id_.store(0);

    frames_.reserve(num_frames_);

    client_table_.reserve(num_frames_);
    std::cout << "the num_frames is " << num_frames << std::endl;
    for (size_t i = 0; i < num_frames_; i++) {
        frames_.push_back(std::make_shared<FrameHeader>(i));
        free_frames_.push_back(static_cast<int>(i));
    }
}

// 析构函数：关闭监听套接字和 epoll 文件描述符（如果已创建）
Server::~Server() {
    if(listen_fd != -1) close(listen_fd);
    if(epoll_fd != -1) close(epoll_fd);
}


auto Server::DeleteClient(client_id_t client_id) -> bool {}

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

void Server::cacheManage(client_id_t client_id, std::string buf) {
    // std::lock_guard<std::mutex> lock(mutex_);
    logger.info("what the fuck!!!!!!!!!!");

    // 不在内存中
    // if(client_table_.find(client_id) == client_table_.end()) {
        // logger.info("the response is: ");
        // logger.info(buf.c_str());
        // 有足够的内存，分配空闲内存
        frame_id_t frame_id;
        if(!free_frames_.empty()) {
            logger.info("有空闲空间");
            frame_id = free_frames_.back();
            client_table_[client_id] = frame_id;
            free_frames_.pop_back();
            std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
            logger.info("Added client_fd: " + std::to_string(client_id) + ", frame_id: " + std::to_string(frame_id));  // 添加日志
            // logger.info("the cache Manage is:");
            // logger.info(frames_[frame_id]->GetDataMut());
        }
        else { // 内存不够，驱逐
            if(cache_->Evict().has_value()) {
                frame_id = cache_->Evict().value();
                std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
                client_table_[client_id] = frame_id;  // 插入数据
                logger.info("Evicted and added client_fd: " + std::to_string(client_id) + ", frame_id: " + std::to_string(frame_id));
            } else {
                logger.error("can't evict any frame");   
            }
        }
    // }

}

// 处理客户端
void Server::handleClient(int client_fd) {
    // std::lock_guard<std::mutex> lock(*bpm_latch_);
    // std::lock_guard<std::mutex> lock(mutex_);
    mutex_.lock();
    logger.info("handle client!");
    std::cout << "ok" <<std::endl;
    std::cout << sizeof(client_table_) << std::endl;
    if (client_table_.empty()) {
        std::cout << "client_table_ is empty" << std::endl;
    } else {
        for (auto [k, v] : client_table_) {
            std::cout << v << ":" << k << std::endl;
        }
    }
    // 在内存中,直接send数据然后返回
    if(client_table_.find(client_fd) != client_table_.end()) {
        logger.info("in the cache!!!!!!");
        cache_->RecordAccess(client_table_[client_fd]);
        // logger.info("the content is: ");
        // logger.info(frames_[client_table_[client_fd]]->GetData());
        ssize_t bytes_sent = send(client_fd, frames_[client_table_[client_fd]]->GetData(), strlen(frames_[client_table_[client_fd]]->GetData()), 0);
        if (bytes_sent < 0) {
            logger.error("send failed: " + std::string(strerror(errno)));
        } else {
            logger.success("send the msg, bytes sent: " + std::to_string(bytes_sent));
        }
        // close(client_fd); // 这里是否应该关闭呢？
        mutex_.unlock();
        return;
    }

    char buffer[MAX_SIZE];
    int count = read(client_fd, buffer, sizeof(buffer));

    if (count <= 0) {
        logger.error("read failed and close connect");
        close(client_fd);  // 读取失败或关闭连接
        mutex_.unlock();
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

    // std::lock_guard<std::mutex> lock(*bpm_latch_);
    // std::lock_guard<std::mutex> lock(mutex_);
    Router router("/home/zbw/www");
    std::string response = router.route("/test.html", client_fd, ip);
    cache_->RecordAccess(client_table_[client_fd]);

    // logger.info("cacheManage response:  ");
    // logger.info(response);
    cacheManage(client_fd, response);
    
    // 回显数据
    // write(client_fd, buffer, count);
    close(client_fd);
    mutex_.unlock();
   // bpm_latch_->unlock();
}

// 处理 epoll 返回的所有事件
void Server::handleEvents() {
    // std::lock_guard<std::mutex> lock(*bpm_latch_);
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
        // logger.info("a new fd come in");
        int fd = events[i].data.fd;
        // 如果事件来自监听套接字，表示有新连接请求
        if(fd == socka.getListendFd()) {
            // std::lock_guard<std::mutex> lock(*bpm_latch_);
            bpm_latch_->lock();
            logger.info("a new listend fd come in");
            
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
                bpm_latch_->unlock();
                continue;
            }

            // 配置新连接的 epoll 事件，使用边缘触发模式（EPOLLET）
            epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = client_fd;
            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                logger.error("epoll_ctl add client failed");
                close(client_fd);
                bpm_latch_->unlock();
                continue;
            }
            logger.info("Accepted new connection",std::string(ip));
            bpm_latch_->unlock();
        } else {
            // std::lock_guard<std::mutex> lock(mutex_);
            logger.info("a new client fd come in");
            thread_pool.enqueue([this, fd]() { handleClient(fd); });
            // close(fd);
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
