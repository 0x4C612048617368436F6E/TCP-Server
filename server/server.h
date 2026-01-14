//check what operating system
#ifndef SERVER
#define SERVER
//some errors stuff
#define _ERROR
#define _DEBUG
#define _INFO

#define PORT 1025
#define BACKLOG 5
#define SERVERRUNNING 1
#define MESSAGESIZELIMIT ((1024)*(1024)*10)//lets make it 10MB
#include<stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_DEBUG)
#define CUSTOM_DEBUG(...) {printf("[ DEBUG ] ");printf(__VA_ARGS__);printf("\n");}
#endif

#if defined(_ERROR)
#define CUSTOM_ERROR(...) {printf("[ ERROR ] ");printf(__VA_ARGS__);printf("\n");}
#endif

#if defined(_INFO)
#define CUSTOM_INFO(...) {printf("[ INFO ] ");printf(__VA_ARGS__);printf("\n");}
#endif

extern void server(void);
extern ssize_t recv_all(int, void *, size_t);

#endif

#if defined(_WIN32) || defined(_WIN64)
//link the lib
#pragma comment(lib,"ws2_32.lib")
//include windows header
#include<Winsock2.h>
#include<windows.h>
#include<Ws2ipdef.h>
#elif defined(__unix__) || defined(__linux__) || defined(_POSIX_VERSION)
//otherwise Linuz POSIX header
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif