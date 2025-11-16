#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#define RECEIVEDMESSAGE 2024
#define PORT 1025
#define BACKLOG 5
#define SERVERRUNNING 1
#ifndef SERVER
#define SERVER

void server();
ssize_t recv_all(int, void *, size_t);

#endif
