
#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>

static char file_buffer[8192];
static char read_buffer[8192];

static const char *default_header =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "\r\n";

static const char *default_body =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\" />\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n"
"    <title>CursedHTTP is running 🚀</title>\n"
"    <style>\n"
"        :root {\n"
"            --accent: #00ffc3;\n"
"            --bg: #0e0e0e;\n"
"            --text: #e6e6e6;\n"
"            --muted: #888;\n"
"        }\n"
"\n"
"        * {\n"
"            box-sizing: border-box;\n"
"        }\n"
"\n"
"        body {\n"
"            margin: 0;\n"
"            font-family: \"Inter\", \"Segoe UI\", Roboto, sans-serif;\n"
"            background: var(--bg);\n"
"            color: var(--text);\n"
"            display: flex;\n"
"            flex-direction: column;\n"
"            justify-content: center;\n"
"            align-items: center;\n"
"            height: 100vh;\n"
"            text-align: center;\n"
"            overflow: hidden;\n"
"        }\n"
"\n"
"        .logo {\n"
"            font-size: 2.8rem;\n"
"            font-weight: 700;\n"
"            letter-spacing: 1px;\n"
"            color: var(--accent);\n"
"            text-shadow: 0 0 10px var(--accent), 0 0 30px #00ffc388;\n"
"            animation: glow 3s ease-in-out infinite alternate;\n"
"            user-select: none;\n"
"        }\n"
"\n"
"        @keyframes glow {\n"
"            from { text-shadow: 0 0 10px var(--accent), 0 0 30px #00ffc388; }\n"
"            to { text-shadow: 0 0 20px var(--accent), 0 0 60px #00ffc3; }\n"
"        }\n"
"\n"
"        h1 {\n"
"            font-size: 2rem;\n"
"            margin-top: 0.8em;\n"
"            color: var(--accent);\n"
"        }\n"
"\n"
"        p {\n"
"            font-size: 1.1rem;\n"
"            max-width: 460px;\n"
"            line-height: 1.6;\n"
"            color: var(--muted);\n"
"        }\n"
"\n"
"        a {\n"
"            color: var(--accent);\n"
"            text-decoration: none;\n"
"            transition: opacity 0.2s;\n"
"        }\n"
"\n"
"        a:hover {\n"
"            opacity: 0.8;\n"
"        }\n"
"\n"
"        footer {\n"
"            position: absolute;\n"
"            bottom: 15px;\n"
"            font-size: 0.9rem;\n"
"            color: var(--muted);\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"<div class=\"logo\">CursedHTTP</div>\n"
"<h1>Server is up and running 🚀</h1>\n"
"<p>\n"
"    For configuration and usage details, visit the\n"
"    <a href=\"https://localhost/docs\" target=\"_blank\" rel=\"noopener noreferrer\">\n"
"        official CursedHTTP documentation\n"
"    </a>.\n"
"</p>\n"
"<footer>\n"
"    powered by <a href=\"#\">CursedHTTP</a> ⚡\n"
"</footer>\n"
"</body>\n"
"</html>\n";


void start(struct WebServer *self) {
    char *ip = inet_ntoa(self->address.sin_addr);
    printf("Starting server on http://%s:%d\n", ip, ntohs(self->address.sin_port));

    while (1) {
        struct sockaddr_in client_addr;

        socklen_t address_len = sizeof(client_addr);

        int sock = accept(self->socket, (struct sockaddr *)&client_addr, &address_len);
        if (sock < 0) {
            perror("accept");
            continue;
        }

        int n = read(sock, read_buffer, sizeof(read_buffer) - 1);
        if (n > 0) {
            size_t len = 0;
            read_buffer[n] = '\0';

            char *newline = strstr(read_buffer, "\r\n");
            if (newline) *newline = '\0';
            printf("%s\n", read_buffer);

            write(sock, default_header, strlen(default_header));
            write(sock, default_body, strlen(default_body));
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

struct WebServer NewWebServer(char *interface, int port) {


    struct WebServer webserver;
    webserver.domain = AF_INET;
    webserver.service = SOCK_STREAM;
    webserver.protocol = 0;
    webserver.interface = interface;
    webserver.port = port;
    webserver.backlog = 128;
    webserver.start = start;
    webserver.address.sin_family = webserver.domain;
    webserver.address.sin_port = htons(port);

    if (inet_pton(AF_INET, interface, &webserver.address.sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(1);
    }

    webserver.socket = socket(webserver.domain, webserver.service, webserver.protocol);
    if (webserver.socket == -1) {
        perror("Failed to create socket\n");
        exit(1);
    }

    if ((bind(webserver.socket, (struct sockaddr *)&webserver.address, sizeof(webserver.address))) < 0) {
        perror("Failed to bind socket\n");
        exit(1);
    };

    if ((listen(webserver.socket, webserver.backlog)) < 0) {
        perror("Failed to listen on socket\n");
        exit(1);
    };

    return webserver;
};