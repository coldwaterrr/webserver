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
    if (path == "/index.html") {
        logger.info("Routing request: " + path, clientIp);
    }

    // If the request path is a directory, return index.html by default
    if (std::filesystem::is_directory(filePath)) {
        filePath += "/index.html";
        if (path == "/index.html") {
            logger.info("Directory requested, serving index.html", clientIp);
        }
    }
    std::string response = "";
    // Check if the file exists
    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath)) {
        logger.warning("File not found: " + filePath, clientIp);
            response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        send(client_socket, response.c_str(), strlen(response.c_str()), 0);
    }
    else {
        logger.info("the file is exists");
        // [[likely]]
        std::string content = readFileContent(filePath);
        // logger.info("the content is: " + content);
        std::string mimeType = getMimeType(filePath);
        response = Http::sendResponse(client_socket, content, mimeType, 200, clientIp);
    }

    // usleep(100000); // 1 毫秒
    // Close the client socket and log
    // close(client_socket);
    // if (path == "/index.html") {
    //     logger.info("Connection closed", clientIp);
    // }
    return response;
}