#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>


Logger::Logger() {        
    LogFile.open("zbw.log",std::ios::app); // append方法写入日志
    info("Logger initialized",""); 
}

Logger::~Logger() {
    if(LogFile.is_open()) {
        info("Logger close","");
        LogFile.close();
    }
 }

void Logger::info(const std::string &msg, const std::string &ip) {
    // 创建一个锁对象，自动上锁和解锁，保证在多线程环境下日志输出不会混乱
    std::lock_guard<std::mutex> lock(mtx);
    
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    
    // 将当前时间转换为 time_t 类型，便于格式化输出
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // 使用 std::ctime 将 time_t 类型的时间转换为可读的字符串格式（注意：ctime 返回的字符串末尾带换行符）
    // 输出信息日志到标准输出（std::cout）
    std::cout << "[INFO] " << std::ctime(&now_time) << ": " << ip << ":" << msg << std::endl;
    LogFile << "[INFO] " << std::ctime(&now_time) << ": " << ip << ":" << msg << std::endl;
    LogFile.flush(); // 确保数据立即刷新回磁盘
    
}

void Logger::error(const std::string &msg) {
    // 同样加锁，确保多线程下的安全输出
    std::lock_guard<std::mutex> lock(mtx);
    
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    
    // 转换时间为 time_t 类型
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // 输出错误日志到标准错误流（std::cerr），便于区分普通日志和错误日志
    std::cerr << "[ERROR] " << std::ctime(&now_time) << ": " << msg << std::endl;
    LogFile << "[ERROR] " << std::ctime(&now_time) << ": " << msg << std::endl;
    LogFile.flush(); // 确保数据立即刷新回磁盘
}


void Logger::success(const std::string &msg) {

    std::lock_guard<std::mutex> lock(mtx);
    
    auto now = std::chrono::system_clock::now();
    
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    std::cerr << "[SUCCESS] " << std::ctime(&now_time) << ": " << msg << std::endl;
    LogFile << "[SUCCESS] " << std::ctime(&now_time) << ": " << msg << std::endl;
    LogFile.flush(); // 确保数据立即刷新回磁盘
}