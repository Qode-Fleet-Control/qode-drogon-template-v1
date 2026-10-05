#include <drogon/drogon.h>

#include <cstdlib>

int main() {
    // Set HTTP listener address and port. The fleet injects $PORT; read it at
    // runtime (default 8080) and listen on every interface so the container
    // is reachable.
    const char *envPort = std::getenv("PORT");
    const int port = envPort ? std::atoi(envPort) : 0;
    drogon::app().addListener("0.0.0.0", port > 0 && port < 65536 ? port : 8080);
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    drogon::app().setThreadNum(0);  // one IO thread per CPU core
    LOG_INFO << "Listening on 0.0.0.0:" << (port > 0 && port < 65536 ? port : 8080);
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
