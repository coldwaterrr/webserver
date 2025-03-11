#include <string>
#include <fstream>
#include <sys/socket.h>
#include "logger.h"
#include "http.h"
#include <sstream>
#include <cstring>

#define MAX_SIZE 8192

std::string Http::getRequestPath(const std::string &request) {
    size_t pos1 = request.find("GET ");
    size_t pos2 = request.find(" HTTP/");
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return "/";
    }
    return request.substr(pos1 + 4, pos2 - (pos1 + 4)); // Extract the path from the request
}

std::string Http::sendResponse(int client_socket, const std::string &content,
                        const std::string &mimeType, int statusCode, const std::string &clientIp) {
    std::ostringstream response;
    Logger logger;
    response << "HTTP/1.1 " << statusCode << " OK\r\n"
             << "Content-Type: " << mimeType << "\r\n"
             << "Connection: keep-alive\r\n"
             << "Content-Length: " << content.size() << "\r\n"
             << "\r\n"
             << content;
    std::string responseStr = response.str();
    // memcpy(data, response.str().c_str(), sizeof(response.str().c_str()));
    size_t bytes_sent = send(client_socket, responseStr.c_str(), responseStr.size(), 0);
    logger.success("send the msg, bytes sent: " + std::to_string(bytes_sent));

    logger.info(
        "Response sent: status=" + std::to_string(statusCode) +
            ", size=" + std::to_string(content.size()) +
            ", type=" + mimeType,
        clientIp);

    return responseStr;
}