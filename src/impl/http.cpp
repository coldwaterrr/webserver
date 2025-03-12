#include <string>
#include <fstream>
#include <sys/socket.h>
#include <sstream>
#include <cstring>
#include "logger.h"
#include "http.h"

#define MAX_SIZE 8192

// 预分配的响应头缓冲区
char Http::header_buffer[MAX_HEADER_SIZE];

/**
 * @brief HTTP请求解析器的主要处理函数
 * @param data 输入数据
 * @param len 数据长度
 * @return 解析结果
 * 
 * 使用状态机模式逐字符解析HTTP请求，
 * 支持流式处理，不需要等待完整数据
 */
HttpRequestParser::ParseResult HttpRequestParser::parse(const char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        char ch = data[i];
        
        switch (result_.state) {
            case State::REQUEST_LINE:
                parseRequestLine(ch);  // 解析请求行
                break;
            case State::HEADERS:
                parseHeaders(ch);      // 解析头部
                break;
            case State::BODY:
                parseBody(ch);         // 解析消息体
                break;
            case State::FINISHED:
            case State::ERROR:
                return result_;        // 解析完成或出错
        }
    }
    return result_;
}

/**
 * @brief 解析HTTP请求行
 * @param ch 输入字符
 * 
 * 格式：METHOD PATH HTTP/VERSION
 * 例如：GET /index.html HTTP/1.1
 */
void HttpRequestParser::parseRequestLine(char ch) {
    if (ch == '\r') {
        return;  // 忽略回车符
    }
    if (ch == '\n') {
        // 请求行结束，进行解析
        std::vector<std::string> parts = split(lineBuffer_, ' ');
        if (parts.size() != 3) {
            result_.state = State::ERROR;  // 格式错误
            return;
        }
        result_.method = parts[0];   // 请求方法
        result_.path = parts[1];     // 请求路径
        result_.version = parts[2];   // HTTP版本
        
        result_.state = State::HEADERS;  // 转入头部解析状态
        lineBuffer_.clear();
        return;
    }
    lineBuffer_ += ch;  // 累积字符
}

/**
 * @brief 解析HTTP头部
 * @param ch 输入字符
 * 
 * 支持多行头部解析，处理头部字段和值
 */
void HttpRequestParser::parseHeaders(char ch) {
    static bool expectingHeaderValue = false;
    
    if (ch == '\r') {
        return;  // 忽略回车符
    }
    
    if (ch == '\n') {
        if (lineBuffer_.empty()) {
            // 空行表示头部结束
            if (!result_.headers["Content-Length"].empty()) {
                result_.state = State::BODY;  // 有消息体，转入消息体解析
            } else {
                result_.state = State::FINISHED;  // 无消息体，解析完成
            }
            return;
        }
        
        if (expectingHeaderValue) {
            // 完成一个头部字段的解析
            result_.headers[currentHeaderField_] = trim(currentHeaderValue_);
            currentHeaderField_.clear();
            currentHeaderValue_.clear();
            expectingHeaderValue = false;
        }
        lineBuffer_.clear();
        return;
    }
    
    if (ch == ':' && !expectingHeaderValue) {
        expectingHeaderValue = true;
        currentHeaderField_ = trim(lineBuffer_);
        lineBuffer_.clear();
        return;
    }
    
    if (expectingHeaderValue) {
        currentHeaderValue_ += ch;
    } else {
        lineBuffer_ += ch;
    }
}

/**
 * @brief 解析HTTP消息体
 * @param ch 输入字符
 * 
 * 根据Content-Length读取指定长度的消息体
 */
void HttpRequestParser::parseBody(char ch) {
    result_.body += ch;
    if (result_.body.length() == std::stoul(result_.headers["Content-Length"])) {
        result_.state = State::FINISHED;  // 消息体接收完成
    }
}

/**
 * @brief 构建HTTP响应
 * @param content 响应内容
 * @param contentType 内容类型
 * @param statusCode 状态码
 * @return 完整的HTTP响应
 * 
 * 使用预分配的缓冲区构建响应头，
 * 避免频繁的字符串拼接
 */
std::string Http::buildResponse(const std::string& content, const std::string& contentType, int statusCode) {
    // 使用snprintf直接写入预分配的缓冲区
    int written = snprintf(header_buffer, MAX_HEADER_SIZE,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: keep-alive\r\n"
        "Keep-Alive: timeout=5, max=100\r\n"
        "Server: %s\r\n"
        "\r\n",
        statusCode,
        statusCode == 200 ? "OK" : 
        statusCode == 404 ? "Not Found" : 
        statusCode == 400 ? "Bad Request" : "Internal Server Error",
        contentType.c_str(),
        content.length(),
        SERVER_NAME
    );

    if (written >= MAX_HEADER_SIZE) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    }

    // 只进行一次字符串拼接
    return std::string(header_buffer, written) + content;
}

/**
 * @brief 工具函数：分割字符串
 * @param str 输入字符串
 * @param delimiter 分隔符
 * @return 分割后的字符串数组
 */
std::vector<std::string> HttpRequestParser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

/**
 * @brief 工具函数：去除字符串首尾空白字符
 * @param str 输入字符串
 * @return 处理后的字符串
 */
std::string HttpRequestParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

std::string Http::parseRequest(const std::string& request) {
    std::string path;
    size_t pathStart = request.find("GET ") + 4;
    if (pathStart != std::string::npos) {
        size_t pathEnd = request.find(" HTTP", pathStart);
        if (pathEnd != std::string::npos) {
            path = request.substr(pathStart, pathEnd - pathStart);
        }
    }
    return path.empty() ? "/index.html" : path;
}

bool Http::isKeepAlive(const std::string& request) {
    return request.find("Connection: keep-alive") != std::string::npos;
}

std::string Http::build404Response() {
    return buildResponse("404 Not Found", "text/plain", HTTP_NOT_FOUND);
}

std::string Http::build500Response() {
    return buildResponse("500 Internal Server Error", "text/plain", HTTP_SERVER_ERROR);
}