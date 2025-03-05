#pragma once

#include <string>
#include <mutex>
#include <fstream>

class Logger {
public:
    Logger();
    ~Logger();

    // 记录信息日志
    void info(const std::string &msg, const std::string &ip = "-");
    // 记录错误日志
    void error(const std::string &msg);
    //
    void success(const std::string &msg);

private:
    std::mutex mtx; // 用于保护输出（多线程下保证日志不会交叉）
    std::ofstream LogFile;
};

