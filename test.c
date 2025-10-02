#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include </Users/marco/CLionProjects/CursedHTTP/webserver.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void start(struct WebServer server) {
    char *ip = inet_ntoa(server.address.sin_addr);
    printf("Starting server on http://%s:%d\n", ip, ntohs(server.address.sin_port));

    while (1) {
        char buffer[1024];
        struct sockaddr_in client_addr;

        socklen_t address_len = sizeof(client_addr);

        int sock = accept(server.socket, (struct sockaddr *)&client_addr, &address_len);
        if (sock < 0) {
            perror("accept");
            continue;
        }

        int n = read(sock, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            size_t len = 0;
            char *body = read_html_to_body("index.html", &len);
            char header[256];
            int hlen = snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Cache-Control: no-store\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n"
                "\r\n", len);

            buffer[n] = '\0';
            printf("Incoming Request: %s\n", buffer);

            write(sock, header, strlen(header));
            write(sock, body, strlen(body));
        }
        close(sock);
    }
}

int main() {
    struct WebServer server = NewWebServer(
        AF_INET,
        SOCK_STREAM,
        0,
        "127.0.0.1",
        8443,
        10,
        start
        );

    server.start(server);

}
