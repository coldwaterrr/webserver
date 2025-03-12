#include "server.h"
#include <sys/socket.h>  // socket、bind、listen、accept 等函数
#include <netinet/in.h>  // sockaddr_in 结构体
#include <netinet/tcp.h> // TCP_NODELAY
#include <sys/uio.h>
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

void Server::cacheManage(const std::string& cache_key, std::string buf) {
    logger.info("Cache management for path: " + cache_key);

    // 如果已经在内存中，只需要更新访问记录
    if(client_table_.find(cache_key) != client_table_.end()) {
        frame_id_t frame_id = client_table_[cache_key];
        std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
        cache_->RecordAccess(frame_id);
        logger.info("Updated existing cache entry for path: " + cache_key);
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
        
        // 找到并删除被驱逐的缓存项
        for(auto it = client_table_.begin(); it != client_table_.end(); ) {
            if(it->second == frame_id) {
                logger.info("Evicting cache entry for path: " + it->first);
                it = client_table_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 更新缓存
    client_table_[cache_key] = frame_id;
    std::strncpy(frames_[frame_id]->GetDataMut(), buf.c_str(), MAX_SIZE);
    cache_->RecordAccess(frame_id);
    logger.info("Added new cache entry - path: " + cache_key + ", frame: " + std::to_string(frame_id));
}

// 处理客户端
void Server::handleClient(int client_fd) {
    std::unique_ptr<char[]> buffer(new char[MAX_SIZE]);
    size_t total_read = 0;
    ssize_t bytes_read;

    // 使用状态机解析HTTP请求
    HttpRequestParser parser;
    
    while (true) {
        bytes_read = read(client_fd, buffer.get() + total_read, MAX_SIZE - total_read);
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // 没有更多数据可读
            }
            logger.error("Read error: " + std::string(strerror(errno)));
            close(client_fd);
            return;
        }
        if (bytes_read == 0) {
            close(client_fd);  // 连接已关闭
            return;
        }

        total_read += bytes_read;
        auto result = parser.parse(buffer.get(), total_read);
        
        if (result.state == HttpRequestParser::State::ERROR) {
            // 发送400错误响应
            std::string error_response = Http::buildResponse("Bad Request", "text/plain", 400);
            send(client_fd, error_response.c_str(), error_response.length(), MSG_NOSIGNAL);
            close(client_fd);
            return;
        }
        
        if (result.isComplete()) {
            // 处理缓存
            std::string cache_key = result.path;
            bool cache_hit = false;
            
            {
                std::shared_lock<std::shared_mutex> lock(cache_mutex_);
                if (client_table_.find(cache_key) != client_table_.end()) {
                    frame_id_t frame_id = client_table_[cache_key];
                    cache_->RecordAccess(frame_id);
                    
                    // 使用writev进行聚集写
                    struct iovec iov[1];
                    iov[0].iov_base = (void*)frames_[frame_id]->GetData();
                    iov[0].iov_len = strlen(frames_[frame_id]->GetData());
                    
                    ssize_t total_sent = 0;
                    while (total_sent < iov[0].iov_len) {
                        ssize_t sent = writev(client_fd, iov, 1);
                        if (sent < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                continue;
                            }
                            logger.error("Send error: " + std::string(strerror(errno)));
                            break;
                        }
                        total_sent += sent;
                        iov[0].iov_base = (char*)iov[0].iov_base + sent;
                        iov[0].iov_len -= sent;
                    }
                    cache_hit = true;
                }
            }
            
            if (!cache_hit) {
                // 生成新响应
                Router router("/home/zbw/www");
                std::string response = router.route(result.path, client_fd, "");
                
                // 更新缓存
                cacheManage(cache_key, response);
                
                // 发送响应
                ssize_t total_sent = 0;
                while (total_sent < response.length()) {
                    ssize_t sent = send(client_fd, response.c_str() + total_sent, 
                                      response.length() - total_sent, MSG_NOSIGNAL);
                    if (sent < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        logger.error("Send error: " + std::string(strerror(errno)));
                        break;
                    }
                    total_sent += sent;
                }
            }
            
            // 处理keep-alive
            if (Http::isKeepAlive(std::string(buffer.get()))) {
                epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event) < 0) {
                    logger.error("Failed to modify client in epoll");
                    close(client_fd);
                }
            } else {
                close(client_fd);
            }
            return;
        }
        
        if (total_read >= MAX_SIZE) {
            // 请求太大，发送413错误
            std::string error_response = Http::buildResponse("Request Entity Too Large", "text/plain", 413);
            send(client_fd, error_response.c_str(), error_response.length(), MSG_NOSIGNAL);
            close(client_fd);
            return;
        }
    }
}

// 处理 epoll 返回的所有事件
void Server::handleEvents() {
    epoll_event events[MAX_EVENTS];
    
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if(nfds < 0) {
            if (errno == EINTR) {
                continue;  // 被信号中断，继续等待
            }
            logger.error("epoll_wait error: " + std::string(strerror(errno)));
            break;
        }

        // 批量处理连接请求
        std::vector<std::function<void()>> batch_tasks;
        batch_tasks.reserve(nfds);  // 预分配空间

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;

            if (fd == socka.getListendFd()) {
                // 批量接受新连接
                for (int j = 0; j < 16; ++j) {  // 每次最多接受16个新连接
                    std::string client_ip;
                    int client_fd = socka.acceptConnection(client_ip);
                    if (client_fd < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            logger.error("Accept failed: " + std::string(strerror(errno)));
                        }
                        break;
                    }
                    
                    epoll_event client_event;
                    client_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    client_event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) < 0) {
                        logger.error("Failed to add client to epoll");
                        close(client_fd);
                        continue;
                    }
                }
            } else {
                if ((ev & EPOLLERR) || (ev & EPOLLHUP)) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    continue;
                }

                if (ev & EPOLLIN) {
                    batch_tasks.push_back([this, fd]() {
                        handleClient(fd);
                    });
                }
            }
        }

        // 批量提交任务到线程池
        for (auto& task : batch_tasks) {
            thread_pool.enqueue(std::move(task));
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
