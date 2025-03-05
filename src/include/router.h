#include "logger.h"
#include <string>

class Router {
public:
    Router(const std::string &staticFolder) : staticFolder(staticFolder) {
        logger.info("Router initialized with static folder: " + staticFolder);
    }
    void route(const std::string &path, int client_socket, const std::string &clientIp);

private:
    std::string staticFolder;
    std::string getMimeType(const std::string &path);
    std::string readFileContent(const std::string &filePath);
    Logger logger;
};