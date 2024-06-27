// Cross-Platform HTTP Client
// Author: Jack Burton
// 
// Provides a simple HTTP GET client, compatible with
// both Windows and Linux environments


#include "client.h"


const int MAX_SIZE = 2048; // bytes
const int TIMEOUT = 5000; // milliseconds


// Main driver function
// Purpose: Creates request to specified URL and cleans up
// Allocates: Buffer for response
// Returns: Program exit status, either EXIT_SUCCESS or EXIT_FAILURE
int main(int argc, char *argv[])
{
    addr_info config, *result;
    char *buf = (char *)malloc(MAX_SIZE);
    int sockfd;

    if (initialize(argc, argv, &config, &result, &sockfd) != EXIT_SUCCESS) {
        free(buf);
        return EXIT_FAILURE;
    }

    if (set_timeout(sockfd, TIMEOUT) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    
    request(sockfd, buf);

    cleanup(sockfd, result, buf);

    return EXIT_SUCCESS;
}

// Initialize
// Purpose: Establishes winsock (if on Windows) and connection with server
// Allocates: None
// Returns: EXIT_FAILURE if connection fails, or EXIT_SUCCESS otherwise
int initialize(int argc, char *argv[], addr_info *config, addr_info **result, int *sockfd)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    #ifdef WIN32
    if (est_winsock() != 0) {
        return EXIT_FAILURE;
    }
    #endif

    if (est_addrinfo(argv[1], argv[2], config, result) != 0) {
        return EXIT_FAILURE;
    }

    if ((*sockfd = est_connection(*result)) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Cleanup
// Purpose: Frees all allocated memory
// Allocates: None
// Returns: None
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

// est_winsock
// Purpose: Establishes Winsock2 instance (only executed on Windows)
// Allocates: Winsock2 instance
// Returns: EXIT_FAILURE if failed to establish, or EXIT_SUCCESS otherwise
#ifdef WIN32
int est_winsock()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 of Winsock is not available.\n");
        WSACleanup();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#endif

// est_addrinfo
// Purpose: Establishes addrinfo struct for specified host & port
// Allocates: Addrinfo struct
// Returns: EXIT_FAILURE if failed to establish, or EXIT_SUCCESS otherwise
int est_addrinfo(const char *host, const char *port, addr_info *config, addr_info **result)
{
    int status;
    memset(config, 0, sizeof(*config));
    config->ai_family = AF_UNSPEC;  // use IPv4 or IPv6
    config->ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, config, result)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// est_connection
// Purpose: Establishes socket connection
// Allocates: Socket instance
// Returns: -1 if socket failed to be established, or socket ID otherwise
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

// set_timeout
// Purpose: Creates timeout to load target
// Allocates: None
// Returns: EXIT_FAILURE if timeout cannot be established, or EXIT_SUCCESS otherwise
int set_timeout(int sockfd, int ms)
{
    #ifdef WIN32
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms)) < 0) {
        fprintf(stderr, "setsockopt failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    #else
    struct timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "setsockopt failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    #endif

    return EXIT_SUCCESS;
}

// request
// Purpose: Creates GET request to target host & port
// Allocates: None
// Returns: None
void request(int sockfd, char *buf)
{
    const char *payload = "GET / HTTP/1.0\r\n\r\n";
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
