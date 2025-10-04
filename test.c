#include "webserver.h"

int main() {
    struct WebServer server = NewWebServer(
        "127.0.0.1",
        8433
        );

    server.start(&server);

}
