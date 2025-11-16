#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#define PORT 1025
#ifndef SERVER
#define CLIENTINPUT 1 
#define SIZEOFBUFFER 128
void client();
char *userInput(void);
ssize_t send_all(int, const void *, size_t);
#endif
