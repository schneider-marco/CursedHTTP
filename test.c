#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include </Users/marco/CLionProjects/CursedHTTP/webserver.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


int main() {
    struct WebServer server = NewWebServer(
        AF_INET,
        SOCK_STREAM,
        0,
        "127.0.0.1",
        8443,
        10
        );

    server.start(&server);

}
