#include<stdio.h>
#include<sys/socket.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#define PORT 1025
#define PENDINGCLIENT 5
#define SERVERRUNNING 1
#ifndef SERVER
#define SERVER

void server();

#endif
