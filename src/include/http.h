#pragma once

#include <string>
#include <fstream>
#include <sys/socket.h>
#include "logger.h"

class Http {
public:
    // Http();
    // ~Http();
    static std::string getRequestPath(const std::string &request);
    static std::string sendResponse(int client_socket, const std::string &content,
                             const std::string &mimeType, int statusCode, const std::string &clientIp);
private:
    static Logger logger;
};

