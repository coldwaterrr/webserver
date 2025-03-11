#include "server.h"
#include <sys/socket.h>  // socket、bind、listen、accept 等函数
#include <netinet/in.h>  // sockaddr_in 结构体
#include <netinet/tcp.h> // TCP_NODELAY
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

    // 配置监听套接字的 epoll 事件：
    epoll_event event;
    event.events = EPOLLIN; // 
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
    logger.info("Cache management for client: " + std::to_string(client_id));

    // 如果已经在内存中，只需要更新访问记录
    if(client_table_.find(client_id) != client_table_.end()) {
        frame_id_t frame_id = client_table_[client_id];
        std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
        cache_->RecordAccess(frame_id);
        logger.info("Updated existing cache entry for client: " + std::to_string(client_id));
        return;
    }

    // 不在内存中，需要分配新的frame
    frame_id_t frame_id;
    if(!free_frames_.empty()) {
        logger.info("Allocating from free frames");
        frame_id = free_frames_.back();
        free_frames_.pop_back();
    } else {
        // 内存不够，需要驱逐
        logger.info("No free frames, attempting eviction");
        auto outframe = cache_->Evict();
        if(!outframe.has_value()) {
            logger.error("Eviction failed - no evictable frames");
            return;
        }
        frame_id = outframe.value();
        // 将原来的client_id删掉
        for(auto [k,v] : client_table_) {
            if(v == frame_id) {
                client_table_.erase(k);
                break;
            }
        }
    }

    // 更新缓存
    client_table_[client_id] = frame_id;
    std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
    cache_->RecordAccess(frame_id);
    logger.info("Added new cache entry - client: " + std::to_string(client_id) + ", frame: " + std::to_string(frame_id));
}

// 处理客户端
void Server::handleClient(int client_fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    logger.info("Handling client: " + std::to_string(client_fd));

    // 在内存中,直接send数据然后返回
    if(client_table_.find(client_fd) != client_table_.end()) {
        frame_id_t frame_id = client_table_[client_fd];
        cache_->RecordAccess(frame_id);
        logger.info("Cache hit for client: " + std::to_string(client_fd));
        
        const char* data = frames_[frame_id]->GetData();
        size_t total_len = strlen(data);
        size_t sent = 0;
        
        while (sent < total_len) {
            ssize_t n = send(client_fd, data + sent, total_len - sent, MSG_NOSIGNAL);
            if (n <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 如果发送缓冲区满，等待一下再试
                    usleep(1000); // 等待1ms
                    continue;
                }
                logger.error("Send failed: " + std::string(strerror(errno)));
                break;
            }
            sent += n;
        }
        
        // 不要立即关闭连接，让客户端决定是否保持连接
        return;
    }

    // 缓存未命中，需要处理新请求
    char buffer[MAX_SIZE];
    ssize_t total = 0;
    
    while (true) {
        ssize_t n = read(client_fd, buffer + total, sizeof(buffer) - total - 1);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (total > 0) break;  // 如果已经读取了一些数据，就处理它
                usleep(1000); // 否则等待更多数据
                continue;
            }
            logger.error("Read failed: " + std::string(strerror(errno)));
            close(client_fd);
            return;
        }
        if (n == 0) {
            if (total == 0) {
                // 客户端关闭了连接
                close(client_fd);
                return;
            }
            break;  // 读取完成
        }
        total += n;
        if (total >= sizeof(buffer) - 1) {
            break;  // 缓冲区满
        }
        
        // 检查是否收到了完整的HTTP请求
        if (total >= 4 && 
            strstr(buffer, "\r\n\r\n") != nullptr) {
            break;  // 找到了HTTP请求结束标记
        }
    }

    buffer[total] = '\0';
    logger.info("Received request, size: " + std::to_string(total));

    // 解析HTTP请求
    std::string request(buffer);
    std::string path;
    size_t pathStart = request.find("GET ") + 4;
    if (pathStart != std::string::npos) {
        size_t pathEnd = request.find(" HTTP", pathStart);
        if (pathEnd != std::string::npos) {
            path = request.substr(pathStart, pathEnd - pathStart);
        }
    }

    if (path.empty()) {
        path = "/index.html";  // 默认页面
    }

    // 检查是否是Keep-Alive连接
    bool keepAlive = (request.find("Connection: keep-alive") != std::string::npos);

    // 处理请求并缓存响应
    Router router("/home/zbw/www");
    std::string response = router.route(path, client_fd, "");
    cacheManage(client_fd, response);
    
    if (!keepAlive) {
        close(client_fd);
    }
}

// 处理 epoll 返回的所有事件
void Server::handleEvents() {
    epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if(nfds < 0) {
        if (errno == EINTR) {
            return;  // 被信号中断，直接返回
        }
        logger.error("epoll_wait error: " + std::string(strerror(errno)));
        return;
    }

    for (int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        uint32_t ev = events[i].events;
        
        if(fd == socka.getListendFd()) {
            std::lock_guard<std::mutex> lock(*bpm_latch_);
            
            // 接受新连接
            std::string client_ip;
            int client_fd = socka.acceptConnection(client_ip);
            if(client_fd == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    logger.error("accept failed: " + std::string(strerror(errno)));
                }
                continue;
            }

            // 设置非阻塞模式
            int flags = fcntl(client_fd, F_GETFL, 0);
            if (flags == -1) {
                logger.error("fcntl F_GETFL failed");
                close(client_fd);
                continue;
            }
            if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                logger.error("fcntl F_SETFL failed");
                close(client_fd);
                continue;
            }
            
            // 设置TCP_NODELAY
            int optval = 1;
            if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0) {
                logger.error("setsockopt TCP_NODELAY failed");
                close(client_fd);
                continue;
            }

            // 设置接收超时
            struct timeval tv;
            tv.tv_sec = 5;  // 5秒超时
            tv.tv_usec = 0;
            if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                logger.error("setsockopt SO_RCVTIMEO failed");
                close(client_fd);
                continue;
            }
            
            epoll_event event;
            event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;  // 使用EPOLLONESHOT防止多线程竞争
            event.data.fd = client_fd;
            
            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                logger.error("epoll_ctl add client failed: " + std::string(strerror(errno)));
                close(client_fd);
                continue;
            }
            logger.info("Accepted new connection, fd: " + std::to_string(client_fd));
        } else {
            if ((ev & EPOLLERR) || (ev & EPOLLHUP)) {
                logger.error("epoll error on fd " + std::to_string(fd));
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                continue;
            }

            if (ev & EPOLLIN) {
                // 将fd从epoll中移除，防止重复触发
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                
                thread_pool.enqueue([this, fd]() {
                    handleClient(fd);
                });
            }
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
