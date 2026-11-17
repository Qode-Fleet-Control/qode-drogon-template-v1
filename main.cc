#include <drogon/drogon.h>

#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

void jsonEscape(std::string &out, const std::string &in) {
    static const char *hex = "0123456789abcdef";
    for (unsigned char c : in) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += char(c);
        } else if (c < 0x20) {
            out += "\\u00";
            out += hex[c >> 4];
            out += hex[c & 0xf];
        } else {
            out += char(c);
        }
    }
}

// KEEP THIS: one log line per request (status + duration) is what the workspace Monitoring tab charts.
// Called for every response just before it is sent (routes, 404s, static files), and writes
// one JSON line to stdout, e.g.
//   {"status":200,"durationMs":0.123,"method":"GET","path":"/health","ua":"curl/8.5.0"}
// The duration runs from when Drogon created the request. Never the query string, headers
// other than the user agent, or bodies.
void logRequest(const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
    const double ms =
        (trantor::Date::now().microSecondsSinceEpoch() - req->creationDate().microSecondsSinceEpoch()) / 1000.0;
    std::string line = "{\"status\":" + std::to_string(static_cast<int>(resp->statusCode()));
    char dur[32];
    std::snprintf(dur, sizeof dur, "%.3f", ms < 0 ? 0.0 : ms);
    line += ",\"durationMs\":";
    line += dur;
    line += ",\"method\":\"";
    jsonEscape(line, req->methodString());
    line += "\",\"path\":\"";
    jsonEscape(line, req->path());
    line += "\",\"ua\":\"";
    jsonEscape(line, req->getHeader("user-agent"));
    line += "\"}\n";
    std::fwrite(line.data(), 1, line.size(), stdout);  // one write per line: no interleaving across IO threads
    std::fflush(stdout);
}

}  // namespace

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
    // KEEP THIS: one log line per request (status + duration) is what the workspace Monitoring tab charts.
    drogon::app().registerPreSendingAdvice(logRequest);
    LOG_INFO << "Listening on 0.0.0.0:" << (port > 0 && port < 65536 ? port : 8080);
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
