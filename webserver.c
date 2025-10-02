
#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

static char file_buffer[8192];

struct WebServer NewWebServer(int domain, int service, int protocol, char *interface, int port, int backlog,
                              void (*start)(struct WebServer)) {
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
