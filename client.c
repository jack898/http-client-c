#include "client.h"


const int MAX_SIZE = 2048; // bytes
const int TIMEOUT = 5000; // milliseconds

void set_timeout(int sockfd, int ms);
int initialize(int argc, char *argv[], addr_info *config, addr_info **result, int *sockfd);
void cleanup(int sockfd, addr_info *result, char *buf);

int main(int argc, char *argv[])
{
    addr_info config, *result;
    char *buf = (char *)malloc(MAX_SIZE);
    int sockfd;

    if (initialize(argc, argv, &config, &result, &sockfd) != 0) {
        free(buf);
        return 1;
    }

    set_timeout(sockfd, TIMEOUT);
    request(sockfd, buf);

    cleanup(sockfd, result, buf);

    return 0;
}

int initialize(int argc, char *argv[], addr_info *config, addr_info **result, int *sockfd)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        return 1;
    }

    #ifdef WIN32
    if (est_winsock() != 0) {
        return 1;
    }
    #endif

    if (est_addrinfo(argv[1], argv[2], config, result) != 0) {
        return 1;
    }

    if ((*sockfd = est_connection(*result)) == -1) {
        return 1;
    }

    return 0;
}

void cleanup(int sockfd, addr_info *result, char *buf)
{
    #ifdef WIN32
    closesocket(sockfd);
    WSACleanup();
    #else
    close(sockfd);
    #endif
    free(buf);
    freeaddrinfo(result);
}

#ifdef WIN32
int est_winsock()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 of Winsock is not available.\n");
        WSACleanup();
        return 1;
    }
    return 0;
}
#endif

int est_addrinfo(const char *host, const char *port, addr_info *config, addr_info **result)
{
    int status;
    memset(config, 0, sizeof(*config));
    config->ai_family = AF_UNSPEC;  // use IPv4 or IPv6
    config->ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, config, result)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }
    return 0;
}

int est_connection(addr_info *result)
{
    int mainfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (mainfd == -1) {
        fprintf(stderr, "Establishing socket failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (connect(mainfd, result->ai_addr, result->ai_addrlen) == -1) {
        fprintf(stderr, "connect failed: %s\n", strerror(errno));
        close(mainfd);
        return -1;
    } else {
        printf("Connected to server.\n");
        return mainfd;
    }
}

void set_timeout(int sockfd, int ms)
{
    #ifdef WIN32
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms)) < 0) {
        fprintf(stderr, "setsockopt failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    #else
    struct timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "setsockopt failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    #endif
}

void request(int sockfd, char *buf)
{
    const char *payload = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    if (send(sockfd, payload, strlen(payload), 0) == -1) {
        fprintf(stderr, "Failed to send GET request: %s\n", strerror(errno));
    } 

    int bytes_received;
    while ((bytes_received = recv(sockfd, buf, MAX_SIZE, 0)) > 0) {
        fwrite(buf, 1, bytes_received, stdout);
        memset(buf, 0, MAX_SIZE); // Clear the buffer for the next recv
    }

    if (bytes_received == 0) {
        printf("\nConnection closed by the server.\n");
    } else if (bytes_received == -1) {
        fprintf(stderr, "Recv ended: %s\n", strerror(errno));
    }
}
