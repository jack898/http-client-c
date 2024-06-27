// Cross-Platform HTTP Client
// Author: Jack Burton
// 
// Provides a simple HTTP GET client, compatible with
// both Windows and Linux environments

#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

typedef struct addrinfo addr_info;

int set_timeout(int sockfd, int ms);
int initialize(int argc, char *argv[], addr_info *config, addr_info **result, int *sockfd);
void cleanup(int sockfd, addr_info *result, char *buf);
int est_addrinfo(const char *host, const char *port, addr_info *config, addr_info **result);
int est_winsock();
int est_connection(addr_info *result);
void request(int sockfd, char *buf);


#endif
