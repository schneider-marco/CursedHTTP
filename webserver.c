
#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>

// static file buffer maybe make dynamic
static char file_buffer[8192];

void start(struct WebServer *self) {
    char *ip = inet_ntoa(self->address.sin_addr);
    printf("Starting server on http://%s:%d\n", ip, ntohs(self->address.sin_port));

    while (1) {
        char buffer[1024];
        struct sockaddr_in client_addr;

        socklen_t address_len = sizeof(client_addr);

        int sock = accept(self->socket, (struct sockaddr *)&client_addr, &address_len);
        if (sock < 0) {
            perror("accept");
            continue;
        }

        int n = read(sock, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            size_t len = 0;
            char *body = read_html_to_body("index.html", &len);
            //char header[256];
            //snprintf(header, sizeof(header),
            const char *header =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Cache-Control: no-store\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n"
                "\r\n";

            buffer[n] = '\0';
            printf("Incoming Request: %s\n", buffer);

            write(sock, header, strlen(header));
            write(sock, body, strlen(body));
        }
        close(sock);
    }
}

char *read_html_to_body(const char *path, size_t *out_len) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("failed to open file\n");
        exit(1);
    }

    ssize_t n = read(fd, file_buffer, sizeof(file_buffer) - 1);
    if (n < 0) {
        perror("file read failed \n");
        close(fd);
        exit(1);
    }

    file_buffer[n] = '\0';
    close(fd);

    if (out_len) *out_len = (size_t)n;
    return file_buffer;
}

struct WebServer NewWebServer(int domain, int service, int protocol, char *interface, int port, int backlog) {



    struct WebServer webserver;
    webserver.domain = domain;
    webserver.service = service;
    webserver.protocol = protocol;
    webserver.interface = interface;
    webserver.port = port;
    webserver.backlog = backlog;
    webserver.start = start;
    webserver.address.sin_family = domain;
    webserver.address.sin_port = htons(port);

    if (inet_pton(AF_INET, interface, &webserver.address.sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(1);
    }

    webserver.socket = socket(domain, service, protocol);
    if (webserver.socket == -1) {
        perror("Failed to create socket\n");
        exit(1);
    }

    if ((bind(webserver.socket, (struct sockaddr *)&webserver.address, sizeof(webserver.address))) < 0) {
        perror("Failed to bind socket\n");
        exit(1);
    };

    if ((listen(webserver.socket, backlog)) < 0) {
        perror("Failed to listen on socket\n");
        exit(1);
    };

    return webserver;
};