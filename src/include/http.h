#pragma once

#include <string>
#include <fstream>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include "logger.h"

class HttpRequestParser {
public:
    enum class State {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISHED,
        ERROR
    };

    struct ParseResult {
        std::string method;
        std::string path;
        std::string version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
        State state{State::REQUEST_LINE};
        
        bool isComplete() const { return state == State::FINISHED; }
    };

    ParseResult parse(const char* data, size_t len);

private:
    ParseResult result_;
    std::string currentHeaderField_;
    std::string currentHeaderValue_;
    std::string lineBuffer_;
    
    void parseRequestLine(char ch);
    void parseHeaders(char ch);
    void parseBody(char ch);
    
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string trim(const std::string& str);
};

class Http {
public:
    static std::string parseRequest(const std::string& request);
    static bool isKeepAlive(const std::string& request);
    static std::string buildResponse(const std::string& content, 
                                    const std::string& contentType, 
                                    int statusCode = 200);
    static std::string build404Response();
    static std::string build500Response();
    
    // 修改为使用 rfind 来检查文件扩展名
    static std::string getMimeType(const std::string& path) {
        if (path.rfind(".html") != std::string::npos) return "text/html";
        if (path.rfind(".css") != std::string::npos) return "text/css";
        if (path.rfind(".js") != std::string::npos) return "application/javascript";
        if (path.rfind(".jpg") != std::string::npos || 
            path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
        if (path.rfind(".png") != std::string::npos) return "image/png";
        if (path.rfind(".gif") != std::string::npos) return "image/gif";
        if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
        return "text/plain";
    }

private:
    static Logger logger;

    // 预分配响应头缓冲区
    static constexpr size_t MAX_HEADER_SIZE = 4096;
    static char header_buffer[MAX_HEADER_SIZE];

    // HTTP状态码常量
    static constexpr int HTTP_OK = 200;
    static constexpr int HTTP_BAD_REQUEST = 400;
    static constexpr int HTTP_NOT_FOUND = 404;
    static constexpr int HTTP_SERVER_ERROR = 500;

    // 响应头常量
    static constexpr const char* SERVER_NAME = "SimpleWebServer";
    static constexpr const char* KEEP_ALIVE_HEADER = "Keep-Alive: timeout=5, max=100";
};

