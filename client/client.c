#include"client.h"
void client(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("socket"); 
        return; 
    }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "192.168.236.112", &srv.sin_addr) != 1) {
        perror("inet_pton");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&srv, sizeof(srv)) == -1) {
        perror("connect");
        close(sock);
        return;
    }

    while (CLIENTINPUT) {
        char *msg = userInput();
        if (!msg) break;
        if (strcmp(msg, "exit") == 0) { 
            free(msg); 
            break; 
        }

        size_t payload_len = strlen(msg) + 1; // include '\0' if you want
        uint32_t netlen = htonl((uint32_t)payload_len);

        if (send_all(sock, &netlen, sizeof(netlen)) < 0) { 
            free(msg); 
            break; 
        }
        if (send_all(sock, msg, payload_len) < 0) { 
            free(msg); 
            break; 
        }

        free(msg);
    }

    close(sock);
    return;
}

char *userInput(void) {
    size_t cap = SIZEOFBUFFER;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) { 
        perror("malloc"); 
        return NULL; 
    }

    printf("> ");
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (len + 1 >= cap) {
            cap += SIZEOFBUFFER;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}


ssize_t send_all(int sock, const void *buf, size_t len) {
    const char *p = buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n < 0) {
            perror("send");
            return -1;
        }
        if (n == 0) return total;
        total += (size_t)n;
    }
    return (ssize_t)total;
}