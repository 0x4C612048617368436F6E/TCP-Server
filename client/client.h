#include<stdio.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdlib.h>
#ifndef SERVER
#define SERVER
#define SIZEOFBUFFER 1024 //1kb
void client();
char* userInput();
#endif
