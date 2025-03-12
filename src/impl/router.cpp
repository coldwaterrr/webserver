#include "router.h"
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <cstring>
#include "http.h"
#include <unistd.h>
#include <filesystem>

#define MAX_SIZE 8192

// Calculate MIME type based on file extension
std::string Router::getMimeType(const std::string &path) {
    if (path.rfind(".html") == path.length() - 5)
        return "text/html";
    if (path.rfind(".css") == path.length() - 4)
        return "text/css";
    if (path.rfind(".js") == path.length() - 3)
        return "application/javascript";
    if (path.rfind(".png") == path.length() - 4)
        return "image/png";
    if (path.rfind(".jpg") == path.length() - 4 || path.rfind(".jpeg") == path.length() - 5)
        return "image/jpeg";
    if (path.rfind(".gif") == path.length() - 4)
        return "image/gif";
    return "text/plain";
}

std::string Router::readFileContent(const std::string &filePath) {
    // 打开指定路径的文件
    std::ifstream file(filePath);
    
    // 如果文件打开失败，记录警告并返回空字符串
    if (!file.is_open()) {
        logger.warning("Failed to open file: " + filePath, "");
        return "";
    }

    // 使用 stringstream 缓冲区读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();  // 将文件内容读取到缓冲区中

    // 返回文件的字符串内容
    return buffer.str();
}


std::string Router::route(const std::string &path, int client_socket, const std::string &clientIp) {
    std::string filePath = staticFolder + path;
    if (std::filesystem::is_directory(filePath)) {
        filePath += "/index.html";
    }

    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath)) {
        return Http::build404Response();
    }

    try {
        std::string content = readFileContent(filePath);
        std::string mimeType = getMimeType(filePath);
        return Http::buildResponse(content, mimeType, 200);
    } catch (const std::exception& e) {
        logger.error("Failed to read file: " + std::string(e.what()));
        return Http::build500Response();
    }
}