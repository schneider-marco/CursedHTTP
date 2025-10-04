#include <sys/socket.h>
#include <netinet/in.h>

struct WebServer {
    int domain;
    int service;
    int protocol;
    char *interface;
    int port;
    int backlog;

    struct sockaddr_in address;

    int socket;

    void (*start)(struct WebServer *self);
};

struct WebServer NewWebServer(int domain, int service, int protocol, char *interface, int port, int backlog);

char *read_html_to_body(const char *path, size_t *out_len);