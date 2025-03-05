#include <string>
#include <fstream>
#include <sys/socket.h>
#include "logger.h"
#include "http.h"
#include <sstream>

std::string Http::getRequestPath(const std::string &request) {
    size_t pos1 = request.find("GET ");
    size_t pos2 = request.find(" HTTP/");
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return "/";
    }
    return request.substr(pos1 + 4, pos2 - (pos1 + 4)); // Extract the path from the request
}

void Http::sendResponse(int client_socket, const std::string &content,
                        const std::string &mimeType, int statusCode, const std::string &clientIp) {
    std::ostringstream response;
    Logger logger;
    response << "HTTP/1.1 " << statusCode << " OK\r\n"
             << "Content-Type: " << mimeType << "\r\n"
             << "Content-Length: " << content.size() << "\r\n"
             << "\r\n"
             << content;
    send(client_socket, response.str().c_str(), response.str().size(), 0);
    // logger.info("receive http request:");
    // logger.info(response.str().c_str());
    // logger.info()
    logger.info(
        "Response sent: status=" + std::to_string(statusCode) +
            ", size=" + std::to_string(content.size()) +
            ", type=" + mimeType,
        clientIp);
}