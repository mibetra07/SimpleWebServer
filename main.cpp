#include "WebServer.h"

constexpr int PORT { 8080 };
constexpr int BUFFER_SIZE { 4096 };

int main()
{
    http::WebServer webServer{ PORT, BUFFER_SIZE };
    webServer.run();
    return 0;
}